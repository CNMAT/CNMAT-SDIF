/*
Copyright (c) 2004.  The Regents of the University of California (Regents).
All Rights Reserved.

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

/*
  sdif-buf.c
  
  Implementation of routines for managing SDIF data in memory
  (see sdif-buf.h for feature list, documentation of public interface)

  authors: Ben "Jacobs" and Matt Wright
  2004/03/19 (0.1.0) Refactored non max-specific code from SDIF-buffer.c (0.7.1) - bj
  2004/06/23 (0.1.1) cleanup - bj

*/


#include <stdio.h>
#include <string.h> /* for strerror() */
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include "sdif.h"
#include "sdif-mem.h"
#include "sdif-buf.h"
#include "sdif-buf-private.h"

#include "ext.h"

/************************/
/*                      */
/* private class fields */
/*                      */
/************************/

typedef struct _SDIFbuf_BufferClassPrivate 
{
  SDIFbuf_BufferClassFriends friends;   //  class fields accessible to friends only
} SDIFbuf_BufferClassPrivate;

SDIFbuf_BufferClassPrivate SDIFbuf_globals;


/***************************/
/*                         */
/* private instance fields */
/*                         */
/***************************/

typedef struct _SDIFbuf_BufferPrivate 
{
  int dummy;
} *SDIFbuf_BufferPrivate;


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

static void instance_reset(SDIFbuf_Buffer b);


/***********************************/
/*                                 */
/* implementation of class methods */
/*                                 */
/***********************************/

SDIFresult SDIFbuf_Init(void) 
{
  //  initialize friends fields
  SDIFbuf_globals.friends.first = NULL;
  SDIFbuf_globals.friends.debug = 0;
  
  return ESDIF_SUCCESS;
}


SDIFbuf_Buffer SDIFbuf_Create(void) 
{
  SDIFbuf_Buffer b;
  SDIFbuf_BufferFriends bp;
  SDIFbuf_BufferPrivate bpp;
  
  SDIFbuf_Buffer next;

  //  alloc public instance data storage
  b = SDIFmem_Alloc(sizeof(*b));
  if(!b)
    return NULL;

  //  alloc friends-private instance data storage
  b->internal = bp = SDIFmem_Alloc(sizeof(*bp));
  if(!bp)
  {
    SDIFbuf_Free(b);
    return NULL;
  }
  
  //  alloc file-private instance data storage
  bp->internal = bpp = SDIFmem_Alloc(sizeof(*bpp));
  if(!bpp)
  {
    SDIFbuf_Free(b);
    return NULL;
  }

  //  reset all public + private instance fields to initial state
  instance_reset(b);
  
  //  link new instance into list as first element
  if(SDIFbuf_globals.friends.first)
  {
    //  non-empty list
    next = SDIFbuf_globals.friends.first;
    SDIFbuf_globals.friends.first = b;
    b->prev = NULL;
    b->next = next;
    next->prev = b;
  }
  else
  {
    //  empty list
    SDIFbuf_globals.friends.first = b;
    b->prev = NULL;
    b->next = NULL;
  }
  
  return b;
}


void SDIFbuf_Free(SDIFbuf_Buffer b) 
{
  SDIFbuf_BufferFriends bp;
  SDIFbuf_BufferPrivate bpp;

  SDIFbuf_Buffer prev;
  SDIFbuf_Buffer next;
  
  //  unlink instance from list
  prev = b->prev;
  next = b->next;
  if(prev)
    prev->next = next;
  else
    SDIFbuf_globals.friends.first = next;
  if(next)
    next->prev = prev;
  
  //  free instance storage (even if malloc failed during instance construction)
  if(bp = b->internal)
  {
    if(bpp = bp->internal)
    {
      //  free SDIF data contained in current instance (follow linked lists)
      SDIFbuf_Clear(b);

      SDIFmem_Free(bpp, sizeof(*bpp));
    }
    SDIFmem_Free(bp, sizeof(*bp));
  }
  SDIFmem_Free(b, sizeof(*b));
}  


void SDIFbuf_FreeAll(void)
{
  while(SDIFbuf_GetFirstBuffer())
    SDIFbuf_Free(SDIFbuf_GetFirstBuffer());
}


SDIFbuf_BufferClassFriends *SDIFbuf_GetBufferClassFriends(void)
{
  return &SDIFbuf_globals.friends;
}


SDIFbuf_Buffer SDIFbuf_GetFirstBuffer(void)
{
  return SDIFbuf_globals.friends.first;
}


SDIFmem_Frame SDIFbuf_GetPrevFrame(SDIFmem_Frame f)
{
  //  future: add code here to retrieve frames from disk or network on demand
  //  (safe code will use this as the only accessor for traversing the list backwards;
  //  frames might not actually exist in memory until the accessor is called)
  return f->prev;
}


SDIFmem_Frame SDIFbuf_GetNextFrame(SDIFmem_Frame f)
{
  //  future: add code here to retrieve frames from disk or network on demand
  //  (safe code will use this as the only accessor for traversing the list forwards;
  //  frames might not actually exist in memory until the accessor is called)
  return f->next;
}


