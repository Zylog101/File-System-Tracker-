#include <sys/cdefs.h>
#include <lib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(void){
    	printf("\n================== CS551: TEST PROGRAM FOR FILE SYSTEMS ==================\n");
    	printf("This program will allow you to test the inodeBitMapWalker\n");
    	int * e=calloc(1017088,4);
        zonemapwalker(e);
    	return 0;
    }
