/**********************************************************************

   File          : cmpsc473-filesys.c

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

/* macros */

/* program variables */

/* Functions */

/**********************************************************************

    Function    : fsReadDir
    Description : Return directory entry for name
    Inputs      : name - name of the file 
                  name_size - length of file name
    Outputs     : new in-memory directory or NULL

***********************************************************************/

dir_t *fsReadDir( char *name, unsigned int name_size )
{
	dir_t *dir;
	ddir_t *diskdir;

	/* there is only one directory in this file system */
	if ( fs->dir != NULL ) {
		return fs->dir;
	}

	/* otherwise retrieve the directory from the disk */
	diskdir = diskReadDir( name, name_size );

	/* build an in-memory directory for the returned dir */
	dir = (dir_t *)malloc(sizeof(dir_t));

	if ( dir == NULL ) {
		errorMessage("fsReadDir: could not alloc dir");
		return (dir_t *)NULL;
	}

	/* store diskdir with dir and create in-memory dir */
	dir->diskdir = diskdir;
	dir->bucket_size = diskdir->buckets;
	dir->buckets = (dentry_t **)malloc(diskdir->buckets * sizeof(dentry_t *));  // ***

	/* assign the root directory to the in-memory file system */
	fs->dir = dir;

	return( dir );
}


/**********************************************************************

    Function    : fsFindDentry
    Description : Retrieve the dentry for this file name
    Inputs      : dir - directory in which to look for name 
                  name - name of the file 
                  name_size - length of file name
    Outputs     : new dentry or NULL if error

***********************************************************************/

dentry_t *fsFindDentry( dir_t *dir, char *name, unsigned int name_size )
{
	int key = fsMakeKey( name, dir->bucket_size, name_size );
	dentry_t *dentry = inList( dir->buckets[key], name, name_size );

	/* if not cached already, have to get it from the disk */
	if ( dentry == NULL ) {
		ddentry_t *disk_dentry = diskFindDentry( dir->diskdir, name, name_size );

		if ( disk_dentry == NULL ) {
			return (dentry_t *)NULL;
		}

		/* build a blank in-memory directory entry for the new file */
		dentry = fsDentryInitialize( name, disk_dentry, name_size );    
	}

	return dentry;  
}


/**********************************************************************

    Function    : fsFindFile
    Description : Retrieve the in-memory file for this file name
    Inputs      : dentry - dentry in which to look for name 
                  name - name of the file 
                  flags - flags requested for this file
                  name_size - length of file name
    Outputs     : new file or NULL if error

***********************************************************************/

file_t *fsFindFile( dentry_t *dentry, char *name, unsigned int flags, unsigned int name_size )
{
	file_t *file = dentry->file;

	/* if not cached already, have to get it from the disk */
	if ( file == NULL ) {
		ddentry_t *disk_dentry = dentry->diskdentry;
		fcb_t *fcb;  /* on disk file */

		if ( disk_dentry == NULL ) {
			errorMessage("fsFindFile: no on-disk entry must create one first");
			return (file_t *)NULL;
		}

		fcb = diskFindFile( disk_dentry );

		if ( fcb == NULL ) {
			errorMessage("fsFindFile: no on-disk file --  must create one first");
			return (file_t *)NULL;
		}

		/* check that flags requests are included in file */
		if (( flags | fcb->flags ) != fcb->flags ) {
			errorMessage("fsFindFile: flags are not acceptable");
			return (file_t *)NULL;
		}      

		/* build a blank in-memory directory entry for the new file */
		file = fsFileInitialize( dentry, name, flags, name_size, fcb );    
	}

	return file;  
}


/**********************************************************************

    Function    : fsCacheFindFile
    Description : Find if file is in the system-wide file table
    Inputs      : filetable - system-wide file table
                  name - file name 
                  flags - file flags 
                  name_size - length of file name
    Outputs     : file or NULL

***********************************************************************/

file_t *fsCacheFindFile( file_t **filetable, char *name, unsigned int flags, unsigned int name_size )
{
	int i;

	for ( i = 0; i < FS_FILETABLE_SIZE; i++ ) {
		file_t *file = filetable[i];
		if (( file != NULL ) && 
		    ( file->name_size == name_size ) &&
		    (( strncmp( file->name, name, file->name_size )) == 0 ) && 
		    (( flags | file->flags) == file->flags ))  {
			return file;
		}
	}
  
	return (file_t *)NULL;
}