SDIFmem_Frame SDIFbuf_GetFrameNearby(SDIFmem_Frame f, sdif_float64 time, SDIFsearchMode direction)
{
  SDIFmem_Frame cur, prev, next;
  Boolean forwards;
  
  prev = next = NULL;

  //  in which direction should we traverse? (list is sorted by increasing time)
  forwards = (time > f->header.time);

  //  loop to traverse list
  cur = f;
  while(cur)
  {
    if(cur->header.time == time)
      //  exact match -- return this frame
      return cur;
    
    prev = SDIFbuf_GetPrevFrame(cur);
    next = SDIFbuf_GetNextFrame(cur);

    if(forwards)
    {
      //  we are traversing forwards
      if(cur->header.time > time)
      {
        //  current frame is past requested time (too late) -- we're done searching
        if(direction == ESDIF_SEARCH_EXACT)
          //  wanted exact time match only, so fail
          return NULL;
        if(direction == ESDIF_SEARCH_FORWARDS)
          //  wanted frame *after* requested time, so return this frame
          return cur;
        else
          //  wanted frame *before* requested time, so return previous frame
          //  (return NULL if there is no previous frame)
          return prev;
      }
      else
        //  we haven't gotten to requested time yet -- keep looking
        cur = next;
    }
    
    else
    {
      //  we are traversing backwards
      if(cur->header.time < time)
      {
        //  current frame is past requested time (too early) -- we're done searching
        if(direction == ESDIF_SEARCH_EXACT)
          //  wanted exact time match only, so fail
          return NULL;
        if(direction == ESDIF_SEARCH_BACKWARDS)
          //  wanted frame *before* requested time, so return this frame
          return cur;
        else
          //  wanted frame *after* requested time, so return next frame
          //  (return NULL if there is no next frame)
          return next;
      }
      else
        //  we haven't gotten to requested time yet -- keep looking
        cur = prev;
    }
  }
  
  //  we traversed to end of list without passing the requested time

  if(forwards && (direction == ESDIF_SEARCH_BACKWARDS))
  {
    //  traversed forwards, but wanted frame immediately *before* requested time, so backtrack
    if(prev)
      return prev->next;
    else
      //  return frame we started with (it was the latest frame)
      return f;
  }

  if(!forwards && (direction == ESDIF_SEARCH_FORWARDS))
    //  traversed backwards, but wanted frame immediately *after* requested time, so backtrack
    if(next)
      return next->prev;
    else
      //  return frame we started with (it was the earliest frame)
      return f;

  //  fail
  return NULL;
}


SDIFmem_Matrix SDIFbuf_GetMatrixInFrame(SDIFmem_Frame f, const char* matrixType)
{
  SDIFmem_Matrix m;
  
  //  loop through matrices in frame
  for(m = f->matrices; m; m = m->next)
    if(SDIF_Char4Eq(matrixType, m->header.matrixType))
      return m;
  
  return NULL;
}


SDIFmem_Matrix SDIFbuf_GetPrevMatrix(SDIFmem_Frame f, 
                                     const char* matrixType, 
                                     sdif_float64 *timeFound
                                     )
{
  SDIFmem_Frame cf;
  SDIFmem_Matrix m;

  //  loop backwards through frame list
  for(cf = SDIFbuf_GetPrevFrame(f); cf; cf = SDIFbuf_GetPrevFrame(cf))
    if(m = SDIFbuf_GetMatrixInFrame(cf, matrixType))
    {
      *timeFound = cf->header.time;
      return m;
    }
  
  return NULL;
}


SDIFmem_Matrix SDIFbuf_GetNextMatrix(SDIFmem_Frame f, 
                                     const char* matrixType,
                                     sdif_float64 *timeFound
                                     )
{
  SDIFmem_Frame cf;
  SDIFmem_Matrix m;

  //  loop forwards through frame list
  for(cf = SDIFbuf_GetNextFrame(f); cf; cf = SDIFbuf_GetNextFrame(cf))
    if(m = SDIFbuf_GetMatrixInFrame(cf, matrixType))
    {
      *timeFound = cf->header.time;
      return m;
    }
  
  return NULL;
}


