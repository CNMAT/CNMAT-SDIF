/* 
Copyright (c) 2004.  The Regents of the University of
California (Regents).  All Rights Reserved.

Written by Ben "Jacobs", The Center for New Music and Audio
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

/* sdif-interp-private.h

  Limited interface to internal data structures, for friend classes only
  (public interface defined in "sdif-interp.h" should be enough for typical users)

  author: Ben "Jacobs"
  2004/03/22 (0.1.0) implementation of initial feature set - bj
  2004/06/23 (0.1.1) cleanup - bj

  NOTES
  
  Assume user has already included these headers:
  "sdif.h", "sdif-mem.h", "sdif-buf.h", "sdif-buf-private.h", "sdif-interp.h"
*/


/*************************************************************/
/*                                                           */
/* friends interface to SDIFinterp_Interpolator class fields */
/*                                                           */
/*************************************************************/

typedef struct _SDIFinterp_InterpolatorClassFriends 
{
  SDIFinterp_Interpolator first;          //  head of doubly-linked list

	int debug;			                        //  0 or non-zero for debug mode
} SDIFinterp_InterpolatorClassFriends;


/****************************************************************/
/*                                                              */
/* friends interface to SDIFinterp_Interpolator instance fields */
/*                                                              */
/****************************************************************/

typedef struct _SDIFinterp_InterpolatorFriends 
{
 	//  state info for an SDIFinterp_Interpolator instance
  SDIFmem_Matrix mat;                     //  the interpolator matrix
                                          //  (cells contain ptrs to interpolator functions)

	int debug;			                        //  0 or non-zero for debug mode
  void *internal;                         //  stuff we don't even show our friends
} *SDIFinterp_InterpolatorFriends;


/**************************************************************/
/*                                                            */
/* friends interface to SDIFinterp_Interpolator class methods */
/*                                                            */
/**************************************************************/

/* SDIFinterp_GetInterpolatorClassFriends (return class data for friends) 
*/
SDIFinterp_InterpolatorClassFriends *SDIFinterp_GetInterpolatorClassFriends(void);


/*****************************************************************/
/*                                                               */
/* friends interface to SDIFinterp_Interpolator instance methods */
/*                                                               */
/*****************************************************************/

/*  SDIFinterp_GetInterpolatorFriends (return instance data for friends) 
*/
SDIFinterp_InterpolatorFriends SDIFinterp_GetInterpolatorFriends(SDIFinterp_Interpolator it);