/**********************************************************************

    Function    : fsDentryInitialize
    Description : Create a memory dentry for this file
    Inputs      : name - file name 
                  disk_dentry - on-disk dentry object (optional)
                  name_size - length of file name
    Outputs     : new directory entry or NULL

***********************************************************************/

dentry_t *fsDentryInitialize( char *name, ddentry_t *disk_dentry, unsigned int name_size )
{
	dentry_t *dentry = (dentry_t *)malloc(sizeof(dentry_t)+name_size+1);

	if ( dentry == NULL ) {
		errorMessage("fsDentryInitialize: could not alloc dentry");
		return NULL;
	}

	dentry->file = (file_t *) NULL;
	memcpy( dentry->name, name, name_size );
	dentry->name[name_size] = 0;   // null terminate
	dentry->name_size = name_size;
	dentry->diskdentry = disk_dentry;
	dentry->next = (dentry_t *) NULL;

	return dentry;
}


/**********************************************************************

    Function    : fsFileInitialize
    Description : Create a memory file for the specified file
    Inputs      : dir - directory object
                  name - name of the file 
                  flags - flags for file access
                  name_size - length of file name
                  fcb - file control block (on-disk) reference for file (optional)
    Outputs     : new file or NULL

***********************************************************************/

file_t *fsFileInitialize( dentry_t *dentry, char *name, unsigned int flags, unsigned int name_size, fcb_t *fcb )
{
	file_t *file = (file_t *)malloc(sizeof(file_t)+name_size+1);

	if ( file == NULL ) {
		errorMessage("fsFileInitialize: could not alloc file");
		return NULL;
	}

	/* construct file object */
	file->flags = flags;
	file->size = ( fcb ? fcb->size : 0 );  /* fcb is only null at create time */
	file->diskfile = fcb;
	file->ct = 1;
	file->attr_block = BLK_INVALID;
	memset( file->blocks, BLK_INVALID, FILE_BLOCKS * sizeof(unsigned int) );
	memcpy( file->name, name, name_size );
	file->name[name_size] = 0; // null terminate
	file->name_size = name_size;

	/* associate dentry with file */
	dentry->file = file;

	return file;
}


/**********************************************************************

    Function    : fsAddDentry
    Description : Add the dentry to its directory
    Inputs      : dir - directory object
                  dentry - dentry object
    Outputs     : 0 if success, -1 if error 

***********************************************************************/

int fsAddDentry( dir_t *dir, dentry_t *dentry )
{
	int key;

	key = fsMakeKey( dentry->name, dir->bucket_size, dentry->name_size );

	if ( key >= dir->bucket_size ) {
		errorMessage("fsAddDentry: problem with the bucket count");
		return -1;
	}

	/* add to in-memory directory representation -- not same as disk list */
	addToList( &dir->buckets[key], dentry );

	return 0;
}


/**********************************************************************

    Function    : fsAddFile
    Description : Add the file to the system-wide open file cache
    Inputs      : filetable - system-wide file table 
                  file - file to be added
    Outputs     : an index, or -1 on error 

***********************************************************************/

int fsAddFile( file_t **filetable, file_t *file) 
{
	int i;

	for ( i = 0; i < FS_FILETABLE_SIZE; i++ ) {
		if ( filetable[i] == NULL ) {
			filetable[i] = file;
			return i;
		}
	}
  
	errorMessage("fsAddFile: system-wide file table is full");
	return -1;
}


/**********************************************************************

    Function    : fsAddProcFile
    Description : Add the file to the per-process open file cache
    Inputs      : proc - process
                  file - file to be added
    Outputs     : a file descriptor, or -1 on error 

***********************************************************************/

int fsAddProcFile( proc_t *proc, file_t *file) 
{
	int i;
	fstat_t *fstat;

	/* make the fstat structure for the per-process table */
	fstat = (fstat_t *)malloc(sizeof(fstat));
	fstat->file = file;
	fstat->offset = 0;

	/* add to the per-process file table */
	for ( i = 0; i < PROC_FILETABLE_SIZE; i++ ) {
		if ( proc->fstat_table[i] == NULL ) {
			proc->fstat_table[i] = fstat;
			/* return the index (file descriptor) */
			return i;
		}
	}

	errorMessage("fsAddProcFile: per-process file table is full");
	return -1;
}


/**********************************************************************

    Function    : fileCreate
    Description : Create directory entry and file object
    Inputs      : name - name of the file 
                  flags - creation options
    Outputs     : new file descriptor or -1 if error

***********************************************************************/

