#include "fs.h"
#include <sys/stat.h>
#include <string.h>
#include <minix/com.h>
#include "buf.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <sys/param.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
/* Defines */
#define EXIT_OK                    0
#define EXIT_USAGE                 1
#define EXIT_UNRESOLVED            2
#define EXIT_ROOT_CHANGED          4
#define EXIT_CHECK_FAILED          8
#define EXIT_SIGNALED             12
#define INDCHUNK	((int) (CINDIR * ZONE_NUM_SIZE))

/* Global variables */
bitchunk_t *imap_disk;			 /* imap from the disk */
bitchunk_t *zmap_disk;			 /* zmap from the disk */
static struct super_block *sb;   /* super block */
unsigned int WORDS_PER_BLOCK;    /* # words in a block */
unsigned int BLK_SIZE;			 /* block size */
int NATIVE;
bit_t ORIGIN_IMAP;		 		 /* sb->s_isearch */
bit_t ORIGIN_ZMAP;		 		 /* sb->s_zsearch */
zone_t  FIRST;					 /* first data zone */
block_t BLK_IMAP;			 	 /* starting block for imap */
block_t BLK_ZMAP;			 	 /* starting block for zmap */
block_t BLK_ILIST;			 	 /* starting block for inode table */
unsigned int N_IMAP;			 /* # blocks used for imap */
unsigned int N_ZMAP;			 /* # blocks used for zmap */
unsigned int N_ILIST;			 /* # blocks used for inode table */
unsigned int NB_INODES;			 /* # inodes */
unsigned int NB_ZONES;			 /* # zones */
unsigned int NB_USED = 0;		 /* # used (zones or inodes) */
unsigned int NB_INODES_USED = 0; /* # zones used (from IMAP) */
unsigned int NB_ZONES_USED_Z = 0;/* # zones used (from ZMAP) */
unsigned int NB_ZONES_USED_I = 0;/* # zones used (from IMAP) */

int repair    = 0;
int markdirty = 0;
int type = 0;

bitchunk_t *alloc_bitmap(int nblk);
void lsuper();
char *int2binstr(unsigned int i);
void free_bitmap(bitchunk_t *p);
int* get_list_used(bitchunk_t *bitmap, int type);
void init_global();
void get_bitmap(bitchunk_t *bitmap, int type);
void traverseDirectory(struct inode *resultingINode);
int fs_zonemapwalker();
int fs_inodewalker();
int fs_inodedamage();
int fs_dirwalker();
int fs_inodefixer();
void fatal(char *s);
struct inode * findSpecificDirectory( struct inode *rip,char *userPath);


/*===========================================================================*
   *				fatal			     		*
   *===========================================================================*/
void fatal(s)
char *s;
{
    	/* Print the string `s' and exit. */
    	printf("%s\nfatal\n", s);
    	exit(EXIT_CHECK_FAILED);
}


/*===========================================================================*
   *				alloc			     		*
   *===========================================================================*/
char *alloc(nelem, elsize)
unsigned nelem, elsize;
{
    	/* Allocate some memory and zero it. */
    	char *p;
    
    	if ((p = (char *)malloc((size_t)nelem * elsize)) == 0) {
        		fatal("out of memory");
        	}
    	memset((void *) p, 0, (size_t)nelem * elsize);
    	return(p);
    }
/*===========================================================================*
   *				alloc_bitmap	     		*
   *===========================================================================*/
bitchunk_t *alloc_bitmap(nblk)
int nblk;
{
      /* Allocate `nblk' blocks worth of bitmap. */
      register bitchunk_t *bitmap;
      bitmap = (bitchunk_t *) alloc((unsigned) nblk, BLK_SIZE);
      *bitmap |= 1;
      return bitmap;
    }



/*===========================================================================*
   *				int2binstr		     		*
   *===========================================================================*/
char *int2binstr(unsigned int i)
{
    	/* Convert an int to a binary string */
    	size_t bits = sizeof(unsigned int) * CHAR_BIT;
    	char * str = malloc(bits + 1);
    	if(!str) return NULL;
    	str[bits] = 0;
    
    	unsigned u = *(unsigned *)&i;
    	for(; bits--; u >>= 1)
        		str[bits] = u & 1 ? '1' : '0';
        
        	return str;
    }


/*===========================================================================*
   *				get_bitmap	         		*
   *===========================================================================*/
