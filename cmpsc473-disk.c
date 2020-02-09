
/**********************************************************************

   File          : cmpsc473-disk.c

   Description   : File system function implementations
                   (see .h for applications)

***********************************************************************/
/**********************************************************************
Copyright (c) 2019 The Pennsylvania State University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of The Pennsylvania State University nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

/* Include Files */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Project Include Files */
#include "cmpsc473-filesys.h"
#include "cmpsc473-disk.h"
#include "cmpsc473-list.h"
#include "cmpsc473-util.h"

/* Definitions */

/* program variables */

/* Functions */


/**********************************************************************

    Function    : diskDirInitialize
    Description : Initialize the root directory on disk
    Inputs      : directory reference
    Outputs     : 0 if success, -1 on error

***********************************************************************/

int diskDirInitialize( ddir_t *ddir )
{
	/* Local variables */
	dblock_t *first_dentry_block;
	int i;
	ddh_t *ddh; 

	/* clear disk directory object */
	memset( ddir, 0, FS_BLOCKSIZE );

	/* initialize disk directory fields */
	ddir->buckets = ( FS_BLOCKSIZE - sizeof(ddir_t) ) / (sizeof(ddh_t));
	ddir->freeblk = FS_METADATA_BLOCKS+1;    /* fs+super+directory */
	ddir->free = 0;                          /* dentry offset in that block */

	/* assign first dentry block - for directory itself */
	first_dentry_block = (dblock_t *)disk2addr( fs->base, 
						    (( FS_METADATA_BLOCKS+1 ) 
						     * FS_BLOCKSIZE ));
	memset( first_dentry_block, 0, FS_BLOCKSIZE );
	first_dentry_block->free = DENTRY_BLOCK;
	first_dentry_block->st.dentry_map = DENTRY_MAP;
	first_dentry_block->next = BLK_INVALID;

	/* initialize ddir hash table */
	ddh = (ddh_t *)ddir->data;     /* start of hash table data -- in ddh_t's */
	for ( i = 0; i < ddir->buckets; i++ ) {
		(ddh+i)->next_dentry = BLK_SHORT_INVALID;
		(ddh+i)->next_slot = BLK_SHORT_INVALID;
	}

	return 0;  
}


/**********************************************************************

    Function    : diskReadDir
    Description : Retrieve the on-disk directory -- only one in this case
    Inputs      : name - name of the file 
                  name_size - length of name
    Outputs     : on-disk directory or -1 if error

***********************************************************************/

ddir_t *diskReadDir( char *name, unsigned int name_size ) 
{
	return ((ddir_t *)block2addr( fs->base, dfs->root ));
}


/**********************************************************************

    Function    : diskFindDentry
    Description : Retrieve the on-disk dentry from the disk directory
    Inputs      : diskdir - on-disk directory
                  name - name of the file 
                  name_size - length of file name
    Outputs     : on-disk dentry or NULL if error

***********************************************************************/

ddentry_t *diskFindDentry( ddir_t *diskdir, char *name, unsigned int name_size ) 
{
	int key = fsMakeKey( name, diskdir->buckets, name_size );
	ddh_t *ddh = (ddh_t *)&diskdir->data[key];

	// find block in cache?  if not get from disk and put in cache

	while (( ddh->next_dentry != BLK_SHORT_INVALID ) || ( ddh->next_slot != BLK_SHORT_INVALID )) {
		dblock_t *dblk = (dblock_t *)disk2addr( fs->base, (block2offset( ddh->next_dentry )));
		ddentry_t *disk_dentry = (ddentry_t *)disk2addr( dblk, dentry2offset( ddh->next_slot ));
    
		if (( disk_dentry->name_size == name_size ) && 
		    ( strncmp( disk_dentry->name, name, disk_dentry->name_size ) == 0 )) {
			return disk_dentry;
		}

		ddh = &disk_dentry->next;
	}

	return (ddentry_t *)NULL;  
}


/**********************************************************************

    Function    : diskFindFile
    Description : Retrieve the on-disk file from the on-disk dentry
    Inputs      : disk_dentry - on-disk dentry
    Outputs     : on-disk file control block or NULL if error

***********************************************************************/

fcb_t *diskFindFile( ddentry_t *disk_dentry ) 
{
	if ( disk_dentry->block != BLK_INVALID ) {
		dblock_t *blk =  (dblock_t *)disk2addr( fs->base, (block2offset( disk_dentry->block )));
		return (fcb_t *)disk2addr( blk, sizeof(dblock_t) );
	}

	errorMessage("diskFindFile: no such file");
	return (fcb_t *)NULL;  
}


