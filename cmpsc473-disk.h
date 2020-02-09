
/**********************************************************************

   File          : cmpsc473-disk.h

   Description   : This file contains the file system prototypes

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

/* defines */

/* macros */
#define block2offset( block ) ( FS_BLOCKSIZE * block )
#define dentry2offset( dentry_index ) ( sizeof(dblock_t) + ( sizeof(ddentry_t) * dentry_index )) // for ddentry's
#define FS_BLKDATA ( FS_BLOCKSIZE - sizeof(dblock_t) )


/* set the index in bitmap to 0 */
#define clearbit( bitmap, index, max ) ( bitmap &= ~( 1 << ( index % max )))

/* get the first free index in the bitmap */
static inline int findbit( unsigned int bitmap, unsigned int max ) 
{
	int i;

	for ( i = 0; i < max; i++ ) {
		if ( bitmap & ( 1 << i )) {
			return i;
		}
	}
	return BLK_INVALID;
}


static inline ddh_t *diskDirBucket( ddir_t *diskdir, int key ) 
{
	ddh_t *ddh = (ddh_t *)diskdir->data;

	ddh += key;

	return ddh;
}

/* functions */

/**********************************************************************

    Function    : diskDirInitialize
    Description : Initialize the root directory on disk
    Inputs      : directory reference
    Outputs     : 0 if success, -1 on error

***********************************************************************/

extern int diskDirInitialize( ddir_t *ddir );

/**********************************************************************

    Function    : diskReadDir
    Description : Retrieve the on-disk directory -- only one in this case
    Inputs      : name - name of the file 
                  name_size - length of file name
    Outputs     : on-disk directory or -1 if error

***********************************************************************/

extern ddir_t *diskReadDir( char *name, unsigned int name_size );


/**********************************************************************

    Function    : diskCreateDentry
    Description : Create disk entry for the dentry on directory
    Inputs      : base - ptr to base of file system on disk
                  dir - in-memory directory
                  dentry - in-memory dentry
    Outputs     : none

***********************************************************************/

extern void diskCreateDentry( unsigned int base, dir_t *dir, dentry_t *dentry );

/**********************************************************************

    Function    : diskCreateFile
    Description : Create file block for the new file
    Inputs      : base - ptr to base of file system on disk
                  dentry - in-memory dentry
                  file - in-memory file
    Outputs     : 0 on success, <0 on error

***********************************************************************/

extern int diskCreateFile( unsigned int base, dentry_t *dentry, file_t *file );

/**********************************************************************

    Function    : diskFindFile
    Description : Retrieve the on-disk file from the on-disk dentry
    Inputs      : disk_dentry - on-disk dentry
    Outputs     : on-disk file control block or NULL if error

***********************************************************************/

extern fcb_t *diskFindFile( ddentry_t *disk_dentry );

/**********************************************************************

    Function    : diskFindDentry
    Description : Retrieve the on-disk dentry from the disk directory
    Inputs      : diskdir - on-disk directory
                  name - name of the file 
                  name_size - length of file name
    Outputs     : on-disk dentry or NULL if error

***********************************************************************/

extern ddentry_t *diskFindDentry( ddir_t *diskdir, char *name, unsigned int name_size );

/**********************************************************************

    Function    : diskWrite
    Description : Write the buffer to the disk
    Inputs      : disk_offset - reference to location where offset is stored on disk
                  block - index to block to be written
                  buf - data to be written
                  bytes - the number of bytes to write
                  offset - offset from start of file
                  sofar - bytes written so far
    Outputs     : number of bytes written or -1 on error 

***********************************************************************/

extern unsigned int diskWrite( unsigned int *disk_offset, unsigned int block, 
			       char *buf, unsigned int bytes, 
			       unsigned int offset, unsigned int sofar );

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

extern unsigned int diskRead( unsigned int block, char *buf, unsigned int bytes, 
			      unsigned int offset, unsigned int sofar );

/**********************************************************************

    Function    : diskGetBlock
    Description : Get the block corresponding to this file location
    Inputs      : file - in-memory file pointer
                  index - block index in file
    Outputs     : block index or BLK_INVALID

***********************************************************************/

extern unsigned int diskGetBlock( file_t *file, unsigned int index );

/**********************************************************************

    Function    : allocDblock
    Description : Get a free data block
    Inputs      : index - index for the block found or BLK_INVALID
                  blk_type - the type of use for the block
    Outputs     : 0 on success, <0 on error                  

***********************************************************************/

extern int allocDblock( unsigned int *index, unsigned int blk_type );

/**********************************************************************

    Function    : diskGetAttrBlock
    Description : Get the block for the file's attributes
    Inputs      : file - in-memory file pointer
                  flag - create block (BLOCK_CREATE) or not (BLOCK_EXISTS)
    Outputs     : block index or BLK_INVALID

***********************************************************************/

extern unsigned int diskGetAttrBlock( file_t *file, unsigned int flags );

/**********************************************************************

    Function    : diskSetAttr
    Description : Set the attribute for the file control block associated with file
    Inputs      : attr_block - index of attribute block
                  name - name of attribute
                  value - value for attribute
                  name_size - length of name string in bytes
                  value_size - length of value string in bytes
    Outputs     : 0 on success, <0 on error                  

***********************************************************************/

extern int diskSetAttr( unsigned int attr_block, char *name, char *value, 
			unsigned int name_size, unsigned int value_size );

/**********************************************************************

    Function    : diskSetAttrValue
    Description : Set the attribute for the file control block associated with file
    Inputs      : dxattr - on-disk xattr structure (this xattr info -- for corresponding name)
                  xcb - on-disk xattr control block
                  value - value string
                  value_size - length of value string in bytes
    Outputs     : 0 on success, <0 on error                  

***********************************************************************/

extern int diskSetAttrValue( dxattr_t *dxattr, xcb_t *xcb, char *value, 
			     unsigned int value_size );

/**********************************************************************

    Function    : diskGetAttr
    Description : Get the value for the attribute for name of the file 
                  control block associated with file
    Inputs      : index of attribute block
                  name - name of attribute
                  value - value buffer for attribute
                  name_size - length of name string in bytes
                  size - max amount that can be read
                  existsp - flag to check for existence of attr of name only
    Outputs     : 0 on success, <0 on error                  

***********************************************************************/

extern int diskGetAttr( unsigned int attr_block, char *name, char *value, 
			unsigned int name_size, unsigned int size, unsigned int existsp );