SDIFmem_Matrix SDIFbuf_GetMatrixNearby(SDIFmem_Frame f, 
                                       const char* matrixType, 
                                       sdif_float64 time, 
                                       SDIFsearchMode direction,
                                       sdif_float64 *timeFound
                                       )
{
  SDIFmem_Frame cur, prev, next;
  SDIFmem_Matrix m;
  Boolean forwards;
  
  prev = next = NULL;

  //  in which direction should we traverse? (list is sorted by increasing time)
  forwards = (time > f->header.time);

  //  loop to traverse list
  //  (optimized with the assumption that we will be searching through a long list,
  //  where most frames do contain the requested matrix type -- i.e. we don't want to
  //  bother searching for an actual matrix type match until we get to the right time;
  //  we would rather do the small extra work of backtracking by a frame if necessary)
  cur = f;
  while(cur)
  {
    prev = SDIFbuf_GetPrevFrame(cur);
    next = SDIFbuf_GetNextFrame(cur);

    if(cur->header.time == time)
    {
      //  exact match
      if(m = SDIFbuf_GetMatrixInFrame(cur, matrixType))
      {
        //  return this frame
        *timeFound = cur->header.time;
        return m;
      }
      else
        //  keep looking
        cur = forwards ? next : prev;
    }
    
    else if(forwards)
    {
      //  we are traversing forwards
      if(cur->header.time > time)
      {
        //  current frame is past requested time (too late) -- we're done searching
        if(direction == ESDIF_SEARCH_EXACT)
          //  wanted exact time match only, so fail
          return NULL;
        else if(direction == ESDIF_SEARCH_FORWARDS)
        {
          //  wanted frame *after* requested time, so try this frame
          if(m = SDIFbuf_GetMatrixInFrame(cur, matrixType))
          {
            //  return this frame
            *timeFound = cur->header.time;
            return m;
          }
          else
            //  keep looking
            cur = next;
        }
        else
          //  wanted frame *before* requested time, so need to backtrack
          return SDIFbuf_GetPrevMatrix(cur, matrixType, timeFound);
      }
      else
        //  we haven't gotten to requested time yet -- keep looking
        cur = next;
    }
    
    else
    {
      //  we are traversing backwards
      if(cur->header.time < time)
      {
        //  current frame is past requested time (too early) -- we're done searching
        if(direction == ESDIF_SEARCH_EXACT)
          //  wanted exact time match only, so fail
          return NULL;
        else if(direction == ESDIF_SEARCH_BACKWARDS)
        {
          //  wanted frame *before* requested time, so try this frame
          if(m = SDIFbuf_GetMatrixInFrame(cur, matrixType))
          {
            //  return this frame
            *timeFound = cur->header.time;
            return m;
          }
          else
            //  keep looking
            cur = prev;
        }
        else
          //  wanted frame *after* requested time, so need to backtrack
          return SDIFbuf_GetNextMatrix(cur, matrixType, timeFound);
      }
      else
        //  we haven't gotten to requested time yet -- keep looking
        cur = prev;
    }
  }
  
  //  we traversed to end (or beginning) of list without passing the requested time
  
  if(forwards && (direction == ESDIF_SEARCH_BACKWARDS))
  {
    //  traversed forwards, but wanted frame immediately *before* requested time, so backtrack
    if(prev)
      return SDIFbuf_GetPrevMatrix(prev, matrixType, timeFound);
    else
      //  can't backtrack - this is the earliest frame
      return NULL;
  }
  
  if(!forwards && (direction == ESDIF_SEARCH_FORWARDS))
  {
    //  traversed backwards, but wanted frame immediately *after* requested time, so backtrack
    if(next)
      return SDIFbuf_GetNextMatrix(next->prev, matrixType, timeFound);
    else
      //  can't backtrack - this is the latest frame
      return NULL;
  }

  //  fail
  return NULL;
}


SDIFresult SDIFbuf_GetValueInFrame(SDIFmem_Frame f,
                                   const char *matrixType,
                                   sdif_int32 column,
                                   sdif_int32 row,
                                   SDIFactionOnNaN actionNaN,
                                   sdif_float64 *valueFound
                                   )
{
  SDIFmem_Matrix m;
  
  if(!(m = SDIFbuf_GetMatrixInFrame(f, matrixType)))
    return ESDIF_NOT_AVAILABLE;
  
  return SDIFbuf_GetValueInMatrix(m, column, row, actionNaN, valueFound);
}


SDIFresult SDIFbuf_GetValueInMatrix(SDIFmem_Matrix m,
                                    sdif_int32 column,
                                    sdif_int32 row,
                                    SDIFactionOnNaN actionNaN,
                                    sdif_float64 *valueFound
                                    )
{
  void *p = (char *)m->data + 
            (((row * m->header.columnCount) + column) * 
             SDIF_GetMatrixDataTypeSize(m->header.matrixDataType));
  
  switch(m->header.matrixDataType)
  {
    case SDIF_FLOAT32:
      *valueFound = *(sdif_float32 *)p;
      break;
    case SDIF_FLOAT64:
      *valueFound = *(sdif_float64 *)p;
      break;
  }
  
  //  finish if found cell with a real value (not a NaN)
  if(isfinite(*valueFound))
    return ESDIF_SUCCESS;

  //  otherwise, take requested action on NaN data
  switch(actionNaN)
  {
    case ESDIF_NAN_ACTION_ACCEPT:
      return ESDIF_SUCCESS;
    case ESDIF_NAN_ACTION_KEEP_LOOKING:
      return ESDIF_NOT_AVAILABLE;
    case ESDIF_NAN_ACTION_SET_ZERO:
      *valueFound = 0;
      return ESDIF_SUCCESS;
    case ESDIF_NAN_ACTION_FAIL:
      return ESDIF_FAIL_ON_NAN;
  }

  return ESDIF_BAD_PARAM;
}


