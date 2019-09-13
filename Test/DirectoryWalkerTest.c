#include <sys/cdefs.h>
#include <lib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int main(void){
    	printf("\n================== CS551: TEST PROGRAM FOR FILE SYSTEMS ==================\n");
    	printf("This program will allow you to perform directory walk\n");
    	 printf("Enter Directory: ");
                        char *ch = (char *)malloc(100);
                        scanf("%s",ch);
                        DirectoryWalker(ch);
    	return 0;
    }

