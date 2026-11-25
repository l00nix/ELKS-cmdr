/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "cmdr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
volatile int stopping;
static struct panel panels[2];
static int current;
static void reload(void) {
    int i;for(i=0;i<2;i++)if(loadpanel(&panels[i])<0)error_message("Read directory");
}
static struct entry *selection(void) {
    struct panel *p=&panels[current];
    return p->count?&p->entries[p->selected]:NULL;
}
static int selectedpath(char *out) {
    struct entry *e=selection();
    if(!e || !strcmp(e->name,"..")){strcpy(message,"Select a file or folder, not '..'.");return 0;}
    if(joinpath(out,panels[current].path,e->name)<0){error_message("Path");return 0;}
    return 1;
}
static void draw(void) {
    int p,r,i;char b[MAX_COLS+1];struct entry *e;
    int half=screen_cols/2;
    field(0,0,screen_cols-1," Cmdr " VERSION "                      Two-panel file manager",1);
    for(p=0;p<2;p++) {
        struct panel *q=&panels[p];
        unsigned len=strlen(q->path);
        int x=p*half,width=(p?screen_cols-half:half)-1,names=width-13;
        if(names>NAME_CAP-1)names=NAME_CAP-1;
        if(q->selected<q->top)q->top=q->selected;
        if(q->selected>=q->top+PAGE_ROWS)q->top=q->selected-PAGE_ROWS+1;
        snprintf(b,sizeof(b),"%c %s%s",current==p?'>':' ',len>(unsigned)(width-4)?"...":"",q->path+(len>(unsigned)(width-4)?len-(width-7):0));
        field(1,x,width,b,current==p);
        snprintf(b,sizeof(b)," %-*s %10s",names,"Name","Bytes");
        field(2,x,width,b,0);
        memset(b,'-',width);b[width]=0;field(3,x,width,b,0);
        for(r=0;r<PAGE_ROWS;r++) {
            i=q->top+r;b[0]=0;
            if(i<q->count) {
                e=&q->entries[i];
                if(S_ISDIR(e->mode))snprintf(b,sizeof(b)," %-*.*s  <DIR>",names,names,e->name);
                else if(S_ISLNK(e->mode))snprintf(b,sizeof(b)," %-*.*s  <LINK>",names,names,e->name);
                else if(!S_ISREG(e->mode))snprintf(b,sizeof(b)," %-*.*s  <DEV>",names,names,e->name);
                else snprintf(b,sizeof(b)," %-*.*s %10lu",names,names,e->name,e->size);
            } else if(!q->count && !r)strcpy(b,q->failed?" [Cannot read directory]":" [Empty directory]");
            field(4+r,x,width,b,current==p && i==q->selected && i<q->count);
        }
        snprintf(b,sizeof(b),"%d/%d%s",q->count?q->selected+1:0,q->count,q->limited&LIMIT_MEMORY?" INCOMPLETE LIST (memory)":q->limited?" INCOMPLETE LIST (name/path/stat)":" entries");
        field(screen_rows-4,x,width,b,0);
    }
    for(r=1;r<=screen_rows-4;r++)field(r,half-1,1,"|",0);
    field(screen_rows-3,0,screen_cols-1,message,0);
    field(screen_rows-2,0,screen_cols-1,"1 Help  2 Path  3 View  4 Edit  5 Copy  6 Move  7 Mkdir  8 Delete  9 Shell",1);
    field(screen_rows-1,0,screen_cols-1,"0 Quit | Tab panel | Enter open | R refresh | F1-F9 same as 1-9; F10/Q Quit",0);
    fflush(stdout);
}
static int confirm(const char *action,const char *src,const char *dst) {
    char b[100];int k,i;
    fputs("\033[H\033[2J",stdout);
    snprintf(b,sizeof(b),"%s?",action);field(1,0,screen_cols-1,b,1);
    field(3,0,79,"Source:",0);
    for(i=0;i<4;i++) {
        unsigned off=(unsigned)i*79;
        field(4+i,0,79,off<strlen(src)?src+off:"",0);
    }
    if(dst) {
        field(9,0,79,"Destination:",0);
        for(i=0;i<4;i++) {
            unsigned off=(unsigned)i*79;
            field(10+i,0,79,off<strlen(dst)?dst+off:"",0);
        }
    }
    field(20,0,79,"Press Y to confirm; any other key cancels.",1);
    field(screen_rows-3,0,screen_cols-1,"Existing destinations are never overwritten.",0);
    k=key();return !stopping && (k=='y'||k=='Y');
}
static void enter(void) {
    char path[PATH_CAP];struct entry *e=selection();struct panel *p=&panels[current];
    if(!e)return;
    if(!S_ISDIR(e->mode)){strcpy(message,"Enter opens folders; F3/V views regular files.");return;}
    if(joinpath(path,p->path,e->name)<0 || canonical(path,path)<0){error_message("Open directory");return;}
    strcpy(p->path,path);freepanel(p);p->selected=p->top=0;
    if(loadpanel(p)<0)error_message("Read directory");else message[0]=0;
}
static void parent(void) {
    char path[PATH_CAP];struct panel *p=&panels[current];
    if(joinpath(path,p->path,"..")<0 || canonical(path,path)<0){error_message("Parent");return;}
    strcpy(p->path,path);freepanel(p);p->selected=p->top=0;
    if(loadpanel(p)<0)error_message("Read directory");else message[0]=0;
}
static void change_path(void) {
    char input[PATH_CAP],path[PATH_CAP];struct panel *p=&panels[current];
    strcpy(input,p->path);
    if(!prompt("Directory path (absolute, or relative to this panel):",input,sizeof(input)))return;
    if(input[0]=='/')strcpy(path,input);
    else if(joinpath(path,p->path,input)<0){error_message("Path");return;}
    if(canonical(path,path)<0){error_message("Directory");return;}
    strcpy(p->path,path);freepanel(p);p->selected=p->top=0;
    if(loadpanel(p)<0)error_message("Read directory");else message[0]=0;
}
static void transfer(int moving) {
    char src[PATH_CAP],dst[PATH_CAP],name[NAME_CAP];struct entry *e=selection();
    if(!selectedpath(src))return;
    if(!S_ISREG(e->mode)){strcpy(message,"Copy/move regular files only; no recursive folders.");return;}
    strcpy(name,e->name);
    if(!prompt(moving?"Move to OTHER panel: destination filename":"Copy to OTHER panel: destination filename",name,sizeof(name)))return;
    if(!validname(name)){strcpy(message,"Use a filename, without '/' (not '.' or '..').");return;}
    if(joinpath(dst,panels[1-current].path,name)<0){error_message("Destination");return;}
    if(!confirm(moving?"Move file (copy, then remove source)":"Copy file",src,dst))return;
    field(screen_rows-3,0,screen_cols-1,"Copying... Ctrl-C cancels and exits.",0);fflush(stdout);
    message[0]=0;
    if(copyfile(src,dst,moving)<0) {
        if(!message[0])error_message(moving?"Move failed; check both panels":"Copy failed");
    } else strcpy(message,moving?"File moved.":"File copied.");
    reload();
}
static void make_directory(void) {
    char name[NAME_CAP]="",path[PATH_CAP];
    if(!prompt("Create folder in ACTIVE panel: name",name,sizeof(name)))return;
    if(!validname(name)){strcpy(message,"Use a folder name, without '/' (not '.' or '..').");return;}
    if(joinpath(path,panels[current].path,name)<0){error_message("Path");return;}
    if(mkdir(path,0777)<0)error_message("Create folder");else strcpy(message,"Folder created.");
    reload();
}
static void delete_selected(void) {
    char path[PATH_CAP];if(!selectedpath(path))return;
    if(!confirm("Permanently delete selected file / empty folder",path,NULL))return;
    if(removeitem(path)<0)error_message("Delete");else strcpy(message,"Deleted.");
    reload();
}
static FILE *open_regular(const char *path) {
    struct stat a,b;int fd;
    if(lstat(path,&a)<0)return NULL;
    if(!S_ISREG(a.st_mode)){errno=EINVAL;return NULL;}
    fd=open(path,O_RDONLY);if(fd<0)return NULL;
    if(fstat(fd,&b)<0 || !S_ISREG(b.st_mode) || a.st_dev!=b.st_dev || a.st_ino!=b.st_ino){close(fd);errno=EAGAIN;return NULL;}
    {FILE *f=fdopen(fd,"r");if(!f)close(fd);return f;}
}
static void view(void) {
    char path[PATH_CAP],line[MAX_COLS+1];FILE *f;int r,col,c=0,k,eof=0;unsigned page=1;
    if(!selectedpath(path))return;
    f=open_regular(path);if(!f){error_message("View regular file");return;}
    while(!stopping) {
        field(0,0,screen_cols-1," Cmdr - text viewer (control/non-ASCII bytes shown as '.')",1);
        field(1,0,screen_cols-1,path,0);
        for(r=2;r<=screen_rows-3;r++) {
            col=0;
            while(col<screen_cols-1 && (c=fgetc(f))!=EOF) {
                if(c=='\r')continue;
                if(c=='\n')break;
                if(c=='\t'){do{line[col++]=' ';}while(col<screen_cols-1 && col%8);}
                else line[col++]=(c>=32 && c<127)?c:'.';
            }
            line[col]=0;field(r,0,screen_cols-1,line,0);
            if(c==EOF)eof=1;
        }
        snprintf(line,sizeof(line),"Page %u%s",page,eof?" - end":"");field(screen_rows-2,0,screen_cols-1,line,0);
        field(screen_rows-1,0,screen_cols-1,"Space/Enter/PgDn next | Home first page | Q/Esc back",1);
        k=key();if(k<0||k==27||k=='q'||k=='Q')break;
        if(k==K_HOME){rewind(f);page=1;eof=0;}
        else if((k==' '||k=='\r'||k=='\n'||k==K_PGDN)&&!eof)page++;
        else { /* Keep current page for unrelated keys or EOF. */
            while(!stopping && k!=K_HOME && k!=27 && k!='q' && k!='Q' && !( !eof && (k==' '||k=='\r'||k=='\n'||k==K_PGDN)))k=key();
            if(stopping||k==27||k=='q'||k=='Q')break;
            if(k==K_HOME){rewind(f);page=1;eof=0;}else page++;
        }
    }
    if(ferror(f))error_message("Read file");
    fclose(f);
}
static void external(int edit) {
    char path[PATH_CAP];pid_t child,w;int status=0;
    if(edit && (!selectedpath(path)||!S_ISREG(selection()->mode))){strcpy(message,"Select a regular file to edit.");return;}
    terminal_close();fflush(stdout);child=fork();
    if(child==0) {
        signal(SIGINT,SIG_DFL);signal(SIGTERM,SIG_DFL);signal(SIGHUP,SIG_DFL);signal(SIGQUIT,SIG_DFL);
        if(chdir(panels[current].path)<0)_exit(126);
        if(edit)execlp("vi","vi",path,(char *)NULL);else execl("/bin/sh","sh",(char *)NULL);
        _exit(127);
    }
    if(child==(pid_t)-1)error_message("Start program");
    else {
        do{w=waitpid(child,&status,0);}while(w==(pid_t)-1 && errno==EINTR);
        if(w==(pid_t)-1)error_message("Wait");else if(status)strcpy(message,"Program exited with an error (vi or /bin/sh must be installed).");else message[0]=0;
    }
    if(!stopping && terminal_open()<0){perror("Cmdr: restore terminal");stopping=1;}
    reload();
}
static void help_screen(void) {
    static const char *lines[]={
        " Cmdr " VERSION " - keyboard guide",
        "Tab: change active panel. Arrows: select. Enter: open folder or view file.",
        "Left/Backspace: parent. Home/End/PgUp/PgDn: navigate the list.",
        "1 Help                       6 Move to the other panel",
        "2 Change panel path          7 Create folder in active panel",
        "3 View file                  8 Delete file or EMPTY folder",
        "4 Edit with vi               9 Shell in active directory",
        "5 Copy to other panel        0 Quit",
        "F1-F9 do the same as 1-9; F10 quits. R refreshes listings and terminal size.",
        "Letter shortcuts: ? P V E C M N D ! Q. Dotfiles are included.",
        "Copy/move prompt for a filename; source and destination are then confirmed.",
        "Existing targets are refused. For a rename, point both panels at one folder.",
        "Move copies then removes the source. On failure, check both panels.",
        "No recursive copy/delete, multi-selection, preserved timestamps or ownership.",
        "Low memory / unreadable or unsupported entries show INCOMPLETE LIST.",
        "Keep the same files unchanged in other programs while copying or moving.",
        "Ctrl-C exits through terminal cleanup. Exit before using HP suspend.",
        "Press any key to return."
    };unsigned i;
    fputs("\033[H\033[2J",stdout);
    for(i=0;i<sizeof(lines)/sizeof(lines[0]);i++)field(i,0,79,lines[i],i==0);
    key();
}
int main(int argc,char **argv) {
    int k,i;struct panel *p;struct entry *e;
    if(argc>1 && (!strcmp(argv[1],"--help")||!strcmp(argv[1],"--version"))) {
        puts("Cmdr " VERSION "\nUsage: cmdr [LEFT_DIRECTORY [RIGHT_DIRECTORY]]\nTwo-panel ANSI file manager (80x25 minimum). 1 for help; 0 quits.\nCopy/move regular files without overwrite; deletion requires confirmation.");return 0;
    }
    if(argc>3 || (argc>1 && argv[1][0]=='-')){fputs("Usage: cmdr [LEFT_DIRECTORY [RIGHT_DIRECTORY]]\n",stderr);return 2;}
    for(i=0;i<2;i++) {
        const char *path=i==0?(argc>1?argv[1]:"."):(argc>2?argv[2]:panels[0].path);
        if(canonical(panels[i].path,path)<0){perror(path);return 1;}
    }
    if(terminal_open()<0){perror("Cmdr: needs an 80x25 text terminal");return 1;}
    atexit(terminal_close);reload();strcpy(message,"Ready. Copy/move target is the other panel; 1 shows help.");
    while(!stopping) {
        draw();k=key();p=&panels[current];
        if(k>='1' && k<='9')k=K_F1+k-'1';
        if(k=='0')k=K_F1+9;
        if(k==K_F1+9 || k=='q'||k=='Q'||k<0)break;
        switch(k) {
        case '\t':current=1-current;break;
        case K_UP:if(p->selected)p->selected--;break;
        case K_DOWN:if(p->selected+1<p->count)p->selected++;break;
        case K_HOME:p->selected=0;break;
        case K_END:p->selected=p->count?p->count-1:0;break;
        case K_PGUP:p->selected=p->selected>PAGE_ROWS?p->selected-PAGE_ROWS:0;break;
        case K_PGDN:p->selected+=PAGE_ROWS;if(p->selected>=p->count)p->selected=p->count?p->count-1:0;break;
        case K_LEFT:case 8:case 127:parent();break;
        case K_RIGHT:enter();break;
        case '\r':case '\n':e=selection();if(e && S_ISDIR(e->mode))enter();else view();break;
        case K_F1:case '?':help_screen();break;
        case K_F1+1:case 'p':case 'P':change_path();break;
        case K_F1+2:case 'v':case 'V':view();break;
        case K_F1+3:case 'e':case 'E':external(1);break;
        case K_F1+4:case 'c':case 'C':transfer(0);break;
        case K_F1+5:case 'm':case 'M':transfer(1);break;
        case K_F1+6:case 'n':case 'N':make_directory();break;
        case K_F1+7:case 'd':case 'D':delete_selected();break;
        case K_F1+8:case '!':external(0);break;
        case K_RESIZE:case 'r':case 'R':
            if(terminal_size()<0){stopping=1;break;}
            fputs("\033[H\033[2J",stdout);reload();break;
        }
    }
    terminal_close();for(i=0;i<2;i++)freepanel(&panels[i]);return 0;
}