void get_bitmap(bitmap, type)
bitchunk_t *bitmap;
int type;
{
    	/* Get a bitmap (imap or zmap) from disk */
    	block_t *list;
    	block_t bno;
    	int nblk;
    	if (type == IMAP){
        		bno  = BLK_IMAP;
        		nblk = N_IMAP;
        	}
    	else /* type == ZMAP */ {
        		bno  = BLK_ZMAP;
        		nblk = N_ZMAP;
        	}
    	register int i;
    	register bitchunk_t *p;
    	register struct buf *bp;
    	register bitchunk_t *bf;
    	p = bitmap;
    	for (i = 0; i < nblk; i++, bno++, p += FS_BITMAP_CHUNKS(BLK_SIZE)){
        		bp = get_block(fs_dev, bno, 0);
        		for (int j = 0; j < FS_BITMAP_CHUNKS(BLK_SIZE); ++j){
            			p[j]  = b_bitmap(bp)[j];
            		}
        	}
    }


/*===========================================================================*
   *				init_global		     *
   *===========================================================================*/
void init_global()
{
    	/* Initialize all global variables for convenience of names */
    	BLK_SIZE        = sb->s_block_size;
    	FIRST       	= sb->s_firstdatazone;
    	NB_INODES		= sb->s_ninodes;
    	NB_ZONES		= sb->s_zones;
    	N_IMAP 			= sb->s_imap_blocks;
    	N_ZMAP 			= sb->s_zmap_blocks;
    	N_ILIST			= (sb->s_ninodes + V2_INODES_PER_BLOCK(BLK_SIZE)-1) / V2_INODES_PER_BLOCK(BLK_SIZE);
    	ORIGIN_IMAP		= sb->s_isearch;
    	ORIGIN_ZMAP		= sb->s_zsearch;
    	NATIVE			= sb->s_native;
    	BLK_IMAP 		= 2;
    	BLK_ZMAP 		= BLK_IMAP + N_IMAP;
    	BLK_ILIST 		= BLK_ZMAP + N_ZMAP;
    	WORDS_PER_BLOCK = BLK_SIZE / (int) sizeof(bitchunk_t);
    }

/*===========================================================================*
   *				get_list_used	     *
   *===========================================================================*/
int* get_list_used(bitchunk_t *bitmap, int type)
{
    	/* Get a list of unused blocks/inodes from the zone/inode bitmap */
    	int* list;
    	int nblk;
    	int tot;
    	bitchunk_t *buf;
    	char* chunk;
    	NB_USED = 0;
    	if (type == IMAP){
        		nblk = N_IMAP;
        		tot  = NB_INODES;
        		list = malloc(sizeof(int)*NB_INODES);
        		printf("============= Used inodes ==============\n");
        	}
    	else /* type == ZMAP */ {
        		nblk = N_ZMAP;
        		tot  = NB_ZONES;
        		list = malloc(sizeof(int)*NB_ZONES);
        		printf("============= Used blocks ==============\n");
        	}
    	sleep(1);
    	printf("\n=========================================\n");
    	/* Loop through bitchunks in bitmap */
    	for (int j = 0; j < FS_BITMAP_CHUNKS(BLK_SIZE)*nblk; ++j){
        		chunk = int2binstr(bitmap[j]);
        		/* Loop through bits in bitchunk */
        		for (int k = 0; k < strlen(chunk); ++k){
            			if (chunk[k] == '1'){
                				list[NB_USED] = j*FS_BITCHUNK_BITS + k;
                				printf("%d, ", list[NB_USED]);
                				if (NB_USED % 10 == 0){
                    					printf("\n");
                    				}
                				++NB_USED;
                			}
            		}
        	}
    	if (type == IMAP)    NB_INODES_USED  = NB_USED;
        	else/*(type==ZMAP)*/ NB_ZONES_USED_Z = NB_USED;
            	printf("\n=====================================================\n\n");
            	printf("Used: %d / %d \n", NB_USED, tot);
            	return list;
    }

/*===========================================================================*
   *				free_bitmap		     		*
   *===========================================================================*/
void free_bitmap(p)
bitchunk_t *p;
{
      /* Deallocate the bitmap `p'. */
      free((char *) p);
    }
/*===========================================================================*
   *				lsuper			     	    *
   *===========================================================================*/
void lsuper()
{
    	/* Make a listing of the super block. */
    	printf("ninodes       = %u\n", sb->s_ninodes);
    	printf("nzones        = %d\n", sb->s_zones);
    	printf("imap_blocks   = %u\n", sb->s_imap_blocks);
    	printf("zmap_blocks   = %u\n", sb->s_zmap_blocks);
    	printf("firstdatazone = %u\n", sb->s_firstdatazone_old);
    	printf("log_zone_size = %u\n", sb->s_log_zone_size);
    	printf("maxsize       = %d\n", sb->s_max_size);
    	printf("block size    = %d\n", sb->s_block_size);
    	printf("flags         = ");
    	if(sb->s_flags & MFSFLAG_CLEAN) printf("CLEAN "); else printf("DIRTY ");
        	printf("\n\n");
        }