int fileCreate( char *name, unsigned int flags )
{
	dentry_t *dentry;
	int index, fd;
	dir_t *dir;
	file_t *file;
	int rtn;
	unsigned int name_size = strlen( name ); // p4.c null terminates

	if ( name_size >= MAX_NAME_SIZE ) {
		return -1;
	}
	     
        /* =======  verify file does not already exist -- first in the filetable */
	file = fsCacheFindFile( fs->filetable, name, flags, name_size );

	if ( file ) {
		errorMessage("fileCreat: file already exists in file table");
		return -1;
	}

	/* Then on disk (with a corresponding dentry */
	/* get the in-memory representation of our directory (perhaps on disk) */
	dir = fsReadDir( name, name_size );

	/* retrieve in-memory dentry for this name */
	dentry = fsFindDentry( dir, name, name_size );
  
	if ( dentry ) {
		errorMessage("fileCreat: file already exists on disk");
		return -1;
	}

	/* =======  now build the file */
	/* build a blank in-memory directory entry for the new file */
	dentry = fsDentryInitialize( name, (ddentry_t *)NULL, name_size );

	/* add in-memory dentry to in-memory directory */
	fsAddDentry( dir, dentry );

	/* create file in memory */
	file = fsFileInitialize( dentry, name, flags, name_size,  (fcb_t *)NULL );

	/* add dentry to disk */
	diskCreateDentry( (unsigned int)fs->base, dir, dentry );

	/* add file to disk */
	rtn = diskCreateFile( (unsigned int)fs->base, dentry, file );
	if ( rtn ) return rtn;

	/* add file in system-wide file table */
	index = fsAddFile( fs->filetable, file );
	if ( index < 0 ) {
		return index;
	}

	/* add file to per-process file table */
	fd = fsAddProcFile( fs->proc, file );

	return fd;
}


/**********************************************************************

    Function    : fileOpen
    Description : Open directory entry of specified name
    Inputs      : name - name of the file 
                  flags - creation options
    Outputs     : new file descriptor or -1 if error

***********************************************************************/

int fileOpen( char *name, unsigned int flags )
{
	dentry_t *dentry;
	int index, fd;
	dir_t *dir;
	file_t *file;
	unsigned int name_size = strlen( name ); // p4.c null terminates

	if ( name_size >= MAX_NAME_SIZE ) {
		return -1;
	}

	/* search for file in the system-wide file table */
	file = fsCacheFindFile( fs->filetable, name, flags, name_size );
//	printf("Open file %p %s \n", file, name);
	/* if null, get from disk */
	if ( file == NULL ) {

		/* get the in-memory representation of our directory (perhaps on disk) */
		dir = fsReadDir( name, name_size );

		/* retrieve in-memory dentry for this name */
		dentry = fsFindDentry( dir, name, name_size );

		if ( dentry == NULL ) {
			errorMessage("fileOpen: No such file on disk");
			return -1;
		}

		/* add in-memory dentry to in-memory directory */
		// fsAddDentry( dir, dentry );

		/* retrieve in-memory file for this name */
		file = fsFindFile( dentry, name, flags, name_size );

		if ( file == NULL ) {
			return -1;
		}

		/* add file in system-wide file table */
		index = fsAddFile( fs->filetable, file );
//		printf("Open file %d %s \n", index, name);

		if ( index < 0 ) {
			return index;
		}
	}

	/* add file to per-process file table */
	fd = fsAddProcFile( fs->proc, file );
//	printf("Open file %d %s \n", fd, name);

	return fd;
}


/**********************************************************************

    Function    : listDirectory
    Description : print the files in the root directory currently
    Inputs      : none
    Outputs     : number of bytes read

***********************************************************************/

void listDirectory( void )
{
	dir_t *dir;
	ddir_t *diskdir;
	ddh_t *ddh;
	int i;

	/* get the in-memory representation of our directory (perhaps on disk) */
	dir = fsReadDir( "/", 1 );

	/* get on-disk directory */
	diskdir = dir->diskdir;

	/* list the names of all the files reachable from this directory */
	/* more appropriate file: disk.c */
	ddh = (ddh_t *)&diskdir->data[0];
	for ( i = 0; i < diskdir->buckets; i++ ) {
		ddh_t *thisddh = (ddh+i);   // ****
		while ( thisddh->next_dentry != BLK_SHORT_INVALID ) {
			dblock_t *dblk = (dblock_t *)disk2addr( fs->base, (block2offset( thisddh->next_dentry )));
			ddentry_t *disk_dentry = (ddentry_t *)disk2addr( dblk, dentry2offset( thisddh->next_slot ));

			printf("File[%d]: %s\n", i, disk_dentry->name );

			thisddh = &disk_dentry->next;
		}
	}
}


