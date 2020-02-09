
/**********************************************************************

   File          : cmpsc473-list.h

   Description   : This file contains the list prototypes

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


/**********************************************************************

    Function    : addToList
    Description : add an element to the specified list (at head)
    Inputs      : list - ptr to the list
                  elt - new element for the list
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

extern int addToList( dentry_t **list, dentry_t *elt );


/**********************************************************************

    Function    : removeFromList
    Description : remove the specified element from the list 
    Inputs      : list - ptr to the list 
                  elt - element to remove from the list
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

extern int removeFromList( dentry_t **list, dentry_t *remove );

/**********************************************************************

    Function    : listSize
    Description : Return the number of entries in the list
    Inputs      : list - ptr to the list 
    Outputs     : count of number of entries

***********************************************************************/

extern int listSize( dentry_t *list );

/**********************************************************************

    Function    : listEntry
    Description : Return the list entry at position n
    Inputs      : list - ptr to the list 
                  n - index for target entry
    Outputs     : dentry entry

***********************************************************************/

extern dentry_t *listEntry( dentry_t *list, int n );

/**********************************************************************

    Function    : inList
    Description : Return entry if in list
    Inputs      : list - ptr to the list 
                  name - name of entry
                  name_size - length of name
    Outputs     : dentry entry

***********************************************************************/

extern dentry_t *inList( dentry_t *list, char *name, unsigned int name_size );
