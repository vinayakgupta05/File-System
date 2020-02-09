
/**********************************************************************

   File          : cmpsc473-list.c

   Description   : File for the list management functions 

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

/* Project Include Files */
#include "cmpsc473-filesys.h"
#include "cmpsc473-disk.h"
#include "cmpsc473-list.h"
#include "cmpsc473-util.h"


/* Functions */

/**********************************************************************

    Function    : addToList
    Description : add an element to the specified list (at head)
    Inputs      : list - ptr to the list
                  elt - new element for the list
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int addToList( dentry_t **list, dentry_t *elt )
{
	if ( list == NULL ) {
		errorMessage("addToList: uninitialized list");
		return -1;
	}
	else {
		elt->next = *list;
		*list = elt;
	}
  
	return 0;
}


/**********************************************************************

    Function    : removeFromList
    Description : remove the specified element from the list 
    Inputs      : list - ptr to the list 
                  elt - element to remove from the list
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int removeFromList( dentry_t **list, dentry_t *remove )
{
	dentry_t *elt = *list, *prev = (dentry_t *) NULL;
  
	while ( elt != NULL ) {
		if ( elt == remove ) {
      
			/* remove head from list */
			if ( prev == NULL ) {
				*list = elt->next;
			}
			/* remove normal elt */
			else {
				prev->next = elt->next;
			}

			return 0;
		}

		prev = elt;
		elt = elt->next;
	}
  
	return -1; /* never found remove in the list */
}


/**********************************************************************

    Function    : listSize
    Description : Return the number of entries in the list
    Inputs      : list - ptr to the list 
    Outputs     : count of number of entries

***********************************************************************/

int listSize( dentry_t *list )
{
	int len = 0;
	dentry_t *elt = list;

	while ( elt != NULL ) {
		len++;
		elt = elt->next;
	}

	return len;
}


/**********************************************************************

    Function    : listEntry
    Description : Return the list entry at position n
    Inputs      : list - ptr to the list 
                  n - index for target entry
    Outputs     : dentry entry

***********************************************************************/

dentry_t *listEntry( dentry_t *list, int n ) 
{
	int len = 0;
	dentry_t *elt = list;
  
	while ( elt != NULL ) {
		if ( len == n ) {
			return elt;
		}
		len++;
		elt = elt->next;
	}

	errorMessage("listEntry: beyond the end of the list");
	return (dentry_t *) NULL;
}


/**********************************************************************

    Function    : inList
    Description : Return entry if in list
    Inputs      : list - ptr to the list 
                  name - name of entry
                  name_size - length of name  
    Outputs     : dentry entry

***********************************************************************/

dentry_t *inList( dentry_t *list, char *name, unsigned int name_size ) 
{
	dentry_t *elt = list;
  
	while ( elt != NULL ) {
		if (( elt->name_size == name_size ) && 
		    ( strncmp( elt->name, name, elt->name_size ) == 0 )) {
			return elt;
		}
		elt = elt->next;
	}

	return (dentry_t *) NULL;
}
