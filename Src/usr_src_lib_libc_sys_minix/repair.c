#include <lib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int inodewalker(){
    message m;
    
    memset(&m,0,sizeof(m));

    int x=_syscall(VFS_PROC_NR,106,&m);
    
    return x;
}


int zonemapwalker(int * r){
    message m;
    m.m1_i1=r;
    m.m1_i2=0;
    
    int x=_syscall(VFS_PROC_NR,109,&m);
    
    return x;
}

/*
int inodefixer(int n){
    message m;
    m.m1_i1=NULL;
    int x=_syscall(VFS_PROC_NR,108,&m);

    message m2;
    m2.m1_i1=NULL;
    m2.m1_i2=n;
    int x=_syscall(VFS_PROC_NR,119,&m2)
    return x;
}

*/



