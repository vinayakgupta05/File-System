
/**********************************************************************

   File          : cmpsc473-util.c

   Description   : This is a set of utility functiosn for the CMPSC473
                   projects. (see .h for applications)

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
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>
#include <cmpsc473-util.h>

/* Functions */

/**********************************************************************

    Function    : errorMessage
    Description : prints an error mesage to stderr
    Inputs      : msg - pointer to string message
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int errorMessage( char *msg ) 
{
	/* Print message and return */
	fprintf( stderr, "CMPSC473 Error: %s\n", msg );
	return( 0 );
}

/**********************************************************************

    Function    : warningMessage
    Description : prints an warning mesage to stderr
    Inputs      : msg - pointer to string message
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int warningMessage( char *msg ) 
{
	/* Print message and return */
	fprintf( stderr, "CMPSC473 Warning: %s\n", msg );
	return( 0 );
}


/**********************************************************************

    Function    : printBuffer
    Description : prints buffer to stdout
    Inputs      : msg - header message
                  buf - the buffer
                  len - the length of the buffer
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

void printBuffer( char *msg, char *buf, int len )
{
	/* Print message and return */
	int i;
	if ( msg != NULL ) printf( "%s : ", msg );
	if ( buf != NULL )
	{
		for ( i=0; i<len; i++ ) 
		{
			printf( "%2X ", (unsigned char)buf[i] );
		}
	}
	else
	{
		printf( "(null)" );
	}
	printf( "\n" );
	return;
}

/**********************************************************************

    Function    : readline
    Description : reads a line from file referenced by fd
    Inputs      : fd - file descriptor
                  buf - output buffer 
                  maxlen - size of buffer
    Outputs     : number of bytes if successful, -1 if failure

***********************************************************************/

int readline(int fd, char *buf, int maxlen)
{
	int rc, n;
	char c, *ptr;
	int firstchar = 1;

	ptr = buf;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = read(fd, &c, 1)) == 1) {
			*ptr++ = c;
			if ( firstchar && ((c == '\n') || (c == ' ') || (c == '\t') )) continue;
			firstchar = 0;
			if (c == '\n') {
				*(ptr-1) = 0;
				break;
			}
		} else if (rc == 0) {
			if (n == 1)
				return(0);/* EOF, no data read */
			else
				break;/* EOF, some data was read */
		} else
			return(-1);/* error */
	}

	*ptr = 0;
	return(n);
}

