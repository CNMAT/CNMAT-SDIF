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
  sdif-interp-implem.c
  
  Implementation of interpolation algorithms (plugin code for use with sdif-interp.c)
  (see sdif-interp.h for feature list, documentation of public interface)

  author: Ben "Jacobs"
  2004/06/22 (0.1.0) implementation of initial feature set - bj

*/


#define SDIF_INTERP_IMPLEM_VERSION "0.1.0"

#include <stdio.h>
#include <string.h> /* for strerror() */
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
//#include <math.h>

#include "sdif.h"
#include "sdif-mem.h"
#include "sdif-buf.h"
#include "sdif-interp.h"
#include "sdif-interp-implem.h"
#include "sdif-util.h"



/***********************************/
/*                                 */
/* a few interpolator functions... */
/*                                 */
/***********************************/

//
//  linear interpolator, including NaN support
//
SDIFresult LinearInterpolator(SDIFmem_Frame nearbyFrame,
                              const char *matrixType,
                              sdif_int32 column,
                              sdif_float64 time,
                              SDIFactionOnNaN actionOnNaN,
                              SDIFmem_Matrix matrixOut,
                              va_list args
                              )
{
  SDIFresult r;
  int rows = matrixOut->header.rowCount;
  int i;
  
  //  loop to compute value for this column in each row
  for(i = 0; i < matrixOut->header.rowCount; i++)
  {
    sdif_float64 t1, t2, v1, v2;
    //  first attempt to find value at requested time
    r = SDIFbuf_GetValueNearby(nearbyFrame,
                               matrixType,
                               column,
                               i,
                               time,
                               ESDIF_SEARCH_BACKWARDS,
                               actionOnNaN,
                               &v1,
                               &t1
                               );
    if(r == ESDIF_NOT_AVAILABLE)
    {
      SDIFutil_SetMatrixCell(matrixOut, column, i, NAN);
      continue;
    }
    if(r != ESDIF_SUCCESS)
      //  failed: couldn't find value at or before requested time
      return r;
    
    if(time == t1)
      //  exact time match found, no need to interpolate
      SDIFutil_SetMatrixCell(matrixOut, column, i, v1);
    else
    {
      //  need to interpolate, so we need a second value
      r = SDIFbuf_GetValueNearby(nearbyFrame,
                                 matrixType,
                                 column,
                                 i,
                                 time,
                                 ESDIF_SEARCH_FORWARDS,
                                 actionOnNaN,
                                 &v2,
                                 &t2
                                 );
    if(r == ESDIF_NOT_AVAILABLE)
    {
      SDIFutil_SetMatrixCell(matrixOut, column, i, NAN);
      continue;
    }
      if(r != ESDIF_SUCCESS)
        //  failed: couldn't find value after requested time
        return r;

      //  success: do linear interpolation
      SDIFutil_SetMatrixCell(matrixOut, column, i, v1 + ((time - t1) * (v2 - v1) / (t2 - t1)));
    }
    
    // post("col %d, row %d, v1 %f, v2 %f", column, i, v1, v2);
  }
  
  return ESDIF_SUCCESS;
}


//
//  Lagrange/Waring interpolator (includes NaN handling)
//  var arg #1 = requested polynomial degree
//
SDIFresult LagrangeInterpolator(SDIFmem_Frame nearbyFrame,
                                       const char *matrixType,
                                       sdif_int32 column,
                                       sdif_float64 time,
                                       SDIFactionOnNaN actionOnNaN,
                                       SDIFmem_Matrix matrixOut,
                                       va_list args
                                       )
{
#define MAX_DEGREE 100

  SDIFresult r;
  int i;
  sdif_int32 degree;

  //  read requested degree arg; check against MAX_DEGREE
  degree = va_arg(args, sdif_int32);
  if(degree < 1)
    return ESDIF_BAD_PARAM;
  if(degree > MAX_DEGREE)
    return ESDIF_BAD_PARAM;
  
  //  loop to compute value for this column in each row
  for(i = 0; i < matrixOut->header.rowCount; i++)
  {
    sdif_float64 t1, val;
    sdif_float64 t[MAX_DEGREE + 1], v[MAX_DEGREE + 1];

    //  write NaN into output matrix if we can't find at least
    //  one earlier + one later neighbor value    
    r = SDIFbuf_GetValueNearby(nearbyFrame,
                               matrixType,
                               column,
                               i,
                               time,
                               ESDIF_SEARCH_BACKWARDS,
                               actionOnNaN,
                               &val,
                               &t1
                               );
    r |= SDIFbuf_GetValueNearby(nearbyFrame,
                                matrixType,
                                column,
                                i,
                                time,
                                ESDIF_SEARCH_FORWARDS,
                                actionOnNaN,
                                &val,
                                &t1
                                );
    if(r == ESDIF_NOT_AVAILABLE)
    {
      SDIFutil_SetMatrixCell(matrixOut, column, i, NAN);
      continue;
    }

    //  if no exact time match, interpolate from a full set of neighboring values
    if(t1 != time)
    {
      int j, k;
      sdif_int32 n;

      //  get the neighboring values
      r = SDIFbuf_GetNeighborValues(nearbyFrame,
                                    matrixType,
                                    column,
                                    i,
                                    time,
                                    degree + 1,
                                    actionOnNaN,
                                    t,
                                    v,
                                    &n);
      
      if(n < 2)
        //  fail: didn't even find 2 neighboring values
        return ESDIF_NOT_AVAILABLE;
      
      //  do the interpolation (Lagrange/Waring method)
      val = 0;
      for(j = 0; j < n; j++)
      {
        sdif_float64 vp = 1.0;
        for(k = 0; k < n; k++)
        {
          if(k == j)
            continue;
          vp *= (time - t[k]) / (t[j] - t[k]);
        }
        val += v[j] * vp;
      }
    }

    //  store the result
    SDIFutil_SetMatrixCell(matrixOut, column, i, val);
  }
  
  return ESDIF_SUCCESS;
}



