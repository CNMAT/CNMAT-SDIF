/* 
Copyright (c) 2004.  The Regents of the University of
California (Regents).  All Rights Reserved.

Written by Ben "Jacobs" and Matt Wright, The Center for New Music and Audio
Technologies, University of California, Berkeley.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
     ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
     REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
*/

/* sdif-buf-private.h

  Limited interface to internal data structures, for friend classes only
  (public interface defined in "sdif-buf.h" should be enough for typical users)

  authors: Ben "Jacobs" and Matt Wright
  2004/03/19 (0.1.0) Refactored non max-specific code from SDIF-buffer.c (0.7.1) - bj
  2004/06/23 (0.1.1) cleanup - bj

  NOTES
  
  Assume user has already included these headers:
  "sdif.h", "sdif-mem.h", and "sdif-buf.h"
*/



/****************************************************/
/*                                                  */
/* friends interface to SDIFbuf_Buffer class fields */
/*                                                  */
/****************************************************/

typedef struct _SDIFbuf_BufferClassFriends 
{
  SDIFbuf_Buffer first;                   //  head of doubly-linked list

	int debug;			                        //  0 or non-zero for debug mode
} SDIFbuf_BufferClassFriends;


/*******************************************************/
/*                                                     */
/* friends interface to SDIFbuf_Buffer instance fields */
/*                                                     */
/*******************************************************/

typedef struct _SDIFbuf_BufferFriends 
{
 	//  state info for an SDIFbuf_Buffer instance
 	sdif_int32   streamID;
 	char frameType[4];
 	sdif_float64 min_time;
 	sdif_float64 max_time; 	

	SDIFmem_Frame head, tail;               //  pointers to ends of doubly-linked list

	int debug;			                        //  0 or non-zero for debug mode
  void *internal;                         //  stuff we don't even show our friends
} *SDIFbuf_BufferFriends;


/*****************************************************/
/*                                                   */
/* friends interface to SDIFbuf_Buffer class methods */
/*                                                   */
/*****************************************************/

/* SDIFbuf_FriendsClass (return class data for friends) 
*/
SDIFbuf_BufferClassFriends *SDIFbuf_GetBufferClassFriends(void);


/********************************************************/
/*                                                      */
/* friends interface to SDIFbuf_Buffer instance methods */
/*                                                      */
/********************************************************/

/* SDIFbuf_Friends (return instance data for friends) 
*/
SDIFbuf_BufferFriends SDIFbuf_GetBufferFriends(SDIFbuf_Buffer b);

