/*
Copyright (c) 2004.  The Regents of the University of California (Regents).
All Rights Reserved.

Written by Matt Wright and Ben "Jacobs", The Center for New Music and Audio
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

/*
  sdif-interp.c
  
  Implementation of routines for interpolating SDIF data in an SDIFbuf_Buffer instance
  (see sdif-interp.h for feature list, documentation of public interface)

  author: Ben "Jacobs"
  2004/03/22 (0.1.0) implementation of initial feature set - bj
  2004/06/23 (0.1.0) cleanup - bj

*/


#define SDIF_INTERP_VERSION "0.1.1"

#include <stdio.h>
#include <string.h> /* for strerror() */
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include "sdif.h"
#include "sdif-mem.h"
#include "sdif-buf.h"
#include "sdif-buf-private.h"
#include "sdif-interp.h"
#include "sdif-interp-private.h"


/***********************/
/*                     */
/* private class types */
/*                     */
/***********************/

#ifndef SDIF_BOOLEAN
typedef unsigned char Boolean;
#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0
#define SDIF_BOOLEAN
#endif


/************************/
/*                      */
/* private class fields */
/*                      */
/************************/

typedef struct _SDIFinterp_InterpolatorClassPrivate 
{
  SDIFinterp_InterpolatorClassFriends friends;   //  class fields accessible to friends only
} SDIFinterp_InterpolatorClassPrivate;

SDIFinterp_InterpolatorClassPrivate SDIFinterp_globals;


/***************************/
/*                         */
/* private instance fields */
/*                         */
/***************************/

typedef struct _SDIFinterp_InterpolatorPrivate 
{
  int dummy;
} *SDIFinterp_InterpolatorPrivate;


/****************************************/
/*                                      */
/* prototypes for private class methods */
/*                                      */
/****************************************/


/*******************************************/
/*                                         */
/* prototypes for private instance methods */
/*                                         */
/*******************************************/

static void instance_reset(SDIFinterp_Interpolator it);


/*  local_GetNearbyMatrix
    (common base code for SDIFinterp_GetMatrixNearby() and SDIFinterp_GetMatrix())
*/
SDIFresult local_GetNearbyMatrix(SDIFinterp_Interpolator it,
                                 SDIFmem_Frame f,
                                 const char *matrixType,
                                 sdif_float64 time,
                                 SDIFactionOnNaN actionOnNaN,
                                 SDIFmem_Matrix matrixOut,
                                 va_list va
                                 );


/***********************************/
/*                                 */
/* implementation of class methods */
/*                                 */
/***********************************/

SDIFresult SDIFinterp_Init(void) 
{
  //  initialize friends fields
  SDIFinterp_globals.friends.first = NULL;
  SDIFinterp_globals.friends.debug = 0;

  return ESDIF_SUCCESS;
}


SDIFinterp_Interpolator SDIFinterp_Create(sdif_int32 columns) 
{
  SDIFinterp_Interpolator it;
  SDIFinterp_InterpolatorFriends itp;
  SDIFinterp_InterpolatorPrivate itpp;
  SDIFbuf_BufferClassFriends *bcf = SDIFbuf_GetBufferClassFriends();
  
  SDIFinterp_Interpolator next;

  //  alloc public instance data storage
  it = SDIFmem_Alloc(sizeof(*it));
  if(!it)
    return NULL;

  //  alloc friends-private instance data storage
  it->internal = itp = SDIFmem_Alloc(sizeof(*itp));
  if(!itp)
  {
    SDIFinterp_Free(it);
    return NULL;
  }
  
  //  alloc file-private instance data storage
  itp->internal = itpp = SDIFmem_Alloc(sizeof(*itpp));
  if(!itpp)
  {
    SDIFinterp_Free(it);
    return NULL;
  }

  //  allocate interpolator matrix + data storage
  itp->mat = SDIFmem_CreateEmptyMatrix();
  if(!itp->mat)
  {
    SDIFinterp_Free(it);
    return NULL;
  }
  
  itp->mat->data = SDIFmem_Alloc(sizeof(SDIFinterp_InterpolatorFn *) * columns);
  if(!itp->mat->data)
  {
    SDIFinterp_Free(it);
    return NULL;
  }

  SDIF_Copy4Bytes(itp->mat->header.matrixType, "----"); //  doesn't matter  
  itp->mat->header.matrixDataType = SDIF_INT32;   //  actually, the cells will hold function ptrs
  itp->mat->header.columnCount = columns;
  itp->mat->header.rowCount = 1;

  //  reset all public + private instance fields to initial state
  instance_reset(it);
  
  //  link new instance into list as first element
  if(SDIFinterp_globals.friends.first)
  {
    //  non-empty list
    next = SDIFinterp_globals.friends.first;
    SDIFinterp_globals.friends.first = it;
    it->prev = NULL;
    it->next = next;
    next->prev = it;
  }
  else
  {
    //  empty list
    SDIFinterp_globals.friends.first = it;
    it->prev = NULL;
    it->next = NULL;
  }
  
  return it;
}


