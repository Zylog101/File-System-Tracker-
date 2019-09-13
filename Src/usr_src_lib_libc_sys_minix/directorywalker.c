#include<unistd.h>
#include<lib.h>
#include<stdio.h>
#include<string.h>
//creates and sends message to directory walker
void DirectoryWalker(char *pathName)
{
	message m;
	m.m1_i1=strlen(pathName);
	m.m1_p1=pathName;
	_syscall(VFS_PROC_NR,DIRECTORYWALKER,&m);
}