SDIFresult SDIFbuf_GetPrevValue(SDIFmem_Frame f, 
                                const char* matrixType,
                                sdif_int32 column,
                                sdif_int32 row,
                                sdif_float64 time,
                                SDIFactionOnNaN actionNaN,
                                sdif_float64 *valueFound,
                                sdif_float64 *timeFound
                                )
{
  SDIFmem_Frame cf;
  SDIFresult r;

  //  loop backwards through frame list
  for(cf = SDIFbuf_GetFrameNearby(f, time, ESDIF_SEARCH_BACKWARDS);
      cf; 
      cf = SDIFbuf_GetPrevFrame(cf))
  {
    if(time <= cf->header.time)
      //  not early enough... keep looking
      continue;
    
    r = SDIFbuf_GetValueInFrame(cf,
                                matrixType,
                                column,
                                row,
                                actionNaN,
                                valueFound
                                );
    if(r == ESDIF_SUCCESS)
    {
      //  found requested value
      *timeFound = cf->header.time;
      return ESDIF_SUCCESS;
    }
    else if(r == ESDIF_NOT_AVAILABLE)
      //  keep looking
      continue;
    
    //  if we get here, something went wrong... possibly ESDIF_FAILING_ON_NAN
    return r;
  }
  
  //  traversed to beginning of frame list without finding requested value
  return ESDIF_NOT_AVAILABLE;
}


SDIFresult SDIFbuf_GetNextValue(SDIFmem_Frame f, 
                                const char* matrixType,
                                sdif_int32 column,
                                sdif_int32 row,
                                sdif_float64 time,
                                SDIFactionOnNaN actionNaN,
                                sdif_float64 *valueFound,
                                sdif_float64 *timeFound
                                )
{
  SDIFmem_Frame cf;
  SDIFresult r;

  //  loop forwards through frame list
  for(cf = SDIFbuf_GetFrameNearby(f, time, ESDIF_SEARCH_FORWARDS);
      cf; 
      cf = SDIFbuf_GetNextFrame(cf))
  {
    if(time >= cf->header.time)
      //  not late enough... keep looking
      continue;
    
    r = SDIFbuf_GetValueInFrame(cf,
                                matrixType,
                                column,
                                row,
                                actionNaN,
                                valueFound
                                );
    if(r == ESDIF_SUCCESS)
    {
      //  found requested value
      *timeFound = cf->header.time;
      return ESDIF_SUCCESS;
    }
    else if(r == ESDIF_NOT_AVAILABLE)
      //  keep looking
      continue;
    
    //  if we get here, something went wrong... possibly ESDIF_FAILING_ON_NAN
    return r;
  }
  
  //  traversed to beginning of frame list without finding requested value
  return ESDIF_NOT_AVAILABLE;
}