/**********************************************************************

    Function    : diskCreateDentry
    Description : Create disk entry for the dentry on directory
    Inputs      : base - ptr to base of file system on disk
                  dir - in-memory directory
                  dentry - in-memory dentry
    Outputs     : none

***********************************************************************/

void diskCreateDentry( unsigned int base, dir_t *dir, dentry_t *dentry ) 
{
	ddir_t *diskdir = dir->diskdir;
	ddentry_t *disk_dentry;
	dblock_t *dblk, *nextblk;
	ddh_t *ddh;
	int empty = 0;
	int key;

	// create buffer cache for blocks retrieved from disk - not mmapped

	/* find location for new on-disk dentry */
	dblk = (dblock_t *)disk2addr( base, (block2offset( diskdir->freeblk )));
	disk_dentry = (ddentry_t *)disk2addr( dblk, dentry2offset( diskdir->free ));

	/* associate dentry with ddentry */
	dentry->diskdentry = disk_dentry;  
  
	/* update disk dentry with dentry's data */
	memcpy( disk_dentry->name, dentry->name, dentry->name_size );  // check bounds in dentry
	disk_dentry->name[dentry->name_size] = 0;   // null terminate
	disk_dentry->name_size = dentry->name_size;
 	disk_dentry->block = BLK_INVALID;

	/* push disk dentry into on-disk hashtable */
	key = fsMakeKey( disk_dentry->name, diskdir->buckets, disk_dentry->name_size );
	ddh = diskDirBucket( diskdir, key );
	/* at diskdir's hashtable bucket "key", make this disk_dentry the next head
	   and link to the previous head */
	disk_dentry->next.next_dentry = ddh->next_dentry;   
	disk_dentry->next.next_slot = ddh->next_slot;       
	ddh->next_dentry = diskdir->freeblk;
	ddh->next_slot = diskdir->free;

	/* set this disk_dentry as no longer free in the block */
	clearbit( dblk->st.dentry_map, diskdir->free, DENTRY_MAX );   

	/* update free reference for dir */
	/* first the block, if all dentry space has been consumed */
	if ( dblk->st.dentry_map == 0 ) { /* no more space for dentries here */
		/* need another directory block for disk dentries */
		/* try "next" block first until no more */
		unsigned int next_index = dblk->next;
		while ( next_index != BLK_INVALID ) {
			nextblk = (dblock_t *)disk2addr( base, block2offset( next_index ));
			if ( nextblk->st.dentry_map != 0 ) {
				diskdir->freeblk = next_index;
				dblk = nextblk;
				goto done;
			}
			next_index = nextblk->next;
		}
		
		/* get next file system free block for next dentry block */
		diskdir->freeblk = dfs->firstfree;
      
		/* update file system's free blocks */
		nextblk = (dblock_t *)disk2addr( base, block2offset( dfs->firstfree ));
		dfs->firstfree = nextblk->next;
		nextblk->free = DENTRY_BLOCK;   /* this is now a dentry block */
		nextblk->st.dentry_map = DENTRY_MAP;
		nextblk->next = BLK_INVALID;
		dblk = nextblk;
	}

done:
	/* now update the free entry slot in the block */
	/* find the empty dentry slot */
	empty = findbit( dblk->st.dentry_map, DENTRY_MAX );
	diskdir->free = empty;

	if (empty == BLK_INVALID ) {
		errorMessage("diskCreateDentry: bad bitmap");
		return;
	}      
}


/**********************************************************************

    Function    : diskCreateFile
    Description : Create file block for the new file
    Inputs      : base - ptr to base of file system on disk
                  dentry - in-memory dentry
                  file - in-memory file
    Outputs     : 0 on success, <0 on error 

***********************************************************************/

