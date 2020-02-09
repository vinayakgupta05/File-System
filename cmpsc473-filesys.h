/**********************************************************************

   File          : cmpsc473-filesys.h

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
#define TRUE 1

/* file system in general */
#define FS_BLOCKS 70
#define FS_METADATA_BLOCKS 1   // fs block
#define FS_DIRINIT_BLOCKS 2    // dir block and first dentry block
#define FS_BLOCKSIZE 512
#define MAX_NAME_SIZE 40
#define BLK_INVALID  0xFFFFFFFF
#define BLK_SHORT_INVALID  0xFFFF

/* disk defines */
#define DENTRY_MAP 0x3FF  /* 10 entries per block */
#define DENTRY_MAX 10     /* 10 entries per block */

/* block free values */
#define FREE_BLOCK 0
#define DENTRY_BLOCK 1
#define FILE_BLOCK 2
#define FILE_DATA 3
#define FS_BLOCK 4
#define DIR_BLOCK 5
#define ATTR_BLOCK 6
#define JOURNAL_BLOCK 7

/* file system defines */
#define FS_FILETABLE_SIZE 60
#define PROC_FILETABLE_SIZE 20
#define FS_BCACHE_BLOCKS 50
#define FILE_BLOCKS 10
#define XATTR_BLOCKS 5

/* extended attribute defines */
#define XATTR_CREATE  1
#define XATTR_REPLACE 2

/* xattr block create defines */
#define BLOCK_CREATE  1
#define BLOCK_PRESENT 2

/* macros */

/* translate between disk location and memory location */
#define disk2addr( mbase, doffset ) ( ((char *)mbase)+doffset )
#define addr2disk( mptr, mbase ) ( ((char *)mptr)-mbase )
#define block2addr( mbase, block_num ) ( ((char *) mbase ) + ( FS_BLOCKSIZE * block_num ))
#define addr2block( mptr, mbase ) ( (((char *) mptr ) - ((char *) mbase)) / FS_BLOCKSIZE )


static inline unsigned int fsMakeKey( char *name, int limit, unsigned int name_size ) 
{
	int len = name_size;
	int i, val = 1;

	for ( i = 0; i < len; i++ ) {
		val = ( (int)name[i] * val ) % limit;
	}
  
	return val;
}

/**********************************************************************

    Structure    : dblock_t 
    Purpose      : corresponds to actual block on the disk -- includes
                   whether the block is free (a next block index if it 
                   is free), a dentry map (for free dentries slots in 
                   a dentry block) and data end (e.g., file data, but 
                   means general block data)

***********************************************************************/

/* data structure for the blocks on disk */
typedef struct dblock {
	unsigned int free;         /* is the block free? also the type of data stored  */
	union status {
		unsigned int dentry_map; /* free dentries bitmap -- out of 10 per block */
		unsigned int data_end;   /* end pointer of file data in block */
	} st;
	unsigned int next;         /* next disk block index -- in free list */
	char data[0];              /* the rest of the block is data -- depends on block size */
} dblock_t;

typedef char block_t;        /* block is a character array of size FS_BLOCKSIZE */


/**********************************************************************

    Structure    : dxattr
    Purpose      : this structure represents an on-disk extended attribute (xattr)
                   on a data block.  An xattr has a name and a value, where
                   names are persistent, but values may be updated (replaced)
                   an arbitrary number of times.  This structure
                   stores the name and a reference to its current value.  
                   It also stores the size of the name and the current value. 
                   Note that names will be stored in one data block, and 
                   values are stored in another in a log-style (i.e., 
                   new values are appended to the end of the log in 
                   the block.

***********************************************************************/

/* on-disk structure for extended attribute */
/* Project 4: xcb data includes name and reference to value in value blocks */

typedef struct dxattr {
	unsigned int name_size;        /* length of name string in bytes */
	unsigned int value_offset;     /* offset of value in value blocks */
	unsigned int value_size;       /* length of value string in bytes */
	char name[0];                  /* reference to the name string */
} dxattr_t;


/**********************************************************************

    Structure    : xcb (xattr control block)
    Purpose      : This structure represents the layout of the 
                   extended attributes for a file on the disk.
                   This consists of one page for name structures
                   (dxattr_t) and up to five pages for values.
                   This structure is placed in the file's attr_block
                   at the beginning, followed by the name structures.

***********************************************************************/