SDIFresult SDIFbuf_GetValueNearby(SDIFmem_Frame f, 
                                  const char* matrixType,
                                  sdif_int32 column,
                                  sdif_int32 row,
                                  sdif_float64 time,
                                  SDIFsearchMode direction,
                                  SDIFactionOnNaN actionNaN,
                                  sdif_float64 *valueFound,
                                  sdif_float64 *timeFound
                                  )
{
  SDIFmem_Frame cur, prev, next;
  SDIFmem_Matrix m;
  Boolean forwards;
  SDIFresult r;
  
  prev = next = NULL;

  //  in which direction should we traverse? (list is sorted by increasing time)
  forwards = (time > f->header.time);

  //  loop to traverse list
  //  (optimized with the assumption that we will be searching through a long list,
  //  where most frames do contain the requested matrix cell -- i.e. we don't want to
  //  bother searching for an actual matrix type match until we get to the right time;
  //  we would rather do the small extra work of backtracking by a frame if necessary)
  cur = f;
  while(cur)
  {
    prev = SDIFbuf_GetPrevFrame(cur);
    next = SDIFbuf_GetNextFrame(cur);

    if(cur->header.time == time)
    {
      //  exact match
      if((m = SDIFbuf_GetMatrixInFrame(cur, matrixType))
         && (ESDIF_SUCCESS == (r = SDIFbuf_GetValueInMatrix(m,
                                                            column,
                                                            row,
                                                            actionNaN,
                                                            valueFound))))
      {
        //  return this frame
        *timeFound = cur->header.time;
        return r;
      }
      else
        //  keep looking
        cur = forwards ? next : prev;
    }
    
    else if(forwards)
    {
      //  we are traversing forwards
      if(cur->header.time > time) 
      {
        //  current frame is past requested time (too late) -- we're done searching
        if(direction == ESDIF_SEARCH_EXACT)
          //  wanted exact time match only, so fail
          return ESDIF_NOT_AVAILABLE;
        else if(direction == ESDIF_SEARCH_FORWARDS)
        {
          //  wanted frame *after* requested time, so try this frame
          if((m = SDIFbuf_GetMatrixInFrame(cur, matrixType))
              && (ESDIF_SUCCESS == (r = SDIFbuf_GetValueInMatrix(m,
                                                                 column,
                                                                 row,
                                                                 actionNaN,
                                                                 valueFound))))
          {
            //  return this frame (valueFound was already filled in)
            *timeFound = cur->header.time;
            return r;
          }
          else
            //  keep looking
            cur = next;
        }
        else
          //  wanted frame *before* requested time, so need to backtrack
          return SDIFbuf_GetPrevValue(cur,
                                      matrixType,
                                      column,
                                      row,
                                      time,
                                      actionNaN,
                                      valueFound,
                                      timeFound);
      }
      else
        //  we haven't gotten to requested time yet -- keep looking
        cur = next;
    }
    
    else
    {
      //  we are traversing backwards
      if(cur->header.time < time) 
      {
        //  current frame is past requested time (too early) -- we're done searching
        if(direction == ESDIF_SEARCH_EXACT)
          //  wanted exact time match only, so fail
          return ESDIF_NOT_AVAILABLE;
        else if(direction == ESDIF_SEARCH_BACKWARDS)
        {
          //  wanted frame *before* requested time, so try this frame
          if((m = SDIFbuf_GetMatrixInFrame(cur, matrixType))
             && (ESDIF_SUCCESS == (r = SDIFbuf_GetValueInMatrix(m,
                                                                 column,
                                                                 row,
                                                                 actionNaN,
                                                                 valueFound))))
          {
            //  return this frame (valueFound was already filled in)
            *timeFound = cur->header.time;
            return r;
          }
          else
            //  keep looking
            cur = prev;
        }
        else
          //  wanted frame *after* requested time, so need to backtrack
          return SDIFbuf_GetNextValue(cur,
                                      matrixType,
                                      column,
                                      row,
                                      time,
                                      actionNaN,
                                      valueFound,
                                      timeFound);
      }
      else
        //  we haven't gotten to requested time yet -- keep looking
        cur = prev;
    }
  }
  
  //  we traversed to end (or beginning) of list without passing the requested time

  if(forwards && (direction == ESDIF_SEARCH_BACKWARDS))
  {
    if(prev)
      //  traversed forwards, but wanted frame immediately *before* requested time, so backtrack
      return SDIFbuf_GetPrevValue(prev,
                                  matrixType,
                                  column,
                                  row,
                                  time,
                                  actionNaN,
                                  valueFound,
                                  timeFound);
    else
      //  can't backtrack - this is the latest frame
      return NULL;
  }
  
  if(!forwards && (direction == ESDIF_SEARCH_FORWARDS))
  {
    if(next)
      //  traversed backwards, but wanted frame immediately *after* requested time, so backtrack
      return SDIFbuf_GetNextValue(next->prev,
                                  matrixType,
                                  column,
                                  row,
                                  time,
                                  actionNaN,
                                  valueFound,
                                  timeFound);
    else
      //  can't backtrack - this is the latest frame
      return NULL;
  }
  
  //  fail
  return ESDIF_NOT_AVAILABLE;
}


SDIFresult SDIFbuf_GetNeighborValues(SDIFmem_Frame nearbyFrame,
                                     const char* matrixType,
                                     sdif_int32 column,
                                     sdif_int32 row,
                                     sdif_float64 time,
                                     sdif_int32 count,
                                     SDIFactionOnNaN actionOnNaN,
                                     sdif_float64 *t,
                                     sdif_float64 *v,
                                     sdif_int32 *countOut
                                     )
{
  SDIFresult r1, r2;
  sdif_float64 t1, t2;
  sdif_int32 i;
  
  i = 0;
  
  //  get value at requested time or closest neighbor *before* requested time
  r1 = SDIFbuf_GetValueNearby(nearbyFrame,
                              matrixType,
                              column,
                              row,
                              time,
                              ESDIF_SEARCH_BACKWARDS,
                              actionOnNaN,
                              &v[0],
                              &t1
                              );
  if(r1 == ESDIF_SUCCESS)
  {
    t[i++] = t1;
    t2 = t1;
  }
  else
    t1 = t2 = time;

  if(i < count)
  {
    //  loop to alternate looking for earlier + later values 
    //  (if we run out of values on one side, keep looking on the other side)
    r2 = ESDIF_SUCCESS;
    while(TRUE)
    {
      if(r2 == ESDIF_SUCCESS)
      {
      //  get value at next available time (later)
        r2 = SDIFbuf_GetNextValue(nearbyFrame,
                                  matrixType,
                                  column,
                                  row,
                                  t2,
                                  actionOnNaN,
                                  &v[i],
                                  &t2
                                  );
        if(r2 == ESDIF_SUCCESS)
          t[i++] = t2;
        else if(r2 != ESDIF_NOT_AVAILABLE)
          return r2;
        if(i == count)
          break;
      }
      
      //  get value at previous available time (earlier)
      if(r1 == ESDIF_SUCCESS)
      {
        r1 = SDIFbuf_GetPrevValue(nearbyFrame,
                                  matrixType,
                                  column,
                                  row,
                                  t1,
                                  actionOnNaN,
                                  &v[i],
                                  &t1
                                  );
        if(r1 == ESDIF_SUCCESS)
          t[i++] = t1;
        else if(r1 != ESDIF_NOT_AVAILABLE)
          return r1;
        if(i == count)
          break;
      }
      
      if((r1 != ESDIF_SUCCESS) && (r2 != ESDIF_SUCCESS))
        //  no more values available on either side
        break;
    }
  }

  //  fill in number of neighbors actually found
  *countOut = i;
  
  return ESDIF_SUCCESS;
}