int get_name_custom(char *path_name, char *string)
{
  size_t len;
  char *cp, *ep;
  int numberOfCharsDone=0;

  cp = path_name;

  /* Skip leading slashes */
  while (cp[0] == '/'){ cp++;numberOfCharsDone++;}

  /* Find the end of the first component */
  ep = cp;
  while(ep[0] != '\0' && ep[0] != '/')
  {
	ep++;numberOfCharsDone++;
  }

  len = (size_t) (ep - cp);

 strncpy(string, cp, len);
 string[len]= '\0';
return numberOfCharsDone;
}

int fs_dirwalker()
{
  printf("\nfs_direwalker in mfs\n");
  cp_grant_id_t grant;
  struct inode *rip;
  int r;
  unsigned int len;
  ino_t dir_ino, root_ino;
  char path[MFS_NAME_MAX];
  struct inode *resultingINode=NULL;

  grant		= (cp_grant_id_t) fs_m_in.REQ_GRANT;
  len		= (int) fs_m_in.REQ_PATH_LEN; /* including terminating nul */
  dir_ino	= (ino_t) fs_m_in.REQ_DIR_INO;
  root_ino	= (ino_t) fs_m_in.REQ_ROOT_INO;
 
  //Copy the link name's last component 
  r = sys_safecopyfrom(VFS_PROC_NR, (cp_grant_id_t) grant,
  		       (vir_bytes) 0, (vir_bytes) path, (size_t) len);
  if (r != OK) 
	return r;
  else 
	if(path[len]=='\0')
	printf("\n copied path %s \n",path);

  /* Find starting inode inode according to the request message */
  if((rip = find_inode(fs_dev, dir_ino)) == NULL) 
	printf("\nfailed to find inode\n");

 if(len==2)
{
	traverseDirectory(rip);
	return 1;
}
 resultingINode=findSpecificDirectory(rip,path);
 if(resultingINode!=NULL)
{
	traverseDirectory(resultingINode);
}
 return 1;
}

ino_t searchDir(struct inode *rip,char *comp)
{
  register struct buf *bp = NULL;
  register struct direct *dp = NULL;

  unsigned new_slots, old_slots;
 
  // Step through the directory one block at a time. 
  old_slots = (unsigned) (rip->i_size/DIR_ENTRY_SIZE);
  new_slots = 0;

  //trying to traverse the path
  if ( (rip->i_mode & I_TYPE) != I_DIRECTORY)
  {
 	 printf("\nIt not a directory\n");
	 return 999999;	
  }
	printf("Searching Component:%s\n",comp);
  off_t pos=0;

  for (; pos < rip->i_size; pos += rip->i_sp->s_block_size) 
  {
	bp = get_block_map(rip, pos);
	if(bp==NULL)
  	{
	     printf("\nunable to retrieve block\n");
 	}

	for (dp = &b_dir(bp)[0];dp < &b_dir(bp)[NR_DIR_ENTRIES(rip->i_sp->s_block_size)];dp++)
	{	
		if (++new_slots > old_slots)
		{ 
			break;
		}
		if(strcmp(dp->mfs_d_name,comp)==0)
		{
			printf("found element %s and inode number %u\n",dp->mfs_d_name,dp->mfs_d_ino);
			
			return dp->mfs_d_ino;
		}
		else
		{
			printf("%s\n",dp->mfs_d_name);
		}
	}
   }
  
  return 999999;
  
}
struct inode * findSpecificDirectory( struct inode *rip,char *userPath)
{
   char component[20];
   char *ch;
   int len=strlen(userPath);

   while(len!=0)
   {
   	int numberOfCharsToMove=get_name_custom(userPath,component);
	userPath+=numberOfCharsToMove;
	len-=numberOfCharsToMove;
	printf("component:%s\n",component);
	ino_t res=searchDir(rip,component);
	if (res==999999)
	{
		printf("\ninvalid path\n");
		return NULL;
	}
	else
	{
		 /* Find inode according to the request message */
  		if((rip = find_inode(fs_dev, res)) == NULL) 
		{
			printf("\nfailed to find inode %u\n",res);
			return NULL;
		}
		else
		{
			traverseDirectory(rip);
		}
	}
	
   }
   return rip;
}