/**********************************************************************

    Function    : fileClose
    Description : close the file associated with the file descriptor
    Inputs      : fd - file descriptor
    Outputs     : none

***********************************************************************/

void fileClose( unsigned int fd )
{
	fstat_t *fstat;
	file_t *file;
	int i;

	/* get the file in per-process file structure */
	fstat = fs->proc->fstat_table[fd];

	if ( fstat == NULL ) {
		errorMessage("fileClose: No file corresponds to fd");
		return;
	}

	file = fstat->file;
  
	/* reduce reference count */
	file->ct--;
  
	/* if ref count is 0, then remove file from system-wide table */
	if ( file->ct == 0 ) {
		/* note: could save the index in the filetable */
		for ( i = 0; i < FS_FILETABLE_SIZE; i++ ) {
			if ( file == fs->filetable[i] ) {
				/* remove entry from the filetable */
				fs->filetable[i] = (file_t *)NULL;
				free( file );
				break;
			}
		}
	}
}


/**********************************************************************

    Function    : fileRead
    Description : Read specified number of bytes from the current file index
    Inputs      : fd - file descriptor
                  buf - buffer for placing data
                  bytes - number of bytes to read
    Outputs     : number of bytes read

***********************************************************************/

int fileRead( unsigned int fd, char *buf, unsigned int bytes )
{
	fstat_t *fstat = fs->proc->fstat_table[fd];
	file_t *file;
	int total = 0;

	if ( fstat == NULL ) {
		errorMessage("fileRead: No file corresponds to fd");
		return -1;
	}

	file = fstat->file;

	if ( file == NULL ) {
		errorMessage("fileRead: No file corresponds to fstat");
		return -1;
	}

	/* read limit is either size of buffer or distance to end of file */
	bytes = min( bytes, ( file->size - fstat->offset ));

	/* read the file from the offset */
	while ( total < bytes ) {   /* more to write */
		int index = fstat->offset / ( FS_BLOCKSIZE - sizeof(dblock_t) );
		unsigned int block = file->blocks[index];
		int block_bytes;

		/* if block has not been brought into memory, copy it */
		if ( block == BLK_INVALID ) {
			block = diskGetBlock( file, index );
			file->blocks[index] = block;
      
			if ( block == BLK_INVALID ) {
				errorMessage("fileRead: Could get block from the disk");
				return -1;
			}
		}

		if ( index >= FILE_BLOCKS ) {
			errorMessage("fileRead: Max size of file reached");
			return total;
		}

		/* read this block */
		block_bytes = diskRead( block, buf, bytes, 
					fstat->offset, total );

		/* update the total written and the file offset as well */
		total += block_bytes; 
		fstat->offset += block_bytes;
		buf += block_bytes;
	}
  
	return total;
}


/**********************************************************************

    Function    : fileWrite
    Description : Write specified number of bytes starting at the current file index
    Inputs      : fd - file descriptor
                  buf - buffer to write
                  bytes - number of bytes to write
    Outputs     : number of bytes written

***********************************************************************/

int fileWrite( unsigned int fd, char *buf, unsigned int bytes )
{
	fstat_t *fstat = fs->proc->fstat_table[fd];
	file_t *file;
	unsigned int total = 0;

	if ( fstat == NULL ) {
		errorMessage("fileWrite: No file corresponds to fd");
		return -1;
	}

	/* get file structure */
	file = fstat->file;

	if ( file == NULL ) {
		errorMessage("fileWrite: No file corresponds to fstat");
		return -1;
	}

	/* write to the file */
	while ( total < bytes ) {   /* more to write */
		int index = fstat->offset / ( FS_BLOCKSIZE - sizeof(dblock_t) );
		unsigned int block = file->blocks[index];
		unsigned int block_bytes;

		/* if block has not been brought into memory, copy it */
		if ( block == BLK_INVALID ) {
			block = diskGetBlock( file, index );
			file->blocks[index] = block;
      
			if ( block == BLK_INVALID ) {
				errorMessage("fileWrite: Could get block from the disk");
				return -1;
			}
		}

		if ( index >= FILE_BLOCKS ) {
			errorMessage("fileWrite: Max size of file reached");
			return total;
		}

		/* write to this block */
		block_bytes = diskWrite( &(file->diskfile->size), block, buf, bytes, 
					 fstat->offset, total );

		/* update the total written and the file offset as well */
		total += block_bytes; 
		fstat->offset += block_bytes;
		buf += block_bytes;
	}

	/* update the file's size (if necessary) */
	if ( fstat->offset > file->size ) {
		file->size = fstat->offset;
	}

	return total;
}


