/* SPDX-License-Identifier: GPL-2.0-or-later
 * Deterministic I/O failures against the real copy implementation. */
#include "cmdr.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
volatile int stopping;
static int fault,calls;
static const char *source;
static int allocations;
static void *faulty_malloc(size_t n) {
    if(fault==5 || (fault==6 && n>32*sizeof(struct entry)) || (fault==4 && n<NAME_CAP && ++allocations>5)){errno=ENOMEM;return NULL;}
    return malloc(n);
}
static ssize_t faulty_write(int fd,const void *buf,size_t n) {
    if(fault==1 && ++calls==2){errno=ENOSPC;return -1;}
    if(fault==2 && n>7)n=7;
    return write(fd,buf,n);
}
static int faulty_unlink(const char *p) {
    if(fault==3 && !strcmp(p,source)){errno=EACCES;return -1;}
    return unlink(p);
}
#define malloc faulty_malloc
#define write faulty_write
#define unlink faulty_unlink
#include "../src/files.c"
#undef malloc
#undef write
#undef unlink
static void matches(const char *p,const char *buf,int len) {
    char b[4096];int fd=open(p,O_RDONLY);assert(fd>=0);
    assert(read(fd,b,sizeof(b))==len);assert(!memcmp(buf,b,len));close(fd);
}
int main(void) {
    char dir[]="/tmp/cmdr-faults-XXXXXX",src[256],dst[256],buf[3072];int fd,i;
    assert(mkdtemp(dir));assert(joinpath(src,dir,"src")==0);assert(joinpath(dst,dir,"dst")==0);source=src;
    for(i=0;i<(int)sizeof(buf);i++)buf[i]=(char)i;
    fd=open(src,O_WRONLY|O_CREAT|O_EXCL,0600);assert(fd>=0);assert(write(fd,buf,sizeof(buf))==sizeof(buf));close(fd);
    fault=1;assert(copyfile(src,dst,0)<0);assert(errno==ENOSPC);assert(access(dst,F_OK)<0);matches(src,buf,sizeof(buf));
    fault=2;assert(copyfile(src,dst,0)==0);matches(dst,buf,sizeof(buf));unlink(dst);
    fault=3;assert(copyfile(src,dst,1)<0);assert(errno==EACCES);matches(src,buf,sizeof(buf));matches(dst,buf,sizeof(buf));
    fault=0;unlink(src);unlink(dst);
    {
        struct panel panel={0};char path[PATH_CAP],name[20];
        strcpy(panel.path,dir);
        for(i=0;i<100;i++) {
            snprintf(name,sizeof(name),"f%03d",i);assert(joinpath(path,dir,name)==0);
            fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0600);assert(fd>=0);close(fd);
        }
        fault=4;assert(loadpanel(&panel)==0);assert(panel.limited&LIMIT_MEMORY);
        assert(panel.count==5);assert(!panel.failed);
        fault=0;assert(loadpanel(&panel)==0);assert(panel.count==101 && !panel.limited);
        panel.selected=100;
        assert(loadpanel(&panel)==0);assert(!strcmp(panel.entries[panel.selected].name,"f099"));
        fault=6;assert(loadpanel(&panel)==0);assert(panel.count==32 && (panel.limited&LIMIT_MEMORY));
        assert(panel.entries[0].name && !strcmp(panel.entries[0].name,".."));
        fault=5;assert(loadpanel(&panel)==0);assert(panel.count==0 && (panel.limited&LIMIT_MEMORY));
        fault=0;assert(loadpanel(&panel)==0);assert(panel.count==101 && !panel.limited);
        freepanel(&panel);freepanel(&panel);
        for(i=0;i<100;i++) {
            snprintf(name,sizeof(name),"f%03d",i);assert(joinpath(path,dir,name)==0);unlink(path);
        }
    }
    rmdir(dir);
    puts("PASS: disk-full rollback, short writes, move source-removal failure keeps both complete files; allocation failure/recovery and selection retention");return 0;
}
