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

/* sdif-utils.h

  Public interface to common generic SDIF utilities:
  - matrix operations
  
  Future features:
  - support for additional matrix data types

  author: Ben "Jacobs"
  2004/03/31 (0.1.0) imported code from sdif unit test kit - bj


  NOTES
  
  - currently supports sdif_float32 and sdif_float64 matrix data only

  - this SDIF library is neither reentrant nor thread-safe.

  - assume user has already included "sdif.h" and "sdif-mem.h"
*/


/**********************************/
/*                                */
/* public types, enums, constants */
/*                                */
/**********************************/


/*******************************/
/*                             */
/* public interface to methods */
/*                             */
/*******************************/

SDIFmem_Matrix SDIFutil_CreateMatrix(sdif_int32 columns, 
                                     sdif_int32 rows, 
                                     SDIF_MatrixDataType dataType,
                                     const char *matrixType
                                     );

SDIFresult SDIFutil_CloneMatrix(SDIFmem_Matrix m, SDIFmem_Matrix *mnew);

SDIFresult SDIFutil_CopyMatrix(SDIFmem_Matrix dstMatrix, SDIFmem_Matrix srcMatrix);

Boolean SDIFutil_MatrixEqual(SDIFmem_Matrix m1, SDIFmem_Matrix m2);

void SDIFutil_LoadMatrixFloat32(SDIFmem_Matrix m, sdif_float32 *list);

void SDIFutil_LoadMatrixFloat64(SDIFmem_Matrix m, sdif_float64 *list);

void *SDIFutil_GetMatrixElement(SDIFmem_Matrix m, sdif_int32 column, sdif_int32 row);


/* GetMatrixCell procedures return cheesy -99999 value instead of trying their best
   on type conversion.  */
sdif_float64 SDIFutil_GetMatrixCell(SDIFmem_Matrix m,
                                    sdif_int32 column,
                                    sdif_int32 row
                                    );

sdif_int32 SDIFutil_GetMatrixCell_int32(SDIFmem_Matrix m,
                                   	 	sdif_int32 column,
                                   		sdif_int32 row
                                   		);



void SDIFutil_SetMatrixCell(SDIFmem_Matrix m, 
                            sdif_int32 column, 
                            sdif_int32 row, 
                            sdif_float64 value
                            );


