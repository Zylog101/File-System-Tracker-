#include <sys/cdefs.h>
#include <lib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


void print_menu(void);

int main(void){
    	printf("\n================== CS551: TEST PROGRAM FOR FILE SYSTEMS ==================\n");
    	printf("This program will allow you to test the following.\n");
    	while(TRUE){
        		print_menu();
        	}
    	return 0;
    }

void print_menu(void){
    	int inode_nb;
    	int choice;
    	char dir_path[128];
        char dir_name[128];
    	printf("\nMENU\n");
        printf("1. Directory Walker\n");
    	printf("2. inodeBitMap Walker\n");
    	printf("3. ZoneBitMap Walker\n");
    	printf("4. Repair Tool\n");
    	printf("5. Damage Inode Bit Map\n");
        printf("6. Damage Zone Bit Map\n");
    	printf("7. Damage directory file\n");
        printf("8. Damage Inode of a Directory File\n");
        printf("9. Exit\n");
    	scanf("%d", &choice);
    	getchar();
    	switch(choice){

                    case 1:
                        printf("\nDirectory Walker\n");
                        printf("Directory: ");
                        char *ch = (char *)malloc(100);
                        scanf("%s",ch);
                        DirectoryWalker(ch);
                        break;
            		case 2:
            			printf("\nINODE WALKER\n");
            			inodewalker();
            			break;
                    case 3:
                        printf("\nZONE WALKER\n");
                        int * e=calloc(1017088,4);
                        zonemapwalker(e);
                        break;

                    case 5:
            			printf("\nDAMAGE INODE\n");
            			int d;
                        printf("Inode to damage: ");
                        scanf("%d",&d);

                        int * r=calloc(9,4);
                        int x=inodedamage(r,d);

/*
	//printf("x: %d\n",x);
	//printf("r: %d  %d  %d\n",r[0],r[1],r[2]);

				FILE * file=fopen("blocks.txt","w");

				int i=0;
				while(r[i]!=0){
				i++;
				}
    				printf("number of blocks referenced by inodes: %d",i);
    				i=0;
				while(r[i]!=0){
     				fprintf(file,"%d",r[i]);
				i++;
				}

				fclose(file);
				free(r);
				printf("Blocks referenced by inodes saved in blocks.txt\n");
            	  		break;
            	  	case 3:
            		    printf("\nZONE WALKER\n");
//            			int * e=calloc(1017088,4);

	//			x=zonemapwalker(e);
	//printf("x: %d\n",x);
	//printf("e: %d  %d  %d\n",e[0],e[1],e[2]);

				FILE * file2=fopen("blocks2.txt","w");

				int j=0;
				while(e[j]!=0){
				j++;
				}
   				printf("number of blocks referenced by zonemap: %d",i-j);
   				printf("difference with inodewalker:");
  				j=0;
				while(e[j]!=0){
       				fprintf(file,"%d",e[j]);
				j++;
				}

				fclose(file2);
				free(e);
				printf("Blocks referenced by zonemap saved in blocks2.txt\n");

            			break;
            		case 4:
            			printf("REPAIR InodeDamage\n");
            			printf("Inode to be repaired: ");
    				int n;
    				scanf("%d",&n);
    				int x=inodefixer(n);

    				printf("Inode fixed...\n"); 
           			break;
            		case 5:
            			printf("\nDAMAGE INODE\n");
            			int d;
   				printf("Inode to damage: ");
   				scanf("%d",&d);

				int * r=calloc(9,4);
				int x=inodedamage(r,d);

				 printf("x: %d\n",x);
				printf("r: %d  %d  %d\n",r[0],r[1],r[2]);

				FILE * file=fopen("Dblocks.txt","w");

				int i=0;
				while(r[i]!=0&&i<9){
       				i++;
				}
    				fprintf(file,"number of block references erased: %d\n",i);
    				i=0;
				while(r[i]!=0&&i<9){
       				fprintf(file,"%d\n",r[i]);
       				i++;
				}

				fclose(file);
				free(r);

				printf("Block numbers of damaged file saved in Dblocks.txt\n");

				

            			break;
                    case 6:
                        printf("\nDAMAGE ZONE\n");
                        printf("Inode number: ");
            	//		scanf("%d", &inode_nb);
            	//		damage_zone(inode_nb);
                        break;
            		case 7:
            			printf("\nDAMAGE DIRECTORY FILE\n");
            			printf("Directory: ");
//            			scanf("%s", dir_path);
//            			damage_dir(dir_path);
            			break;
                    case 8:
                        printf("\nDAMAGE iNODE OF A DIRECTORY FILE\n");
                        printf("Directory: ");
//            			scanf("%s", dir_path);
           			damage_iNode_dir(dir_path);*/ 
                        break;
                    case 9:
                        exit(0);
            	
        	}
    }