void SDIFinterp_Free(SDIFinterp_Interpolator it) 
{
  SDIFinterp_InterpolatorFriends itp = NULL;
  SDIFinterp_InterpolatorPrivate itpp = NULL;
  SDIFbuf_BufferClassFriends *bcf = SDIFbuf_GetBufferClassFriends();

  SDIFinterp_Interpolator prev;
  SDIFinterp_Interpolator next;
  
  itp = it->internal;
  if(itp)
    itpp = itp->internal;

  //  unlink instance from list
  prev = it->prev;
  next = it->next;
  if(prev)
    prev->next = next;
  else
    SDIFinterp_globals.friends.first = next;
  if(next)
    next->prev = prev;
  
  //  free instance storage (even if malloc failed during instance construction)
  if(itp)
  {
    if(itpp)
    {
      //  free SDIF data contained in current instance
      SDIFinterp_Clear(it);

      SDIFmem_Free(itpp, sizeof(*itpp));
    }

    if(itp->mat)
      SDIFmem_FreeMatrix(itp->mat);
    SDIFmem_Free(itp, sizeof(*itp));
  }

  SDIFmem_Free(it, sizeof(*it));
}


void SDIFinterp_FreeAll(void)
{
  while(SDIFinterp_GetFirstInterpolator())
    SDIFinterp_Free(SDIFinterp_GetFirstInterpolator());
}


SDIFinterp_InterpolatorClassFriends *SDIFinterp_GetInterpolatorClassFriends(void)
{
  return &SDIFinterp_globals.friends;
}


SDIFinterp_Interpolator SDIFinterp_GetFirstInterpolator(void)
{
  return SDIFinterp_globals.friends.first;
}


/**************************************/
/*                                    */
/* implementation of instance methods */
/*                                    */
/**************************************/

void SDIFinterp_Clear(SDIFinterp_Interpolator it) 
{
  SDIFinterp_InterpolatorFriends itp = it->internal;
  SDIFinterp_InterpolatorPrivate itpp = itp->internal;
  
  //  free SDIF data contained in current instance
  //  (NOTE: we don't throw away dynamic storage which was allocated in constructor)
  
  //  reset all public + private instance fields to initial state
  instance_reset(it);

  //  NOTE: this instance must be returned in a state ready to be reused
}  


/*  instance_reset()
    reset all public + private instance fields to initial state
    NOTE: assume storage for internal data already successfully allocated
    NOTE: assume our pointers aren't holding sole reference to alloc storage
*/
static void instance_reset(SDIFinterp_Interpolator it)
{
  SDIFinterp_InterpolatorFriends itp = it->internal;
  SDIFinterp_InterpolatorPrivate itpp = itp->internal;
  SDIFinterp_InterpolatorFn **fnptr;
  int i;

  //  initialize public instance fields
  //  (NOTE: we don't unlink this instance from list by initializing prev + next here)

  //  initialize friends instance fields
  //  (NOTE: we don't throw away dynamic storage which was allocated in constructor)
  itp->debug = 0;

  //  clear the interpolator function matrix
  fnptr = itp->mat->data;
  for(i = 0; i < (itp->mat->header.rowCount * itp->mat->header.columnCount); i++)
    fnptr[i] = NULL;

  //  initialize private instance fields
  //  (NOTE: we don't throw away dynamic storage which was allocated in constructor)
}


SDIFinterp_InterpolatorFriends SDIFinterp_GetInterpolatorFriends(SDIFinterp_Interpolator it)
{
  return (SDIFinterp_InterpolatorFriends)(it->internal);
}


