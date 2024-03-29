/*
Copyright (c) 1999.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Sami Khoury and Matt Wright, The Center for New Music and Audio
Technologies, University of California, Berkeley.

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
  sdif-mem.c
  Implementation of routines for representing SDIF data in memory

  by Sami Khoury and Matt Wright
  Split from sdif.c 9/22/99 by Matt Wright
  Updated 10/13/99 by Matt for new SDIF library
  12/16/99 Version 1.1: Bug fixes from Maarten de Boer    
*/

#include <stdio.h>
#include <string.h> /* for strerror() */
#include <errno.h>
#include "sdif.h"
#include "sdif-mem.h"

#ifdef REALASSERT
#include <assert.h>
#else
#define assert(x) /* Do nothing */
#endif



/* The user-supplied malloc/free procedures.  Also used in sdif-types.c */
void *(*my_malloc)(int numBytes) = NULL;
void (*my_free)(void *memory, int numBytes) = NULL;

static unsigned char SDIFmem_InitializedFlag = 0;


SDIFresult SDIFmem_Init(void *(*MemoryAllocator)(int numBytes), 
			void (*MemoryFreer)(void *memory, int numBytes)) {
	my_malloc = MemoryAllocator;
	my_free = MemoryFreer;
	
	SDIFmem_InitializedFlag = 1;

	return ESDIF_SUCCESS;
}


SDIFresult SDIFmem_Initialized(void)
{
  if(!SDIFmem_InitializedFlag)
    return ESDIF_NOT_INITIALIZED;
  
  return ESDIF_SUCCESS;
}


void *SDIFmem_Alloc(int numBytes) {
  return (*my_malloc)(numBytes);
}


void SDIFmem_Free(void *memory, int numBytes) {
  (*my_free)(memory, numBytes);
}


SDIFmem_Frame SDIFmem_CreateEmptyFrame(void) {

    SDIFmem_Frame f;

    f = (*my_malloc)(sizeof(*f));

    if (f == NULL) {
    	return NULL;
    }

    f->header.frameType[0] = '\0';
    f->header.frameType[1] = '\0';
    f->header.frameType[2] = '\0';
    f->header.frameType[3] = '\0';
    f->header.size = 16;
    f->header.time = 0;
    f->header.streamID = 0;
    f->header.matrixCount = 0;
    f->matrices = NULL;

    f->prev = NULL;
    f->next = NULL;
    return f;
}


void SDIFmem_FreeFrame(SDIFmem_Frame f) {
	SDIFmem_Matrix m, next;

	if (f == 0) return;
	
	m = f->matrices;
	while (m != NULL) {
		next = m->next;
		SDIFmem_FreeMatrix(m);
		m = next;
	}
	
	(*my_free)(f, sizeof(*f));
}	


SDIFmem_Matrix SDIFmem_CreateEmptyMatrix(void) {

    SDIFmem_Matrix result;

	result = (*my_malloc)(sizeof(*result));

    if (result == NULL) {
    	return NULL;
    }
    
    SDIF_Copy4Bytes(result->header.matrixType, "\0\0\0\0");
    result->header.matrixDataType = SDIF_NO_TYPE;
    result->header.rowCount = result->header.columnCount = 0;
    result->data = 0;
    
    return result;
}


void SDIFmem_FreeMatrix(SDIFmem_Matrix m) {
	if (m->data != 0) {
		(*my_free)(m->data, SDIF_GetMatrixDataSize(&(m->header)));
	}
	(*my_free)(m, sizeof(*m));
}


void SDIFmem_RepairFrameHeader(SDIFmem_Frame f) {
    sdif_int32 numBytes;
    sdif_int32 numMatrices;

    SDIFmem_Matrix m;

    /* The rest of the frame header: */
    numBytes = sizeof(SDIF_FrameHeader) - 8;
    numMatrices = 0;

    for (m = f->matrices; m != 0; m=m->next) {
	++numMatrices;
	numBytes += sizeof(SDIF_MatrixHeader);
	numBytes += SDIF_GetMatrixDataSize(&(m->header));
    }

    f->header.size = numBytes;
    f->header.matrixCount = numMatrices;
}
    