/**********************************************************************

    Function    : fileSeek
    Description : Adjust offset in per-process file entry
    Inputs      : fd - file descriptor
                  index - new offset 
    Outputs     : 0 on success, -1 on failure

***********************************************************************/

int fileSeek( unsigned int fd, unsigned int index )
{
	fstat_t *fstat = fs->proc->fstat_table[fd];
	file_t *file;

	if ( fstat == NULL ) {
		errorMessage("fileSeek: No file corresponds to fd");
		return -1;
	}

	file = fstat->file;

	if ( file == NULL ) {
		errorMessage("fileSeek: No file corresponds to fstat");
		return -1;
	}

	if ( index <= file->size ) {
		fstat->offset = index;
	}

	return 0;
}


/**********************************************************************

    Function    : fileSetAttr
    Description : Write file extended attribute
    Inputs      : fd - file descriptor
                  name - name of attribute
                  value - value for attribute
                  name_size - length of the name string in bytes
                  value_size - length of the value string in bytes
                  flags - set attribute flags (XATTR_CREATE and XATTR_REPLACE)
    Outputs     : 0 on success, <0 on error

***********************************************************************/

int fileSetAttr( unsigned int fd, char *name, char *value, unsigned int name_size, 
		 unsigned int value_size, unsigned int flags )
{
        fstat_t* fstat = fs->proc->fstat_table[fd];
        file_t* file;
        unsigned int block;
        if (fstat == NULL) {
                errorMessage("fileWrite: No file corresponds to fd");
                return -1;
        }

        /* get file structure */
        file = fstat->file;

        if (file == NULL) {
                errorMessage("fileWrite: No file corresponds to fstat");
                return -1;
        }
        /* write attribute to the file */
        /* IMPLEMENT THIS */
		block = file->attr_block;
		if (block == BLK_INVALID) {
			block = diskGetAttrBlock(file, BLOCK_CREATE);
			file->attr_block = block;
		}

        unsigned int existsp = 1;

        if ((flags == XATTR_CREATE) && (diskGetAttr(block, name, value, name_size, value_size, existsp)==1)) {
                /* Error case: print on failed XATTR_CREATE */
                errorMessage("fileSetAttr fail: already an entry for name - incompatible with flag XATTR_CREATE");
                return -1;
        }


        if ((flags == XATTR_REPLACE) && (diskGetAttr(block, name, value, name_size, value_size, existsp) == 0)) {
                /* Error case: print on failed XATTR_REPLACE */
                errorMessage("fileSetAttr fail: no existing entry for name - incompatible with flag XATTR_REPLACE");
                return -1;
        }


        if (diskSetAttr(file->attr_block, name, value, name_size, value_size) == -1) {
                errorMessage("diskSetAttr fail ");
                return -1;
        }
        

        return 0;
}

/**********************************************************************

    Function    : fileGetAttr
    Description : Read file extended attribute
    Inputs      : fd - file descriptor
                  name - name of attribute
                  value - value for attribute to retrieve
                  name_size - length of the name in bytes
                  size - of buffer for value
    Outputs     : number of bytes on success, <0 on error

***********************************************************************/

int fileGetAttr( unsigned int fd, char *name, char *value, unsigned int name_size, unsigned int size ) 
{
	/* IMPLEMENT THIS */
        fstat_t* fstat = fs->proc->fstat_table[fd];
        file_t* file;
        unsigned int block;
        unsigned int attribute_value_size = 0;

        if (fstat == NULL) {
                errorMessage("GetAttr: No file corresponds to fd");
                return -1;
        }

        /* get file structure */
        file = fstat->file;

        if (file == NULL) {
                errorMessage("GetAttr: No file corresponds to fstat");
                return -1;
        }

        block = file->attr_block;
		if (block == BLK_INVALID) {
			block = diskGetAttrBlock(file, BLOCK_PRESENT);
			file->attr_block = block;
		}
		unsigned int existsp = 0;
        attribute_value_size = diskGetAttr(block, name, value, name_size, size, existsp);
		return attribute_value_size;
}