int diskCreateFile( unsigned int base, dentry_t *dentry, file_t *file )
{
	dblock_t *fblk;
	fcb_t *fcb;
	ddentry_t *disk_dentry;
	int i;
	unsigned int block;

	allocDblock( &block, FILE_BLOCK );

	if ( block == BLK_INVALID ) {
		return -1;
	}  
  
	/* find a file block in file system */
	fblk = (dblock_t *)disk2addr( base, (block2offset( block )));
	fcb = (fcb_t *)disk2addr( fblk, sizeof( dblock_t ));   /* file is offset from block info */

	/* associate file with the on-disk file */
	file->diskfile = fcb;

	// P3 - metadata 
	/* set file data into file block */
	fcb->flags = file->flags;
	/* XXX initialize attributes */
	fcb->attr_block = BLK_INVALID;    /* no block yet */

	/* initial on-disk block information for file */  
	for ( i = 0; i < FILE_BLOCKS; i++ ) {
		fcb->blocks[i] = BLK_INVALID;   /* initialize to empty */
	}

	/* get on-disk dentry */
	disk_dentry = dentry->diskdentry;

	/* set file block in on-disk dentry */
	disk_dentry->block = block;

	return 0;
}


/**********************************************************************

    Function    : diskWrite
    Description : Write the buffer to the disk
    Inputs      : disk_offset - pointer to place where offset is stored on disk
                  block - index to block to be written
                  buf - data to be written
                  bytes - the number of bytes to write
                  offset - offset from start of file
                  sofar - bytes written so far
    Outputs     : number of bytes written or -1 on error 

***********************************************************************/

unsigned int diskWrite( unsigned int *disk_offset, unsigned int block, 
			char *buf, unsigned int bytes, 
			unsigned int offset, unsigned int sofar )
{
	dblock_t *dblk;
	char *start, *end, *data;
	int block_bytes;
	unsigned int blk_offset = offset % FS_BLKDATA;
	/* compute the block addresses and range */
	dblk = (dblock_t *)disk2addr( fs->base, (block2offset( block )));
	data = (char *)disk2addr( dblk, sizeof(dblock_t) );
	start = (char *)disk2addr( data, blk_offset );
	end = (char *)disk2addr( fs->base, (block2offset( (block+1) )));
	block_bytes = min(( end - start ), ( bytes - sofar ));

	/* do the write */
	memcpy( start, buf, block_bytes );
//	printf("Disk write At %p buffer %s Bytes %d \n", start, buf, block_bytes);
	/* compute new offset, and update in fcb if end is extended */
	offset += block_bytes;

	if ( offset > *disk_offset ) {
		*disk_offset = offset;
	}

	return block_bytes;  
}


/**********************************************************************

    Function    : diskRead
    Description : read the buffer from the disk
    Inputs      : block - index to file block to read
                  buf - buffer for data
                  bytes - the number of bytes to read
                  offset - offset from start of file
                  sofar - bytes read so far 
    Outputs     : number of bytes read or -1 on error 

***********************************************************************/

unsigned int diskRead( unsigned int block, char *buf, unsigned int bytes, 
		       unsigned int offset, unsigned int sofar )
{
	dblock_t *dblk;
	char *start, *end, *data;
	int block_bytes;
	unsigned int blk_offset = offset % FS_BLKDATA;

	/* compute the block addresses and range */
	dblk = (dblock_t *)disk2addr( fs->base, (block2offset( block )));
	data = (char *)disk2addr( dblk, sizeof(dblock_t) );
	start = (char *)disk2addr( data, blk_offset );
	end = (char *)disk2addr( fs->base, (block2offset( (block+1) )));
	block_bytes = min(( end - start ), ( bytes - sofar ));

	/* do the read */
	memcpy( buf, start, block_bytes );

	return block_bytes;  
}


/**********************************************************************

    Function    : diskGetBlock
    Description : Get the block corresponding to this file location
    Inputs      : file - in-memory file pointer
                  index - block index in file
    Outputs     : block index or BLK_INVALID

***********************************************************************/

unsigned int diskGetBlock( file_t *file, unsigned int index )
{
	fcb_t *fcb = file->diskfile;
	unsigned int dblk_index;

	if ( fcb == NULL ) {
		errorMessage("diskGetBlock: No file control block for file");
		return BLK_INVALID;
	}

	/* if the index is already in the file control block, then return that */
	dblk_index = fcb->blocks[index]; 
 
	if ( dblk_index != BLK_INVALID ) {
		return dblk_index;
	}

	allocDblock( &dblk_index, FILE_DATA );

	if ( dblk_index == BLK_INVALID ) {
		return BLK_INVALID;
	}

	// P3: Meta-Data 
	/* update the fcb with the new block */
	fcb->blocks[index] = dblk_index;

	return dblk_index;
}


/**********************************************************************

    Function    : allocDblock
    Description : Get a free data block
    Inputs      : index - index for the block found or BLK_INVALID
                  blk_type - the type of use for the block
    Outputs     : 0 on success, <0 on error                  

***********************************************************************/

