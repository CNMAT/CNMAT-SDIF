/*
Copyright (c) 1999.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for educational, research, and not-for-profit purposes, without
fee and without a signed licensing agreement, is hereby granted, provided that
the above copyright notice, this paragraph and the following two paragraphs
appear in all copies, modifications, and distributions.  Contact The Office of
Technology Licensing, UC Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley,
CA 94720-1620, (510) 643-7201, for commercial licensing opportunities.

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
    
*/

#include <stdio.h>
#include <string.h> /* for strerror() */
#include <assert.h>
#include <errno.h>
#include "sdif.h"
#include "sdif-mem.h"



/* Global variables local to this file */
static void *(*my_malloc)(int numBytes);
static void (*my_free)(void *memory, int numBytes);


/* Error world */
static void set_error_code(int code);
static int error_code = ESDIFM_NONE;
static char *error_string_array[ESDIFM_NUM_ERRORS] = {
    "Everything's cool",
    (char *) NULL,  /* this will be set to strerror(errno). */
    (char *) NULL,  /* this will be set to SDIF_GetLastErrorString(). */
    "out of memory"
};

static void set_error_code(int code) {
    error_code = code;
    if (code == 1) error_string_array[1] = strerror(errno);
    if (code == 2) error_string_array[2] = SDIF_GetLastErrorString();
}

int SDIFM_GetLastErrorCode(void) {
    return error_code;
}

char *SDIFM_GetLastErrorString(void) {
    return error_string_array[error_code];
}



int SDIFmem_Init(void *(*MemoryAllocator)(int numBytes), 
              void (*MemoryFreer)(void *memory, int numBytes)) {
	my_malloc = MemoryAllocator;
	my_free = MemoryFreer;

	return 0;
}


SDIFmem_Frame
SDIFmem_CreateEmptyFrame(void) {

    SDIFmem_Frame f;

	f = (*my_malloc)(sizeof(*f));
    if (f == NULL) {
    	set_error_code(ESDIFM_OUT_OF_MEMORY);
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


SDIFmem_Matrix
SDIFmem_CreateEmptyMatrix(void) {

    SDIFmem_Matrix result;

	result = (*my_malloc)(sizeof(*result));

    if (result == NULL) {
    	set_error_code(ESDIF_OUT_OF_MEMORY);
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
    

SDIFmem_Frame SDIFmem_ReadFrameContents(SDIF_FrameHeader *head, FILE *f, SDIFmem_Frame prev, SDIFmem_Frame next) {
	/* The user has just read the header for this frame; now we have to read all the
	   frame's matrices, put them in an SDIFmem_Matrix linked list, and then stuff everything
	   into an SDIFmem_Frame. */
	   
	SDIFmem_Frame result;
	SDIFmem_Matrix matrix;
	int i, sz;
	SDIFmem_Matrix *prevNextPtr;

	result = (SDIFmem_Frame) (*my_malloc)(sizeof(*result));
	
	if (result == 0) {
		set_error_code(ESDIF_OUT_OF_MEMORY);
		return 0;
	}
	
	result->header = *head;
	result->prev = prev;
	result->next = next;
	result->matrices = 0;
	
	prevNextPtr = &(result->matrices);
	
	for (i = 0; i < head->matrixCount; ++i) {
		matrix = SDIFmem_CreateEmptyMatrix();
		if (matrix == 0) {
			return 0;
		}
		
		if (SDIF_ReadMatrixHeader(&(matrix->header), f) != 1) {
			set_error_code(ESDIF_BAD_MATRIX_HEADER);
			return 0;
		}
		
		sz = SDIF_GetMatrixDataSize(&(matrix->header));
		
		matrix->data = (*my_malloc)(sz);
		if (matrix->data == 0) {
			set_error_code(ESDIF_OUT_OF_MEMORY);
			return 0;
		}
		
		if (SDIF_ReadMatrixData(matrix->data, f, &(matrix->header)) != 1) {
			set_error_code(ESDIF_BAD_MATRIX);
			return 0;
		}
		
		/* Get the linked list pointers right */
		*(prevNextPtr) = matrix;
		prevNextPtr = &(matrix->next);
		matrix->next = 0;
	}
	return result;
}


SDIFmem_Frame SDIFmem_ReadFrame(FILE *f, SDIFmem_Frame prev, SDIFmem_Frame next) {
    SDIF_FrameHeader fh;

    if (SDIF_ReadFrameHeader(&fh, f) != 1) {
	return 0;
    }

    return SDIFmem_ReadFrameContents(&fh, f, prev, next);
}


int SDIFmem_AddMatrix(SDIFmem_Frame f, SDIFmem_Matrix m) {
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
    		/* make sure the matrix being added does not have the same type
     		   as a matrix already in the frame */
     		if (SDIF_Str4Eq(p->header.matrixType, m->header.matrixType)) {
     			return 0;
     		}
     		last = p;
        	p = p->next;
        }
        last->next = m;
    }

    sz = SDIF_GetMatrixDataSize(&(m->header));

    f->header.size += sz + sizeof(SDIF_MatrixHeader);
    f->header.matrixCount++;
    return 1;
}


int
SDIFmem_WriteFrame(FILE *sdif_handle, SDIFmem_Frame f) {
    size_t sz, numElements;
    SDIFmem_Matrix p;

    assert(f != NULL);
    assert(sdif_handle != NULL);

    if (SDIF_WriteFrameHeader(&f->header, sdif_handle) != 1) {
	return 0;
    }

    for(p = f->matrices; p != NULL; p = p->next) {
        if (SDIF_WriteMatrixHeader(&(p->header), sdif_handle) != 1) {
	    return 0;
	}
    sz = (size_t) SDIF_GetMatrixDataTypeSize(p->header.matrixDataType);

	numElements = (size_t) (p->header.rowCount * p->header.columnCount);

	switch (sz) {
	    case 1:
		SDIF_Write1(p->data, numElements, sdif_handle);
		break;
	    case 2:
		SDIF_Write2(p->data, numElements, sdif_handle);
		break;
	    case 4:
		SDIF_Write4(p->data, numElements, sdif_handle);
		break;
	    case 8:
		SDIF_Write8(p->data, numElements, sdif_handle);
		break;
		
	    default:
		set_error_code(ESDIF_BAD_MATRIX_DATA_TYPE);
		return 0;
	}
    }

    return 1;
}
