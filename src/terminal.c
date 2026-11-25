/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "cmdr.h"
#include "unikey.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
static int active, pasting;
static volatile int resized;
int screen_cols=80, screen_rows=25;
static void stopped(int sig){signal(sig,stopped);stopping=1;}
#ifdef SIGWINCH
static void window_changed(int sig){signal(sig,window_changed);resized=1;}
#endif
void at(int row,int col){printf("\033[%d;%dH",row+1,col+1);}
void field(int row,int col,int width,const char *s,int reverse) {
    int i;if(row<0||row>=screen_rows||col<0||col>=screen_cols)return;
    if(width>screen_cols-col-1)width=screen_cols-col-1;
    at(row,col);fputs(reverse?"\033[0;7m":"\033[0m",stdout);
    for(i=0;i<width;i++) {
        unsigned char c=*s?(unsigned char)*s++:' ';
        putchar(c>=32 && c<127?c:'?');
    }
    fputs("\033[0m",stdout);
}
void terminal_close(void) {
    if(!active)return;
    fputs("\033[0m\033[?25h\033[H\033[2J",stdout);fflush(stdout);
    tty_restore();active=0;
}
int terminal_size(void) {
    int cols=screen_cols,rows=screen_rows;
    resized=0;
    if(tty_getsize(&cols,&rows)>0) {
        if(cols<80||rows<25){errno=EINVAL;return -1;}
        screen_cols=cols>MAX_COLS?MAX_COLS:cols;
        screen_rows=rows>100?100:rows;
    }
    return 0;
}
int terminal_open(void) {
    if(!isatty(0)||!isatty(1)){errno=ENOTTY;return -1;}
    if(tty_init(CatchISig)<0)return -1;
    active=1;pasting=0;
    signal(SIGINT,stopped);signal(SIGTERM,stopped);signal(SIGHUP,stopped);signal(SIGQUIT,stopped);
#ifdef SIGWINCH
    signal(SIGWINCH,window_changed);
#endif
    if(terminal_size()<0){terminal_close();return -1;}
    fputs("\033[0m\033[H\033[2J\033[?25l",stdout);return 0;
}
int decodekey(const char *s) {
    unsigned k=(unsigned)ansi_to_unikey((char *)s,(int)strlen(s));
    /* Unicode private-use keys may be negative in a 16-bit int. */
    if(k==(unsigned)kUpArrow)return K_UP;
    if(k==(unsigned)kDownArrow)return K_DOWN;
    if(k==(unsigned)kLeftArrow)return K_LEFT;
    if(k==(unsigned)kRightArrow)return K_RIGHT;
    if(k==(unsigned)kHome)return K_HOME;
    if(k==(unsigned)kEnd)return K_END;
    if(k==(unsigned)kPageUp)return K_PGUP;
    if(k==(unsigned)kPageDown)return K_PGDN;
    if(k>=(unsigned)kF1 && k<=(unsigned)kF10)return K_F1+(k-(unsigned)kF1);
    return k<128?(int)k:0;
}
int key(void) {
    char s[64];int n,x,y;
    fflush(stdout);
    while(!stopping) {
        if(resized){resized=0;return K_RESIZE;}
        n=readansi(0,s,sizeof(s));
        if(n<0){if(errno==EINTR)continue;stopping=1;return -1;}
        if(!n)continue;
        if(!strcmp(s,"\033[200~")){pasting=1;continue;}
        if(!strcmp(s,"\033[201~")){pasting=0;continue;}
        if(pasting)continue;
        /* A late probe response is not a command or filename input. */
        if(ansi_dsr(s,n,&x,&y)>0)continue;
        return decodekey(s);
    }
    return -1;
}
int prompt(const char *label,char *buf,int cap) {
    int k,n=(int)strlen(buf);
    field(screen_rows-3,0,screen_cols-1,label,1);field(screen_rows-1,0,screen_cols-1,"Enter accepts | Esc cancels | Backspace edits",0);
    while(!stopping) {
        field(screen_rows-2,0,screen_cols-1,buf+n-(n>screen_cols-3?screen_cols-3:n),0);at(screen_rows-2,n>screen_cols-3?screen_cols-3:n);fputs("\033[?25h",stdout);
        k=key();
        if(k==27||k<0){fputs("\033[?25l",stdout);return 0;}
        if(k=='\r'||k=='\n'){fputs("\033[?25l",stdout);return 1;}
        if((k==8||k==127)&&n)buf[--n]=0;
        else if(k>=32 && k<127 && n<cap-1){buf[n++]=(char)k;buf[n]=0;}
    }
    return 0;
}