int allocDblock( unsigned int *index, unsigned int blk_type ) 
{
	dblock_t *dblk;

	/* if there is no free block, just return */
	if ( dfs->firstfree == BLK_INVALID ) {
		*index = BLK_INVALID;
		return BLK_INVALID;
	}

	/* get from file system's free list */
	*index = dfs->firstfree;

	/* update the filesystem's next free block */
	dblk = (dblock_t *)disk2addr( fs->base, (block2offset( *index )));

	/* mark block as a file block */
	dblk->free = blk_type;

	/* update next freeblock in file system */
	// P3 - metadata below
	dfs->firstfree = dblk->next;

	return 0;
}
    


/* TASK **: Add the disk level implementation of set attributed and get attribute */

/**********************************************************************

    Function    : diskGetAttrBlock
    Description : Get the block for the file's attributes
    Inputs      : file - in-memory file pointer
                  flag - create block (BLOCK_CREATE) or not (BLOCK_PRESENT)
    Outputs     : block index or BLK_INVALID

***********************************************************************/

unsigned int diskGetAttrBlock( file_t *file, unsigned int flags )
{

	fcb_t* fcb = file->diskfile;
	unsigned int ablk_index;

	if (fcb == NULL) {
		errorMessage("diskGetBlock: No file control block for file");
		return BLK_INVALID;
	}

	/* if the index is already in the file control block, then return that */
	ablk_index = fcb->attr_block;

	if (ablk_index != BLK_INVALID) {
		return ablk_index;
	}

	if (flags == BLOCK_CREATE) allocDblock(&ablk_index, ATTR_BLOCK);

	if (ablk_index == BLK_INVALID) {
		return BLK_INVALID;
	}

	// P3: Meta-Data 
	/* update the fcb with the new block */
	fcb->attr_block = ablk_index;

	return ablk_index;
}


/* Project 4: on-disk versions of the xattr functions */

/**********************************************************************

    Function    : diskSetAttr
    Description : Set the attribute for the file control block associated with file
    Inputs      : attr_block - index to attr_block
                  name - name of attribute
                  value - value for attribute
                  name_size - length of name string in bytes
                  value_size - length of value string in bytes
    Outputs     : 0 on success, <0 on error                  

***********************************************************************/

int diskSetAttr(unsigned int attr_block, char* name, char* value,
	unsigned int name_size, unsigned int value_size )
{
	/* IMPLEMENT THIS */
	unsigned int block, index;
	char* buf;
	xcb_t* xcb;
	dxattr_t* xattrs;
	dblock_t* dblk;
	unsigned int total = 0, cursize = 0, disk_offset=0;
	int i = 0, xattrs_offset = 0;

	buf = calloc(1, FS_BLOCKSIZE);
	dblk = (dblock_t*)buf;
	xcb = (xcb_t*)&dblk->data;   /* convert from blank chars to a structure containing xcb and a bunch of dxattrs - union */

	xcb->no_xattrs = 0;
	xcb->size = 0;

	xattrs = (dxattr_t*)(((char*)xcb->xattrs) + xattrs_offset);

	diskRead(attr_block, buf, FS_BLOCKSIZE, 0, 0);
	if (xcb->no_xattrs == 0) {
		for (i = 0; i < XATTR_BLOCKS; i++) {
			xcb->value_blocks[i] = BLK_INVALID;
		}
	}
	for (i = 0; i < xcb->no_xattrs; i++) {
		if (strncmp(xattrs->name, name, name_size) == 0) {
			break;
		}
		xattrs_offset += sizeof(dxattr_t) + xattrs->name_size;
		xattrs = (dxattr_t*)(((char*)xcb->xattrs) + xattrs_offset);
	}

	if (i>= xcb->no_xattrs) {
		//Save attribute name to attribute block on disk
		xcb->no_xattrs++;
		memcpy(xattrs->name, name, name_size);
		xattrs->name_size = name_size;
	}
	cursize = xcb->size;
	xattrs->value_size = value_size;
	xattrs->value_offset = cursize;
	xcb->size += value_size;
//	printf("attribute location %p name %s size %d value size %d value offset %d\n", xattrs, xattrs->name, xattrs->name_size,xattrs->value_size,xattrs->value_offset);


	index = cursize / (FS_BLOCKSIZE - sizeof(dblock_t)); //value blocks - last value block to be used
	while (total < value_size) {   /* more to write */
		unsigned int block_bytes;
		block = xcb->value_blocks[index];
		/* if block has not been brought into memory, copy it */
		if (block == BLK_INVALID) {
			allocDblock(&block, ATTR_BLOCK);
			xcb->value_blocks[index] = block;
			if (block == BLK_INVALID) {
				errorMessage("fileWrite: Could not get block from the disk for attribute");
				return -1;
			}
			if (index >= FILE_BLOCKS) {
				errorMessage("fileWrite: Max size of file reached");
				return total;
			}
		}
		/* write to this block */
		disk_offset = 0;
		block_bytes = diskWrite(&disk_offset, block, value, value_size, cursize + total, total);
//		printf("write to %d offset %d bytes %d value %s \n", block,cursize+total, block_bytes, value);
		/* update the total written and the file offset as well */
		total += block_bytes;
		value += block_bytes;
		index++;
	}
	//write attribute block at end after updating new attribute/existing attribute value_offset and blocks
	disk_offset = 0;
	diskWrite(&disk_offset, attr_block, buf, FS_BLOCKSIZE, 0, 0);
	/* update the file's size (if necessary) */
	free(buf);
	return 0;
	//return disk_offset;
}