/* on-disk structure for extended attribute layout */
/* Project 4: xcb stores xattr names and associated values in value_blocks */

typedef struct xcb {
	unsigned int value_blocks[XATTR_BLOCKS];  /* blocks for xattr values */
	unsigned int no_xattrs;            /* the number of xattrs in the block */
	unsigned int size;                 /* this is the end of the value list in bytes */
	dxattr_t xattrs[0];                /* then a list of xattr structs (names and value refs) */
} xcb_t;


/**********************************************************************

    Structure    : fcb_t 
    Purpose      : this is the file control block -- persistent storage
                   for file attributes and the file data blocks 
                   (actual number of data blocks max determined by FILE_BLOCKS)

***********************************************************************/

/* on-disk data structure for a file -- file control block */
typedef struct file_control_block {
	unsigned int flags;      /* file flags */
	unsigned int size;       /* size of the file on disk */
	/* Project 4: attr_block for storing first attribute block of file */
	unsigned int attr_block; /* index to first attribute block */
	unsigned int blocks[0];  /* indices to data blocks for rest of file block */
} fcb_t;


/**********************************************************************

    Structure    : file_t 
    Purpose      : this corresponds to an inode -- the in-memory 
                   representation for the file (system-wide).  Includes
                   attributes, including name (names are stored
                   with dentries on the disk -- not fcb), reference count, 
                   reference to the fcb location (in ram-disk), and 
                   a set of file blocks

***********************************************************************/

/* in-memory data structure for a file (inode) */
typedef struct file {
	unsigned int flags;                /* file flags */
	unsigned int size;                 /* file size */
	unsigned int ct;                   /* reference count */
	fcb_t *diskfile;                   /* fcb pointer in ramdisk */
	/* Project 4: attr_block for storing first attribute block of file */
	unsigned int attr_block;           /* index to first attribute block */
	unsigned int blocks[FILE_BLOCKS];  /* direct blocks */
	unsigned int name_size;            /* file name length */
	char name[0];                      /* file name */
} file_t;


/**********************************************************************

    Structure    : fstat_t 
    Purpose      : this corresponds to the per-process file structure.
                   Mainly care about the current offset for the file
                   for determining where to read or write

***********************************************************************/

/* in-memory data structure for a file operation status */
typedef struct fstat {
	file_t *file;          /* pointer to system-wide in-memory file reference */
	unsigned int offset;   /* current offset index for reads/writes/seeks */
} fstat_t;


/**********************************************************************

    Structure    : ddh_t 
    Purpose      : represents a location for a dentry on the disk -- 
                   which block and which slot in the block -- used 
                   for dentry hash table entries (linked list)

***********************************************************************/

/* disk directory entry hash component */
typedef struct ddentry_hash {
	unsigned short next_dentry; /* next dentry block in hash table */
	unsigned short next_slot;   /* next dentry slot in hash table */
} ddh_t;


/**********************************************************************

    Structure    : ddentry_t  
    Purpose      : represents an on-disk directory entry -- includes
                   the file name and index for the first block in the file
                   (also reference to next dentry for hash table's list)

***********************************************************************/

/* disk directory entry on disk */
typedef struct ddentry {
	unsigned int block;         /* block number of first block of file */
	ddh_t next;                 /* next dentry in hash table */
	unsigned int name_size;     /* file name length */
	char name[MAX_NAME_SIZE];   /* file name - allocate space since preallocated in dblock */
} ddentry_t;


/**********************************************************************

    Structure    : dentry_t  
    Purpose      : represents an in-memory directory entry -- includes
                   the file name, file pointer, on-disk dentry memory 
                   location, and next entry in the in-memory hash table

***********************************************************************/

/* in-memory directory entry stores a reference to a file in a directory */
typedef struct dentry {
	file_t *file;              /* file reference */
	ddentry_t *diskdentry;     /* reference to corresponding on-disk structure */
	struct dentry *next;       /* next dentry in the list */
	unsigned int name_size;    /* file name length */
	char name[0];              /* file name */
} dentry_t;


/**********************************************************************

    Structure    : ddir_t  
    Purpose      : represents an on-disk directory -- each directory 
                   stores a hash table of on-disk directory entries 
                   (only the first is stored in this block -- the rest 
                    are obtained from ddh_t info), a reference to the 
                    first free spot for a new ddentry (freeblk for block
                    and free for free dentry slot)
                   data stores the hash table buckets

***********************************************************************/