/**************************************/
/*                                    */
/* implementation of instance methods */
/*                                    */
/**************************************/

void SDIFbuf_Clear(SDIFbuf_Buffer b) 
{
  SDIFmem_Frame f, next;
  SDIFbuf_BufferFriends bp = b->internal;
  
  //  free SDIF data contained in current instance (follow linked lists)
  for(f = bp->head; f; f = next)
  {
    next = SDIFbuf_GetNextFrame(f);
    SDIFmem_FreeFrame(f);
  }
  bp->head = bp->tail = NULL;
  
  //  reset all public + private instance fields to initial state
  instance_reset(b);

  //  NOTE: this instance must be returned in a state ready to be reused
}  


/*  instance_reset()
    reset all public + private instance fields to initial state
    NOTE: assume storage for internal data already successfully allocated
    NOTE: assume head/tail pointers aren't holding sole reference to alloc storage
          (i.e. be sure to free all frames in buffer by calling SDIFbuf_Clear() first,
           which calls this procedure anyhow)
*/
static void instance_reset(SDIFbuf_Buffer b)
{
  SDIFbuf_BufferFriends bp = b->internal;
  SDIFbuf_BufferPrivate bpp = bp->internal;

  //  initialize public instance fields
  //  (NOTE: we don't unlink this instance from list by initializing prev + next here)

  //  initialize friends instance fields
  bp->head = bp->tail = NULL;
  bp->streamID = NULL;
  SDIF_Copy4Bytes(bp->frameType, "----");
  bp->min_time = 0;
  bp->max_time = 0;
  bp->debug = 0;

  //  initialize file instance fields
  bpp->dummy = 0;
}


SDIFbuf_BufferFriends SDIFbuf_GetBufferFriends(SDIFbuf_Buffer b)
{
  return (SDIFbuf_BufferFriends)(b->internal);
}


SDIFresult SDIFbuf_ReadStream(SDIFbuf_Buffer b, 
                              char *filename, 
                              SDIFwhichStreamMode mode, 
                              sdif_int32 arg
                              ) 
{
  FILE *f;
  SDIFresult r;
  
  if(!(r = SDIF_OpenRead(filename, &f)))
    return r;
  
  return SDIFbuf_ReadStreamFromOpenFile(b, f, mode, arg);
}


SDIFresult SDIFbuf_ReadStreamFromOpenFile(SDIFbuf_Buffer b, 
					  FILE *f, 
					  SDIFwhichStreamMode mode, 
					  sdif_int32 arg
					  ) 
{  
  SDIF_FrameHeader fh;
  SDIFmem_Frame previous, current, first;
  SDIFresult r;
  sdif_int32 streamID;
  
  SDIFbuf_BufferFriends bp = b->internal;

  //  deal with stream select mode
  if (mode == ESDIF_WHICH_STREAM_NUMBER) 
    streamID = arg;
  else
    //  unrecognized mode
    return ESDIF_BAD_PARAM;
  
  //  if there already was data in current buffer instance, free it
  SDIFbuf_Clear(b);

  first = current = previous = NULL;

  //  loop to read entire file
  while((r = SDIF_ReadFrameHeader(&fh, f)) == ESDIF_SUCCESS) {
  
    if (fh.streamID == streamID)  {
      //  we want this frame
      if (r = SDIFmem_ReadFrameContents(&fh, f, &current)) 
      {
        SDIF_CloseRead(f);
        return ESDIF_READ_FAILED;
      }
      
      current->prev = previous;
      current->next = NULL;
      
      if(first == NULL)
        first = current;
      else
        previous->next = current;
      previous = current;
    } else {      
    //  skip this frame
      if(r = SDIF_SkipFrame(&fh, f)) {
        SDIF_CloseRead(f);
        return ESDIF_READ_FAILED;
      }
    }
  }

  //  make sure we ended with normal EOF
  if (r != ESDIF_END_OF_DATA) 
  {
    SDIF_CloseRead(f);
    return ESDIF_READ_FAILED;
  }

  //  close file  
  r = SDIF_CloseRead(f);

  bp->head = first;
  bp->tail = current;
  bp->streamID = first->header.streamID;
  SDIF_Copy4Bytes(bp->frameType, first->header.frameType);
  bp->min_time = first->header.time;
  bp->max_time = current->header.time;

  return r;
}


SDIFmem_Frame SDIFbuf_GetFirstFrame(SDIFbuf_Buffer b)
{
  SDIFbuf_BufferFriends bp = b->internal;
  return bp->head;
}


