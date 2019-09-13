#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include "fs.h"
#include <sys/stat.h>
#include <string.h>
#include <minix/com.h>
#include <minix/callnr.h>
#include <minix/vfsif.h>
#include <dirent.h>
#include <assert.h>
#include "file.h"
#include "fproc.h"
#include "path.h"
#include "vnode.h"
#include "param.h"
#include "scratchpad.h"

int do_inodewalker(message *UNUSED(m_out));
int do_zonemapwalker(message *UNUSED(m_out));
int do_inodedamage(message *UNUSED(m_out));
//assumes path is given as such /usr/src
int do_dirWalker(message *UNUSED(m_out))
{
	char fullpath[PATH_MAX];
	vir_bytes vname1;
 	size_t vname1_length;
	//struct vnode *vp = NULL;
	//struct vmnt *vmp1 = NULL;
	
	//fetch name from sys call
	vname1 = (vir_bytes) job_m_in.m1_p1;
	vname1_length=job_m_in.m1_i1;

	if(sys_datacopy(who_e, (vir_bytes)vname1, SELF,
	(vir_bytes)fullpath , vname1_length+1)==OK)
	{
		fullpath [vname1_length+1]='\0';
		printf("copied name :%s\n",fullpath);
	}
	else 
		printf("unable to copy");
	 	
	ino_t dir_ino=fp->fp_rd->v_inode_nr;
	endpoint_t fs_e=fp->fp_rd->v_fs_e;
	//ino_t root_ino=fp->fp_rd->v_inode_nr;

	req_dirwalker(fs_e,fullpath,dir_ino);	
	return 0;
}

int RC_CODE;

int do_inodedamage(message *UNUSED(m_out))
{

    printf("successfully called vfs inodedamage...\n");

    struct vmnt *vmp;

    char * dest=(char *)m_in.m1_i1;
    //endpoint_t w=m_in.m_source;
    printf("dest, entering vfs: %d\n",(int)dest);

    for (vmp = &vmnt[0]; vmp < &vmnt[NR_MNTS]; ++vmp) {
    	if ( strcmp("/home", vmp->m_mount_path) == 0 ) {
            printf("found home directory");
            printf(" number of devices: %d", vmp->m_dev);
            message m;
            m.m_type = REQ_INODEDAMAGE;
            m.REQ_DEV = vmp->m_dev;
            m.REQ_INODE_NR = m_in.m1_i2;

            printf("entering mfs");
            RC_CODE = fs_sendrec(vmp->m_fs_e, &m);

           // int size=m.RES_NBYTES;

           // int * blocks=malloc(size);
           // if(sys_datacopy(m.m_source, (vir_bytes)m.RES_DEV, SELF, (vir_bytes)blocks, size)==OK)printf("Copy1 ok\n");
          //  printf("test copy1: %ld %d  %d  %d\n",m.RES_DEV,blocks[0],blocks[1],blocks[2]);

           // if(sys_datacopy(SELF, (vir_bytes)blocks, w , (vir_bytes)dest, size)==OK)printf("copy2 OK\n");

        }
    }


    return 0;
}


int do_inodewalker(message *UNUSED(m_out)){
        printf("DO_INODEWALKER from VSF\n");
    	int r;
    	struct vmnt *vmp;
    	struct vnode *vp;
    	struct lookup resolve;
    	char fullpath[PATH_MAX] = "/";
    
    	/* Get a virtual inode and virtual mount corresponding to the path */
    	lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
    	resolve.l_vmnt_lock = VMNT_READ;
    	resolve.l_vnode_lock = VNODE_READ;
    	if ((vp = last_dir(&resolve, fp)) == NULL) return(err_code);
    
    	/* Emit a request to FS */
    	//r = req_inodewalker(vmp->m_fs_e);
        message m;
        m.m_type = REQ_INODEWALKER;
    
        r = fs_sendrec(vmp->m_fs_e, &m);
    
    	/* Unlock virtual inode and virtual mount */
    	unlock_vnode(vp);
    
    	unlock_vmnt(vmp);
    
    	put_vnode(vp);
    
    
    	return r;
}


int do_zonemapwalker(message *UNUSED(m_out)){
    printf("DO_ZONEMAPWALKER from VSF\n");
    int r;
    struct vmnt *vmp;
    struct vnode *vp;
    struct lookup resolve;
    char fullpath[PATH_MAX] = "/";
    
    /* Get a virtual inode and virtual mount corresponding to the path */
    lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
    resolve.l_vmnt_lock = VMNT_READ;
    resolve.l_vnode_lock = VNODE_READ;
    if ((vp = last_dir(&resolve, fp)) == NULL) return(err_code);
    message m;
    m.m_type = REQ_ZONEMAPWALKER;
    
    r = fs_sendrec(vmp->m_fs_e, &m);
    /* Emit a request to FS */
        
    /* Unlock virtual inode and virtual mount */
    unlock_vnode(vp);
    
    unlock_vmnt(vmp);
    
    put_vnode(vp);
    
    return r;
}