SDIFresult SDIFinterp_SetInterpolatorFn(SDIFinterp_Interpolator it,
                                  sdif_int32 column,
                                  SDIFinterp_InterpolatorFn *fn
                                  )
{
  SDIFinterp_InterpolatorFriends itp = it->internal;
  SDIFinterp_InterpolatorFn **data = itp->mat->data;
  
  data[column] = fn;
  
  return ESDIF_SUCCESS;
}


SDIFresult SDIFinterp_SetAllInterpolatorFn(SDIFinterp_Interpolator it,
                                     SDIFinterp_InterpolatorFn *fn
                                     )
{
  int i;
  SDIFinterp_InterpolatorFriends itp = it->internal;
  SDIFinterp_InterpolatorFn **data = itp->mat->data;
  
  for(i = 0; i < itp->mat->header.columnCount; i++)
    data[i] = fn;

  return ESDIF_SUCCESS;
}


SDIFresult SDIFinterp_GetMatrix(SDIFinterp_Interpolator it,
                                SDIFbuf_Buffer b,
                                const char *matrixType,
                                sdif_float64 time,
                                SDIFactionOnNaN actionOnNaN,
                                SDIFmem_Matrix matrixOut,
                                ...
                                )
{
  SDIFresult r;
  SDIFmem_Frame nearbyFrame;
  SDIFinterp_InterpolatorFriends itp = it->internal;
  SDIFinterp_InterpolatorPrivate itpp = itp->internal;
  
  //  get interpolation parameters (variable argument list starting after matrixOut)
  //  NOTE: assume that user is sending correct arguments to the interpolation function
  va_list va;
  va_start(va, matrixOut);

  //  get a frame from buffer, at requested time or as close as possible before
  //  (for search efficiency, interpolator function will start searches from here)
  //  NOTE: this frame doesn't need to actually contain the desired matrix
  if(!(nearbyFrame = SDIFbuf_GetFrame(b, time, ESDIF_SEARCH_BACKWARDS)))

    //  otherwise, just get first frame in buffer
    if(!(nearbyFrame = SDIFbuf_GetFirstFrame(b)))

      //  if there are no frames in buffer, we can't interpolate
      return ESDIF_NOT_AVAILABLE;
  
  r = local_GetNearbyMatrix(it,
                            nearbyFrame,
                            matrixType,
                            time,
                            actionOnNaN,
                            matrixOut,
                            va
                            );

  va_end(va);
  
  return r;
}


SDIFresult SDIFinterp_GetMatrixNearby(SDIFinterp_Interpolator it,
                                      SDIFmem_Frame nearbyFrame,
                                      const char *matrixType,
                                      sdif_float64 time,
                                      SDIFactionOnNaN actionOnNaN,
                                      SDIFmem_Matrix matrixOut,
                                      ...
                                      )
{
  SDIFresult r;
  
  //  get interpolation parameters (variable argument list starting after matrixOut)
  //  NOTE: assume that user is sending correct arguments to the interpolation function
  va_list va;
  va_start(va, matrixOut);

  r = local_GetNearbyMatrix(it,
                            nearbyFrame,
                            matrixType,
                            time,
                            actionOnNaN,
                            matrixOut,
                            va
                            );
  
  va_end(va);
  
  return r;
}


SDIFresult local_GetNearbyMatrix(SDIFinterp_Interpolator it,
                                 SDIFmem_Frame nearbyFrame,
                                 const char *matrixType,
                                 sdif_float64 time,
                                 SDIFactionOnNaN actionOnNaN,
                                 SDIFmem_Matrix matrixOut,
                                 va_list va
                                 )
{
  int i;
  SDIFresult r = ESDIF_SUCCESS;
  sdif_int32 columns = matrixOut->header.columnCount;

  SDIFinterp_InterpolatorFriends itp = it->internal;
  SDIFinterp_InterpolatorPrivate itpp = itp->internal;
  SDIFinterp_InterpolatorFn **data = itp->mat->data;
  
  //  call user interpolation function to fill in each cell of output matrix
  //  (provide ptr to nearbyFrame so interpolator can search for source data)
  for(i = 0; i < columns; i++)
  {
    SDIFinterp_InterpolatorFn *fn = data[i];

    //  call interpolator for each column
    //  (skip column if its interpolator is NULL)
    if(fn)
      if(ESDIF_SUCCESS != (r = fn(nearbyFrame, matrixType, i, time, actionOnNaN, matrixOut, va)))
        break;
  }

  return r;
}