SDIFmem_Frame SDIFbuf_GetLastFrame(SDIFbuf_Buffer b)
{
  SDIFbuf_BufferFriends bp = b->internal;
  return bp->tail;
}


SDIFmem_Frame SDIFbuf_GetFrame(SDIFbuf_Buffer b, sdif_float64 time, SDIFsearchMode direction)
{
 	SDIFmem_Frame f;
	
	if(!(f = SDIFbuf_GetFirstFrame(b)))
	  return NULL; 	      //  no frames in buffer
	
  return SDIFbuf_GetFrameNearby(f, time, direction);
}


SDIFresult SDIFbuf_InsertFrame(SDIFbuf_Buffer b, SDIFmem_Frame newf, SDIFinsertMode replace)
{
  SDIFmem_Frame f;
  SDIFmem_Frame next;
  SDIFmem_Frame prev;
  
  SDIFbuf_BufferFriends bp = b->internal;
  
  //  case 1: adding first frame to empty buffer

  if(!bp->head)
  {
    newf->prev = newf->next = NULL;
    bp->head = bp->tail = newf;
    bp->min_time = bp->max_time = newf->header.time;
    return ESDIF_SUCCESS;
  }

  //  find frame at this exact time, or closest preceding frame (if any)
  f = SDIFbuf_GetFrame(b, newf->header.time, ESDIF_SEARCH_BACKWARDS);

  if(f)
  {
    prev = SDIFbuf_GetPrevFrame(f);
    next = SDIFbuf_GetNextFrame(f);
  }
  else
  {
    prev = NULL;
    next = bp->head;
  }

  //  case 2: replace a frame (or report error)

  if(f)
    if(newf->header.time == f->header.time)
    {
      if(replace == ESDIF_INSERT_REPLACE)
      {
        if(f == newf)
          return ESDIF_SUCCESS;   //  don't try to replace a frame with itself!
        else
        {
          //  free old frame
          SDIFmem_FreeFrame(f);
          
          if(prev)
            prev->next = newf;
          else
            bp->head = newf;  //  special case: replacing first frame

          newf->next = next;
          newf->prev = prev;

          if(next)
            next->prev = newf;
          else
            bp->tail = newf;  //  special case: replacing last frame
        
        return ESDIF_SUCCESS;
        }
      }
      else
        return ESDIF_FRAME_ALREADY_EXISTS;
    }

  //  case 3: insert a frame

  if(f)
    f->next = newf;
  else
  {
    //  special case: inserting before first frame
    bp->head = newf;
    bp->min_time = newf->header.time;
  }
  
  newf->prev = f;
  newf->next = next;
  
  if(next)
    next->prev = newf;
  else
  {
    //  special case: inserting after last frame
    bp->tail = newf;
    bp->max_time = newf->header.time;
  }

  return ESDIF_SUCCESS;
}


SDIFmem_Matrix SDIFbuf_GetMatrix(SDIFbuf_Buffer b, 
                                const char *matrixType, 
                                sdif_float64 time, 
                                SDIFsearchMode direction,
                                sdif_float64 *timeFound
                                )
{
  SDIFmem_Frame f;
  
  if(!(f = SDIFbuf_GetFirstFrame(b)))
    return NULL;
  
  return SDIFbuf_GetMatrixNearby(f, matrixType, time, direction, timeFound);
}


SDIFresult SDIFbuf_GetValue(SDIFbuf_Buffer b,
                            const char* matrixType,
                            sdif_int32 column,
                            sdif_int32 row,
                            sdif_float64 time,
                            SDIFsearchMode direction,
                            SDIFactionOnNaN actionNaN,
                            sdif_float64 *valueFound,
                            sdif_float64 *timeFound
                            )
{
  SDIFmem_Frame f;
  
  if(!(f = SDIFbuf_GetFirstFrame(b)))
    return ESDIF_NOT_AVAILABLE;
  
  return SDIFbuf_GetValueNearby(f, 
                                matrixType, 
                                column, 
                                row,
                                time, 
                                direction, 
                                actionNaN,
                                valueFound,
                                timeFound);
}


/* SDIFbuf_GetMinTime (get timestamp of earliest frame in an SDIFbuf_Buffer instance)
*/
SDIFresult SDIFbuf_GetMinTime(SDIFbuf_Buffer b, sdif_float64 *time)
{
  SDIFmem_Frame f;
  
  if(!(f = SDIFbuf_GetFirstFrame(b)))
    return ESDIF_NOT_AVAILABLE;
  
  *time = f->header.time;
  
  return ESDIF_SUCCESS;
}


/* SDIFbuf_GetMaxTime (get timestamp of latest frame in an SDIFbuf_Buffer instance)
*/
SDIFresult SDIFbuf_GetMaxTime(SDIFbuf_Buffer b, sdif_float64 *time)
{
  SDIFmem_Frame f;
  
  if(!(f = SDIFbuf_GetLastFrame(b)))
    return ESDIF_NOT_AVAILABLE;
  
  *time = f->header.time;
  
  return ESDIF_SUCCESS;
}


