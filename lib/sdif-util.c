/*
Copyright (c) 2004.  The Regents of the University of California (Regents).
All Rights Reserved.

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

/*
  sdif-utils.c
  
  Implementation of common generic SDIF utilities
  (see sdif-utils.h for feature list, documentation of public interface)

  author: Ben "Jacobs"
  2004/03/31 (0.1.0) imported code from sdif unit test kit - bj
  
  NOTES
  
  - currently supports sdif_float32 and sdif_float64 matrix data only

*/


#define SDIF_UTILS_VERSION "0.1.0"


#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "sdif.h"
#include "sdif-mem.h"
#include "sdif-util.h"


/**************************/
/*                        */
/* method implementations */
/*                        */
/**************************/


SDIFmem_Matrix SDIFutil_CreateMatrix(sdif_int32 columns, 
                                     sdif_int32 rows, 
                                     SDIF_MatrixDataType dataType,
                                     const char *matrixType
                                     )
{
  int i, j;
  SDIFmem_Matrix m;
  
  if(!(m = SDIFmem_CreateEmptyMatrix()))
    return NULL;
  
  if(!(m->data = SDIFmem_Alloc(columns * rows * SDIF_GetMatrixDataTypeSize(dataType))))
  {
    SDIFmem_FreeMatrix(m);
    return NULL;
  }

  m->header.matrixDataType = dataType;
  m->header.columnCount = columns;
  m->header.rowCount = rows;
  SDIF_Copy4Bytes(m->header.matrixType, matrixType);
  
  for(i = 0; i < rows; i++)
    for(j = 0; j < columns; j++)
      SDIFutil_SetMatrixCell(m, j, i, 0);
    
  return m;
}


SDIFresult SDIFutil_CloneMatrix(SDIFmem_Matrix m, SDIFmem_Matrix *mnew)
{
  if(!(*mnew = SDIFutil_CreateMatrix(m->header.columnCount,
                                     m->header.rowCount,
                                     m->header.matrixDataType,
                                     m->header.matrixType
                                     )))
    return ESDIF_OUT_OF_MEMORY;
    
  return SDIFutil_CopyMatrix(*mnew, m);
}


SDIFresult SDIFutil_CopyMatrix(SDIFmem_Matrix dstMatrix, SDIFmem_Matrix srcMatrix)
{
  int i, j;
  
  for(i = 0; i < srcMatrix->header.rowCount; i++)
    for(j = 0; j < srcMatrix->header.columnCount; j++)
      SDIFutil_SetMatrixCell(dstMatrix, j, i, SDIFutil_GetMatrixCell(srcMatrix, j, i));
  
  return ESDIF_SUCCESS;
}


Boolean SDIFutil_MatrixEqual(SDIFmem_Matrix m1, SDIFmem_Matrix m2)
{
  int i, j;
  
  if(!m1 && !m2)
    return TRUE;
  if(!m1 || !m2)
    return FALSE;
  
  for(i = 0; i < m1->header.rowCount; i++)
    for(j = 0; j < m1->header.columnCount; j++)
    {
      sdif_float64 v1 = SDIFutil_GetMatrixCell(m1, j, i);
      sdif_float64 v2 = SDIFutil_GetMatrixCell(m2, j, i);

      if(isnan(v1))
      {
        if(!isnan(v2))
          return FALSE;
      }
      else if(v1 != v2)
        return FALSE;
    }

  return TRUE;
}


void SDIFutil_LoadMatrixFloat32(SDIFmem_Matrix m, sdif_float32 *list)
{
  sdif_float32 *pi = list;
  int i, j;

  for(i = 0 ; i < m->header.rowCount; i++)
    for(j = 0; j < m->header.columnCount; j++)
      SDIFutil_SetMatrixCell(m, j, i, *pi++);
}


void SDIFutil_LoadMatrixFloat64(SDIFmem_Matrix m, sdif_float64 *list)
{
  sdif_float64 *pi = list;
  int i, j;

  for(i = 0 ; i < m->header.rowCount; i++)
    for(j = 0; j < m->header.columnCount; j++)
      SDIFutil_SetMatrixCell(m, j, i, *pi++);
}


sdif_float64 SDIFutil_GetMatrixCell(SDIFmem_Matrix m,
                                    sdif_int32 column,
                                    sdif_int32 row
                                    )
{
  sdif_float64 v;
  
  void *p = (char *)m->data + 
            (((row * m->header.columnCount) + column) * 
             SDIF_GetMatrixDataTypeSize(m->header.matrixDataType));
  
  switch(m->header.matrixDataType)
  {
    case SDIF_FLOAT32:
      v = *(sdif_float32 *)p;
      break;
    case SDIF_FLOAT64:
      v = *(sdif_float64 *)p;
      break;
  }
  
  return v;
}


void SDIFutil_SetMatrixCell(SDIFmem_Matrix m, 
                            sdif_int32 column, 
                            sdif_int32 row, 
                            sdif_float64 value
                            )
{
  void *p = (char *)m->data + 
            (((row * m->header.columnCount) + column) * 
             SDIF_GetMatrixDataTypeSize(m->header.matrixDataType));
  
  switch(m->header.matrixDataType)
  {
    case SDIF_FLOAT32:
      *(sdif_float32 *)p = value;
      break;
    case SDIF_FLOAT64:
      *(sdif_float64 *)p = value;
      break;
  }
}


