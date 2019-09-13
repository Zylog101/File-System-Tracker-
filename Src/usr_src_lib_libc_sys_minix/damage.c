#include <lib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>



int inodedamage(int * r, int inodeNb){
    message m;
    m.m1_i1=r;
    m.m1_i2=inodeNb;
    printf("avant syscall\n");
    int x=_syscall(VFS_PROC_NR,110,&m);
    printf("apres syscall\n");
    return x;
}