/* disk data structure for the directory */
/* directory stores a hash table to find files */
typedef struct ddir {
	unsigned int buckets;        /* number of hash table buckets in the directory */
	unsigned int freeblk;        /* number of the next free block for dentry */
	unsigned int free;           /* first free dentry in block */
	ddh_t data[0];               /* reference to the hash table */
} ddir_t;


/**********************************************************************

    Structure    : dir_t  
    Purpose      : represents an in-memory directory -- each directory 
                   stores a hash table of in-memory directory entries
                   (normal in-memory hash table) and reference to 
                   disk directory in ramdisk

***********************************************************************/

typedef struct dir {
	unsigned int bucket_size; /* number of buckets in the dentry hash table */
	dentry_t **buckets;       /* hash table for directory entries */
	ddir_t *diskdir;          /* reference to the directory in ramdisk */
} dir_t;


/**********************************************************************

    Structure    : proc_t
    Purpose      : represents the in memory process -- just to store 
                   a reference the per-process file table

***********************************************************************/

/* in-memory per-process file table */
typedef struct process {
	fstat_t **fstat_table;     /* per-process open file table */
} proc_t;


/**********************************************************************

    Structure    : dfilesys
    Purpose      : represents the on-disk file system -- stores the 
                   overall file system information: number of blocks, 
                   the firstfree block on disk, and the block for the 
                   root directory

***********************************************************************/

/* data structure for a file system */
typedef struct dfilesys {
	unsigned int bsize;     /* number of blocks in the file system */
	unsigned int firstfree; /* offset in blocks to the first free block */ 
	unsigned int root;      /* offset to the root directory block */
} dfilesys_t;


/**********************************************************************

    Structure    : filesys
    Purpose      : represents the in-memory file system (superblock) 
                   and ad hoc info about bootstrapping the ramdisk --
                   fd refs to the file descriptor for the ramdisk file,
                   base is the base memory address of the ramdisk, 
                   sb is file stat for the ramdisk file,
                   dir is the in-memory root directory, 
                   filetable is the system-wide, in-memory file table, 
                   proc is our process (yes only one at a time, please)
                   function pointers for the file system commands

***********************************************************************/

/* in-memory info for the file system */
typedef struct filesys {
	int fd;                 /* mmap'd fs file descriptor */
	void *base;             /* mmap pointer -- start of mmapped region */
	struct stat sb;         /* stat buffer for mmapped file */
	dir_t *dir;             /* root directory in the file system (only directory) */
	file_t **filetable;     /* system-wide open file table */
	block_t **block_cache;  /* cache of blocks read into memory */
	proc_t *proc;           /* process making requests to the fs */
	int (*create)( char *name, unsigned int flags );      /* file create */
	int (*open)( char *name, unsigned int flags );        /* file open */
	void (*list)( void );                                 /* list directory */
	void (*close)( unsigned int fd );                     /* close file descriptor */
	int (*read)( unsigned int fd, char *buf, unsigned int bytes );       /* file read */
	int (*write)( unsigned int fd, char *buf, unsigned int bytes );      /* file write */
	int (*seek)( unsigned int fd, unsigned int index );    /* file seek */
	/* Project 4: These are the new function pointers for the xattr function */
	int (*setxattr)( unsigned int fd, char *name, char *value, unsigned int name_size, 
			 unsigned int value_size, unsigned int flags );    /* set attribute */
	int (*getxattr)( unsigned int fd, char *name, char *value, unsigned int name_size, 
			 unsigned int size );    /* get attribute */
} filesys_t;


/* Global variables */
filesys_t *fs;
dfilesys_t *dfs;


/**********************************************************************

    Function    : fsReadDir
    Description : Return directory entry for name
    Inputs      : name - name of the file 
                  name_size - length of file name
    Outputs     : new in-memory directory or NULL

***********************************************************************/

extern dir_t *fsReadDir( char *name, unsigned int name_size );

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

extern file_t *fsFileInitialize( dentry_t *dentry, char *name, unsigned int flags, 
				 unsigned int name_size, fcb_t *fcb );