SDIFresult SDIFmem_ReadFrameContents(SDIF_FrameHeader *head, FILE *f,
                     SDIFmem_Frame *putithere) {

    /* The user has just read the header for this frame; now we have to read
       all the frame's matrices, put them in an SDIFmem_Matrix linked list,
       and then stuff everything into an SDIFmem_Frame. */
    SDIFresult r;
    SDIFmem_Frame result;
    SDIFmem_Matrix matrix;
    int i, sz;
    SDIFmem_Matrix *prevNextPtr;

    result = (SDIFmem_Frame) (*my_malloc)(sizeof(*result));
    
    if (result == 0) {
        return ESDIF_OUT_OF_MEMORY;
    }
    
    result->header = *head;
    result->prev = 0;
    result->next = 0;
    result->matrices = 0;
    
    prevNextPtr = &(result->matrices);

    for (i = 0; i < head->matrixCount; ++i) {
        matrix = SDIFmem_CreateEmptyMatrix();
        if (matrix == 0) {
	    SDIFmem_FreeFrame(result);
            return ESDIF_OUT_OF_MEMORY;
        }
        
        /* Get the linked list pointers right.  Now if we bomb out
           and call SDIFmem_FreeFrame(result) it will free the matrix
           we just allocated */
        *(prevNextPtr) = matrix;
        prevNextPtr = &(matrix->next);
        matrix->next = 0;


        if (r = SDIF_ReadMatrixHeader(&(matrix->header), f)) {
            SDIFmem_FreeFrame(result);
            return r;
        }

        sz = SDIF_GetMatrixDataSize(&(matrix->header));

	/* Note: SDIF_GetMatrixDataSize includes padding bytes. */

	if (sz == 0) {
	    matrix->data = 0;
	} else {
	    matrix->data = (*my_malloc)(sz);
	    if (matrix->data == 0) {
		SDIFmem_FreeFrame(result);
		return ESDIF_OUT_OF_MEMORY;
	    }
	}
        
        if (r = SDIF_ReadMatrixData(matrix->data, f, &(matrix->header))) {
	    SDIFmem_FreeFrame(result);
            return r;
        }

	/* Note: SDIF_ReadMatrixData advanced the file pointer past any 
	   padding bytes. */
    }

    *putithere = result;
    return ESDIF_SUCCESS;
}


SDIFresult SDIFmem_ReadFrame(FILE *f, SDIFmem_Frame *putithere) {
    SDIFresult r;
    SDIF_FrameHeader fh;

    if (r = SDIF_ReadFrameHeader(&fh, f)) {
	return r;
    }

    return SDIFmem_ReadFrameContents(&fh, f, putithere);
}


SDIFresult SDIFmem_AddMatrix(SDIFmem_Frame f, SDIFmem_Matrix m) {
    int sz;
    SDIFmem_Matrix p, last;
	
    assert(f != NULL);
    assert(m != NULL);

    m->next = NULL;

    p = f->matrices;
    if (p == NULL) {
        f->matrices = m;
    } else {
        while (p != NULL) {
	    if (SDIF_Char4Eq(p->header.matrixType, m->header.matrixType)) {
		return ESDIF_DUPLICATE_MATRIX_TYPE_IN_FRAME;
	    }
	    last = p;
	    p = p->next;
    }
        last->next = m;
    }

    sz = SDIF_GetMatrixDataSize(&(m->header));

    f->header.size += sz + sizeof(SDIF_MatrixHeader);
    f->header.matrixCount++;
    return ESDIF_SUCCESS;
}

SDIFresult SDIFmem_RemoveMatrix(SDIFmem_Frame f, SDIFmem_Matrix m) {
	SDIFmem_Matrix p;
	
	if (f->matrices == m) {
		/* m is the first in f's linked list of matrices */
		f->matrices = m->next;
		SDIFmem_FreeMatrix(m);
		return ESDIF_SUCCESS;
	}

	/* Search the rest of the linked list */
	for (p=f->matrices; p->next != NULL; p=p->next) {
		if (p->next == m) {
			p->next = m->next;
			SDIFmem_FreeMatrix(m);
			return ESDIF_SUCCESS;
		}
	}
	
	/* Didn't find it */
#define 	ESDIF_MATRIX_NOT_FOUND -99;
	return ESDIF_MATRIX_NOT_FOUND;
}


SDIFresult SDIFmem_WriteMatrix(FILE *sdif_handle, SDIFmem_Matrix m) {
    SDIFresult r;
    sdif_int32 sz, numElements;
    int paddingNeeded;

    if (r = SDIF_WriteMatrixHeader(&(m->header), sdif_handle)) {
	return r;
    }

    sz = SDIF_GetMatrixDataTypeSize(m->header.matrixDataType);
    numElements = (m->header.rowCount * m->header.columnCount);

    switch (sz) {
	case 1:
	    r = SDIF_Write1(m->data, numElements, sdif_handle);
	    break;
	case 2:
	    r = SDIF_Write2(m->data, numElements, sdif_handle);
	    break;
	case 4:
	    r = SDIF_Write4(m->data, numElements, sdif_handle);
	    break;
	case 8:
	    r = SDIF_Write8(m->data, numElements, sdif_handle);
	    break;
	    
	default:
	    return ESDIF_BAD_MATRIX_DATA_TYPE;
    }

    if (r) return r;
    paddingNeeded = SDIF_PaddingRequired(&(m->header));
    if (paddingNeeded) {
	char pad[8] = "\0\0\0\0\0\0\0\0";
	r = SDIF_Write1(pad, paddingNeeded, sdif_handle);
    }
    return ESDIF_SUCCESS;
}

SDIFresult SDIFmem_WriteFrame(FILE *sdif_handle, SDIFmem_Frame f) {
    SDIFresult r;
    SDIFmem_Matrix p;

    assert(f != NULL);
    assert(sdif_handle != NULL);

    if (r = SDIF_WriteFrameHeader(&f->header, sdif_handle)) {
	return r;
    }

    for (p = f->matrices; p != NULL; p = p->next) {
	if (r = SDIFmem_WriteMatrix(sdif_handle, p)) return r;
    }

    return ESDIF_SUCCESS;
}