void traverseDirectory(struct inode *rip)
{
  register struct buf *bp = NULL;
  register struct direct *dp = NULL;

  unsigned new_slots, old_slots;
  
  // Step through the directory one block at a time. 
  old_slots = (unsigned) (rip->i_size/DIR_ENTRY_SIZE);
  new_slots = 0;

  off_t pos=0;
  int match=0;
  for (; pos < rip->i_size; pos += rip->i_sp->s_block_size) 
  {
	bp = get_block_map(rip, pos);
	if(bp==NULL)
  	{
	     printf("\nunable to retrieve block\n");
 	}

	for (dp = &b_dir(bp)[0];dp < &b_dir(bp)[NR_DIR_ENTRIES(rip->i_sp->s_block_size)];dp++)
	{	
		if (++new_slots > old_slots)
		{ 
			break;
		}
		printf("\n----------------------------------------------------\n");
		printf("directory entry Inode: %u\tname: %s",dp->mfs_d_ino,dp->mfs_d_name);
		printf("\n----------------------------------------------------\n");
		int scale;
		unsigned long excess, zone, block_pos;
		scale = rip->i_sp->s_log_zone_size;
		block_pos = pos/rip->i_sp->s_block_size;
		zone = block_pos >> scale;	/* position's zone */
		printf("%lu",zone );
		sleep(1);
	}
 }

}

int fs_zonemapwalker(){
    /* Get the list of blocks used by the system from the zone bitmap */
    printf("=== ZONEWALKER in MFS===\n");
    printf("Getting super node from device %u ...\n", fs_dev);
    type = ZMAP;
    sb = get_super(fs_dev);
    read_super(sb);
    lsuper();
    sleep(3);
    init_global();
    zmap_disk = alloc_bitmap(N_ZMAP);
    printf("Loading zone bitmap from disk ...\n");
    get_bitmap(zmap_disk, ZMAP);
    printf(" done.\n\n");
    sleep(3);
    //print_bitmap(zmap_disk);
    int* list = get_list_used(zmap_disk, ZMAP);
    free_bitmap(zmap_disk);
    return 0;
}


int fs_inodewalker(){

/* Get the list of blocks in use by the system from the inode bitmap*/
    	printf("=== INODEWALKER in MFS ===\n");
    	printf("Getting super node from device %u ...\n", fs_dev);
        type = IMAP;
    	sb = get_super(fs_dev);
    	read_super(sb);
    	lsuper();
    	init_global();
    	imap_disk = alloc_bitmap(N_IMAP);
    	printf("Loading inode bitmap from disk ...\n");
    	get_bitmap(imap_disk, IMAP);
    	printf(" done.\n");
    	sleep(3);
    	//print_bitmap(imap_disk);
    	int *list_inodes = get_list_used(imap_disk, IMAP);
    	sleep(5);
//    	int *list_blocks;
//    	if ((list_blocks = get_list_blocks_from_inodes(list_inodes)) == NULL)
//        		return -1;
    	free_bitmap(imap_disk);
    	return 0;
}


/*flipbitInInodeBitmapAtIndex(DamagedInode)
{
	
}

*/
int fs_inodedamage(){
printf("\nfs_inodedamage\n");
register struct inode *rip;
	
TAILQ_FOREACH(rip,&unused_inodes,i_unused)
{
	//if(DamagedInode==rip->i_num)
	//flipbitInInodeBitmapAtIndex(DamagedInode);
	printf("%u ",rip->i_num);
}
printf("\n");
for (int i=0;i<NR_INODES;i++)
{
	if(inode[i].i_num!=0)
	printf("%u,%u ",inode[i].i_num,inode[i].i_nlinks);
}
return 0;
/*
	fprintf(stderr, "fs_inodedamage\n");

	//struct super_block * sp=get_super(fs_m_in.REQ_DEV);

	block_numbers=calloc(9,4);
	int index=0;

    broken_inodeNumber = fs_m_in.REQ_INODE_NR;

    struct inode * ino = get_inode(fs_m_in.REQ_DEV,broken_inodeNumber);

    int j;
    for(j=0;j<9;j++){
        if(ino->i_zone[j]!=0){
            block_numbers[index] = ino->i_zone[j];
            ino->i_zone[j] = 0;
            index++;
        }
    }
    put_inode(ino);

	printf("test: %d, %d, %d\n",block_numbers[0],block_numbers[1],block_numbers[2]);
	printf("index: %d\n",index);

	fs_m_out.RES_DEV=(int)block_numbers;
	fs_m_out.RES_NBYTES=index*4;


	return 0;*/
}