/**********************************************************************

    Function    : fsDentryInitialize
    Description : Create a memory dentry for this file
    Inputs      : name - file name 
                  disk_dentry - on-disk dentry object (optional)
                  name_size - length of file name
    Outputs     : new directory entry or NULL

***********************************************************************/

extern dentry_t *fsDentryInitialize( char *name, ddentry_t *disk_dentry, unsigned int name_size );

/**********************************************************************

    Function    : fsAddDentry
    Description : Add the dentry to its directory
    Inputs      : dir - directory object
                  dentry - dentry object
    Outputs     : 0 if success, -1 if error 

***********************************************************************/

extern int fsAddDentry( dir_t *dir, dentry_t *dentry );

/**********************************************************************

    Function    : fsAddFile
    Description : Add the file to the system-wide open file cache
    Inputs      : filetable - system-wide file table 
                  file - file to be added
    Outputs     : a file descriptor, or -1 on error 

***********************************************************************/

extern int fsAddFile( file_t **filetable, file_t *file);

/**********************************************************************

    Function    : fileCreate
    Description : Create directory entry and file object
    Inputs      : name - name of the file 
                  flags - creation options 
    Outputs     : new file descriptor or -1 if error

***********************************************************************/

extern int fileCreate( char *name, unsigned int flags );

/**********************************************************************

    Function    : fileOpen
    Description : Open directory entry of specified name
    Inputs      : name - name of the file 
                  flags - creation options
    Outputs     : new file descriptor or -1 if error

***********************************************************************/

extern int fileOpen( char *name, unsigned int flags );

/**********************************************************************

    Function    : fileWrite
    Description : Write specified number of bytes starting at the current file index
    Inputs      : fd - file descriptor
                  buf - buffer to write
                  bytes - number of bytes to write
    Outputs     : number of bytes written

***********************************************************************/

extern int fileWrite( unsigned int fd, char *buf, unsigned int bytes );

/**********************************************************************

    Function    : fileRead
    Description : Read specified number of bytes from the current file index
    Inputs      : fd - file descriptor
                  buf - buffer for placing data
                  bytes - number of bytes to read
    Outputs     : number of bytes read

***********************************************************************/

extern int fileRead( unsigned int fd, char *buf, unsigned int bytes );

/**********************************************************************

    Function    : fsFindDentry
    Description : Retrieve the dentry for this file name
    Inputs      : dir - directory in which to look for name 
                  name - name of the file 
                  name_size - length of file name
    Outputs     : new dentry or NULL if error

***********************************************************************/

extern dentry_t *fsFindDentry( dir_t *dir, char *name, unsigned int name_size );

/**********************************************************************

    Function    : fsFindFile
    Description : Retrieve the in-memory file for this file name
    Inputs      : dentry - dentry in which to look for name 
                  name - name of the file 
                  flags - flags requested for this file
                  name_size - length of file name
    Outputs     : new file or NULL if error

***********************************************************************/

extern file_t *fsFindFile( dentry_t *dentry, char *name, unsigned int flags, unsigned int name_size );

/**********************************************************************

    Function    : listDirectory
    Description : print the files in the root directory currently
    Inputs      : none
    Outputs     : number of bytes read

***********************************************************************/

extern void listDirectory( void );



/**********************************************************************

    Function    : fileClose
    Description : close the file associated with the file descriptor
    Inputs      : fd - file descriptor
    Outputs     : none

***********************************************************************/

extern void fileClose( unsigned int fd );

/**********************************************************************

    Function    : fileSeek
    Description : Adjust offset in per-process file entry
    Inputs      : fd - file descriptor
                  index - new offset 
    Outputs     : 0 on success, -1 on failure

***********************************************************************/

extern int fileSeek( unsigned int fd, unsigned int index );

/**********************************************************************

    Function    : fileWrite
    Description : Write specified number of bytes starting at the current file index
    Inputs      : fd - file descriptor
                  buf - buffer to write
                  bytes - number of bytes to write
    Outputs     : number of bytes written

***********************************************************************/

extern int fileWrite( unsigned int fd, char *buf, unsigned int bytes );

/* Project 4: function prototypes for fileSetAttr and fileGetAttr */

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

extern int fileSetAttr( unsigned int fd, char *name, char *value, unsigned int name_size, 
			unsigned int value_size, unsigned int flags );

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

extern int fileGetAttr( unsigned int fd, char *name, char *value, unsigned int name_size, 
		 unsigned int size );