/**********************************************************************

    Function    : diskGetAttr
    Description : Get the value for the attribute for name of the file 
                  control block associated with file
    Inputs      : attr_block - index of attribute block
                  name - name of attribute
                  value - value buffer for attribute
                  name_size - length of name string in bytes
                  size - max amount that can be read
                  existsp - flag to check for existence of attr of name only
    Outputs     : number of bytes read on success, <0 on error                  

***********************************************************************/

int diskGetAttr( unsigned int attr_block, char *name, char *value, 
		 unsigned int name_size, unsigned int size, unsigned int existsp)
{
	char* buf;
	xcb_t* xcb;
	dxattr_t* xattrs;
	dblock_t* dblk;
	int i = 0, xattrs_offset = 0, attribute_value_size = 0,total=0;
	unsigned int block, index;
	unsigned int attribute_value_offset = 0;

	/* IMPLEMENT THIS */
	/* if block has not been brought into memory, copy it */
	if (attr_block == BLK_INVALID) {
		errorMessage("No attribute exists with file");
		return 0;
	}
	buf = calloc(1, FS_BLOCKSIZE);
	dblk = (dblock_t*)buf;
	xcb = (xcb_t*)&dblk->data;   /* convert from blank chars to a structure containing xcb and a bunch of dxattrs - union */
	xattrs = (dxattr_t*)(((char*)xcb->xattrs) + xattrs_offset);
	diskRead(attr_block, buf, FS_BLOCKSIZE, 0, 0);
	for (i = 0; i < xcb->no_xattrs; i++) {
		if (strncmp(xattrs->name, name, name_size) == 0) {
			attribute_value_offset = xattrs->value_offset;
			attribute_value_size = xattrs->value_size;
			break;
		}
		xattrs_offset += sizeof(dxattr_t) + xattrs->name_size;
		xattrs = (dxattr_t*)(((char*)xcb->xattrs) + xattrs_offset);
	}
	if (i >= xcb->no_xattrs) {
		return 0;
	}
//	printf("attribute location %p name %s size %d value size %d value offset %d\n", xattrs, xattrs->name, xattrs->name_size, xattrs->value_size, xattrs->value_offset);
	if (existsp) return 1;
	block = BLK_INVALID;
	/* read limit is either size of buffer or distance to end of file */
	size = min(size, attribute_value_size);
	/* read the file from the offset */
	while (total < size) {   /* more to read */
		int block_bytes;
		/* if block has not been brought into memory, copy it */
		index = attribute_value_offset / (FS_BLOCKSIZE - sizeof(dblock_t));
		block = xcb->value_blocks[index];
		if (block == BLK_INVALID) {
			//				printf("Unexpected error No block exists for value. %d block %d value offset %d\n",i,index,attribute_value_offset);
			return 0;
		}


		/* read this block */
		block_bytes = diskRead(block, value, size, attribute_value_offset, total);
		//		printf("read from %d offset %d bytes %d value %s \n", block, attribute_value_offset,block_bytes, value);
		/* update the total written and the file offset as well */
		total += block_bytes;
		attribute_value_offset += block_bytes;
		value += block_bytes;
		i++;
	}
	return total;
}