/* SDIFbuf_GetMinMatrixTime 
   (get timestamp of earliest frame in an SDIFbuf_Buffer instance containing the specified matrix)
*/
SDIFresult SDIFbuf_GetMinMatrixTime(SDIFbuf_Buffer b, const char *matrixType, sdif_float64 *time)
{
  SDIFmem_Frame f;
  SDIFmem_Matrix m;
  sdif_float64 t;
  
  //  get first frame in buffer
  if(!(f = SDIFbuf_GetFirstFrame(b)))
    return ESDIF_NOT_AVAILABLE;
  t = f->header.time;

  //  scan forwards until we find first instance of the requested matrix
  if(!(m = SDIFbuf_GetMatrixNearby(f, matrixType, t, ESDIF_SEARCH_FORWARDS, time)))
    return ESDIF_NOT_AVAILABLE;

  return ESDIF_SUCCESS;
}


/* SDIFbuf_GetMaxMatrixTime 
   (get timestamp of latest frame in an SDIFbuf_Buffer instance containing the specified matrix)
*/
SDIFresult SDIFbuf_GetMaxMatrixTime(SDIFbuf_Buffer b, const char *matrixType, sdif_float64 *time)
{
  SDIFmem_Frame f;
  SDIFmem_Matrix m;
  sdif_float64 t;
  
  //  get first frame in buffer
  if(!(f = SDIFbuf_GetLastFrame(b)))
    return ESDIF_NOT_AVAILABLE;
  t = f->header.time;

  //  scan backwards until we find closest instance of the requested matrix
  if(!(m = SDIFbuf_GetMatrixNearby(f, matrixType, t, ESDIF_SEARCH_BACKWARDS, time)))
    return ESDIF_NOT_AVAILABLE;

  return ESDIF_SUCCESS;
}


SDIFresult SDIFbuf_TimeShiftToZero(SDIFbuf_Buffer b)
{
  SDIFresult r;
  SDIFmem_Frame f;
  sdif_float64 tMin;

  if(!(f = SDIFbuf_GetFirstFrame(b)))
    //  no frames in buffer -- we're done
    return ESDIF_SUCCESS;

  tMin = f->header.time;

  //  this loop couldn't be any less thread safe  
  while(f)
  {
    f->header.time -= tMin;
    f = SDIFbuf_GetNextFrame(f);
  }
    
  return NULL;
}


SDIFresult SDIFbuf_GetMaxNumColumns(SDIFbuf_Buffer b, const char *matrixType, sdif_int32 *result) {
  SDIFmem_Frame f;
  SDIFmem_Matrix m;
  sdif_int32 r, c;

  if(!(f = SDIFbuf_GetFirstFrame(b)))
    //  buffer is empty
    return ESDIF_END_OF_DATA;

   r = -1;  // Matrix type not yet found

   while(f) {
    for (m = f->matrices; m != NULL; m = m->next) {
		if (SDIF_Char4Eq(matrixType, m->header.matrixType)) {
		    c = m->header.columnCount;
		    if (c > r) r = c;
		}
    } 
    f = SDIFbuf_GetNextFrame(f);
   }  
   
   if (r == -1) return ESDIF_BAD_MATRIX_DATA_TYPE;
   
	*result = r;
	return  ESDIF_SUCCESS;  
}

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))


SDIFresult SDIFbuf_GetColumnRanges(SDIFbuf_Buffer b, const char *matrixType, sdif_int32 ncols, 
								   sdif_float64 *column_mins, sdif_float64 *column_maxes) {
	SDIFmem_Frame f;
	SDIFmem_Matrix m;
	sdif_int32 r, c;
	Boolean seenAnyYet = FALSE;
	sdif_float64 cellValue;
	SDIFresult result;

	if(!(f = SDIFbuf_GetFirstFrame(b))) {
		//  buffer is empty
		return ESDIF_END_OF_DATA;
	}

	for (c = 0; c < ncols; ++c) {
		column_mins[c] = DBL_MAX;
		column_maxes[c] = DBL_MIN;
	}

	while (f) {
		for (m = f->matrices; m != NULL; m = m->next) {
			if (SDIF_Char4Eq(matrixType, m->header.matrixType)) {
			   	seenAnyYet = TRUE;
			    for (r = 0; r < m->header.rowCount; ++r) {
			    	for (c = 0; c < m->header.columnCount && c < ncols; ++c) {
			    		result = SDIFbuf_GetValueInMatrix(m, c, r, ESDIF_NAN_ACTION_FAIL, &cellValue);
			    		if (result == ESDIF_SUCCESS) {
			    			column_mins[c] = MIN(column_mins[c], cellValue);
			    			column_maxes[c] = MAX(column_maxes[c], cellValue);
			    		}
			    	}
			    }
			}
		} 
		f = SDIFbuf_GetNextFrame(f);
	}

	if (!seenAnyYet) return ESDIF_BAD_MATRIX_DATA_TYPE;
	return  ESDIF_SUCCESS;  
}

