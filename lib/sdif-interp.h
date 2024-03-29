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

/* sdif-interp.h

  Public interface to functions for interpolating generic SDIF data in an SDIFbuf_Buffer instance:
  - 
  
  Future features:
  -

  author: Ben "Jacobs"
  2004/03/22 (0.1.0) Implementation of initial feature set - bj
  2004/06/23 (0.1.1) cleanup - bj

  NOTES
  
  sdif-interp.c accesses "friend-private" fields of an SDIFbuf_Buffer instance
  through the interface defined in sdif-buf-private.h; users of sdif-interp.c
  shouldn't include the private headers
  
  Return value/error message conventions: same as sdif.[ch], except
  for the constructor/lookup procedures, which return a pointer to an object
  or zero if out of memory / not found. 

  This SDIF library is neither reentrant nor thread-safe.

  Assume user has already included these headers:
  "sdif.h", "sdif-mem.h", "sdif-buf.h"
*/


/**********************************/
/*                                */
/* public types, enums, constants */
/*                                */
/**********************************/


/***************************************************************/
/*                                                             */
/* public interface to SDIFinterp_Interpolator instance fields */
/*                                                             */
/***************************************************************/

typedef struct _SDIFinterpInterpolatorStruct *SDIFinterp_Interpolator;
typedef struct _SDIFinterpInterpolatorStruct
{
  // SDIFinterp_Interpolator public instance data 
	SDIFinterp_Interpolator prev, next;    // doubly linked list of all SDIFinterp_Interpolator instances
 	void *internal;                        // private stuff (defined in "sdif-interp-private.h")
} SDIFinterpInterpolatorStruct;


/******************************************/
/*                                        */
/* public callback function pointer types */
/*                                        */
/******************************************/

/* an SDIFinterp_InterpolatorFn is a user interpolation function,
   which generates a column of interpolated matrix data for the specified time
   (result data is written into matrixOut)
   NOTE: data type is assumed to be SDIF_FLOAT32 or SDIF_FLOAT64 for all matrices
   NOTE: additional variable arg list contains interpolation parameters
         (it is the user's responsibility to send the correct number + type of
          parameter arguments for the requested interpolator function)
*/
typedef SDIFresult (SDIFinterp_InterpolatorFn)(SDIFmem_Frame nearbyFrame,
                                               const char *matrixType,
                                               sdif_int32 column,
                                               sdif_float64 time,
                                               SDIFactionOnNaN actionOnNaN,
                                               SDIFmem_Matrix matrixOut,
                                               va_list args
                                               );


/*******************************************************/
/*                                                     */
/* public interface to SDIFinterp_Interpolator methods */
/*                                                     */
/*******************************************************/

/*----------------------*/
/* Class Initialization */
/*----------------------*/

/* SDIFinterp_Init --
   You must call this before any of the other procedures in this library.
   
   NOTE: this module requires sdif-buf.c, so make sure to call SDIFbuf_Init() too, 
   and be sure to call SDIFmem_Init() first, to set up the user-provided memory 
   alloc/free functions.
*/
SDIFresult SDIFinterp_Init(void);


/*-------------- */
/* Class Methods */
/*---------------*/

/* SDIFinterp_Create (create an SDIFinterp_Interpolator instance) --
   Returns NULL if out of memory.
*/
SDIFinterp_Interpolator SDIFinterp_Create(sdif_int32 columns);


/* SDIFinterp_Free (free an SDIFinterp_Interpolator instance)
*/
void SDIFinterp_Free(SDIFinterp_Interpolator it);


/* SDIFinterp_FreeAll (free all SDIFinterp_Interpolator instances in list) --
*/
void SDIFinterp_FreeAll(void);


/* SDIFinterp_GetFirstInterpolator (return first SDIFinterp_Interpolator instance in list) --
   Returns NULL if no instances in list.
*/
SDIFinterp_Interpolator SDIFinterp_GetFirstInterpolator(void);


/*----------------- */
/* Instance Methods */
/*------------------*/

/* SDIFinterp_Clear (release SDIF data contained within an SDIFinterp_Interpolator instance)
*/
void SDIFinterp_Clear(SDIFinterp_Interpolator it);


/* SDIFinterp_SetInterpolatorFn 
   (set interpolator callback function for one column of matrix)
   NOTE: column param is 0-based
*/
SDIFresult SDIFinterp_SetInterpolatorFn(SDIFinterp_Interpolator it,
                                  sdif_int32 column,
                                  SDIFinterp_InterpolatorFn *fn
                                  );


/* SDIFinterp_SetAllInterpolatorFn 
   (set interpolator callback function for all cells in matrix) 
*/
SDIFresult SDIFinterp_SetAllInterpolatorFn(SDIFinterp_Interpolator it,
                                           SDIFinterp_InterpolatorFn *fn
                                           );


/* SDIFinterp_GetMatrix 
   (interpolate to get specified matrix from an SDIFbuf_Buffer instance at specified time)
   - results returned in matrixOut
   - additional var args are passed to the interpolator function
   NOTE: dimensions of matrixOut and the matrices being fetched from the SDIFbuf_Buffer
         are assumed to be same as the SDIFinterp_Interpolator instance's own matrix
   NOTE: data types of all matrices are assumed to be SDIF_FLOAT32 or SDIF_FLOAT64
*/
SDIFresult SDIFinterp_GetMatrix(SDIFinterp_Interpolator it,
                                SDIFbuf_Buffer b,
                                const char *matrixType,
                                sdif_float64 time,
                                SDIFactionOnNaN actionOnNaN,
                                SDIFmem_Matrix matrixOut,
                                ...
                                );


/* SDIFinterp_GetMatrixNearby 
   (interpolate to get specified matrix for the specified time; linear search for matrix
    data begins from the specified frame)
   - results returned in matrixOut
   - additional var args are passed to the interpolator function
   NOTE: dimensions of matrixOut and the matrix being fetched
         are assumed to be same as the SDIFinterp_Interpolator instance's own matrix
*/
SDIFresult SDIFinterp_GetMatrixNearby(SDIFinterp_Interpolator it,
                                      SDIFmem_Frame f,
                                      const char *matrixType,
                                      sdif_float64 time,
                                      SDIFactionOnNaN actionOnNaN,
                                      SDIFmem_Matrix matrixOut,
                                      ...
                                      );