int fs_inodefixer(){
printf("\nfs_inodefixer\n");return 0;
/*
	fprintf(stderr, "fs_inodeFixer\n");

	int index=0;

	printf("test:  %d  %d  \n",block_numbers[0],lost_blocks[0]);
	broken_inodeNumber = fs_m_in.REQ_INODE_NR;
	if(broken_inodeNumber==0)return 0;

    struct inode * ino = get_inode(fs_m_in.REQ_DEV,broken_inodeNumber);

    int j;
    for(j=0;j<7;j++){
        if(ino->i_zone[j]==0){
            ino->i_zone[j]= lost_blocks[index] ;
            index++;
        }
    }
    if(lost_blocks[8]!=0) ino->i_zone[7]=lost_blocks[8];
    if(lost_blocks[1033]!=0) ino->i_zone[8]=lost_blocks[1033];
    put_inode(ino);

	printf("done\n");
	free(lost_blocks);
	free(block_numbers);

	return 0;*/
}


/*

dp++;
printf("\n----------------------------------------------------\n");
printf("directory entry Inode: %u\tname: %s",dp->mfs_d_ino,dp->mfs_d_name);
printf("\n----------------------------------------------------\n");
dp++;
printf("\n----------------------------------------------------\n");
printf("directory entry Inode: %u\tname: %s",dp->mfs_d_ino,dp->mfs_d_name);
printf("\n----------------------------------------------------\n");

dp++;
printf("\n----------------------------------------------------\n");
printf("directory entry Inode: %u\tname: %s",dp->mfs_d_ino,dp->mfs_d_name);
printf("\n----------------------------------------------------\n");


  struct inode  *drip ;
  register int r;
  char path[MFS_NAME_MAX];
 // struct inode *new_ip;
ino_t numb;

  phys_bytes len;

 // len = min( (unsigned) fs_m_in.REQ_PATH_LEN, sizeof(path));

   Copy the link name's last component 
  r = sys_safecopyfrom(VFS_PROC_NR, (cp_grant_id_t) grant,
  		       (vir_bytes) 0, (vir_bytes) path, (size_t) len);

 // if (r != OK) return r;
	printf("\n safecopy returned:%d \n vfs-mfs copied path %s \n",r,path);


	//getting inode from the inode number passed 
	if( (drip = get_inode(fs_dev, (ino_t) fs_m_in.REQ_INODE_NR)) == NULL)
	  return(EINVAL);
 if ( (drip ->i_mode & I_TYPE) == I_DIRECTORY)  {
	printf("Its is an directory");
   }
	//printf("Its is an directory");
  //if ( (err_code = search_dir(drip , path, &numb, LOOK_UP, CHK_PERM)) != OK) {
//	return(0);
//}
	printf("\n err_code=%d ok=%d inode searched %d\n",err_code,OK,numb);
//printf("\n retrieved inode :%d and the one sent :%ld\n",rip->i_num,fs_m_in.REQ_INODE_NR);
//return 
	return 1;
*/
/*
  register struct buf *bp = NULL;
  register struct direct *dp = NULL;

  unsigned new_slots, old_slots;
  //splitPath(path);
  // Step through the directory one block at a time. 
  old_slots = (unsigned) (rip->i_size/DIR_ENTRY_SIZE);
  new_slots = 0;

  //trying to traverse the path
  if ( (rip->i_mode & I_TYPE) == I_DIRECTORY)
  {
 	 printf("\nIt is a directory\n");	
  }
  off_t pos=0;
  int match=0;
  for (; pos < rip->i_size; pos += rip->i_sp->s_block_size) 
  {
	bp = get_block_map(rip, pos);
	if(bp==NULL)
  	{
	     printf("\nunable to retrieve block\n");
 	}

	for (dp = &b_dir(bp)[0];dp < &b_dir(bp)[NR_DIR_ENTRIES(rip->i_sp->s_block_size)];dp++)
	{	
		if (++new_slots > old_slots)
		{ 
			break;
		}
		if(strcmp(dp->mfs_d_name,path)==0)
		{
			match=1;
			return dp->mfs_d_ino;
			
		}
		printf("\n----------------------------------------------------\n");
		printf("directory entry Inode: %u\tname: %s",dp->mfs_d_ino,dp->mfs_d_name);
		printf("\n----------------------------------------------------\n");
		struct inode *inodePointer = find_inode(fs_dev, dir_ino);
		u32_t *inodeZonePointer=inodePointer->i_zone;
		for(int i=0;i<V2_NR_TZONES;i++)
		{
			printf("%u",inodeZonePointer[i]);
		}
		sleep(3);
	}
 }
	if(match==0)
*/





