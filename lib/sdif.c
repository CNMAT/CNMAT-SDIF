/*
* Copyright (c) 1997,1998, 1999 Regents of the University of California.
* All rights reserved.
* The name of the University may not be used to endorse or promote
* products derived from this software without specific prior written
* permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
* IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
* PURPOSE.

  sdif.c

  Utilities for formatting data into SDIF

  SDIF spec: http://www.cnmat.berkeley.edu/SDIF/

  See sdif.h for revision history.

*/

/* #include <assert.h> */
#define assert(x) /* Do nothing */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdif.h"


/* error handling stuff. */
static int error_code = ESDIF_NONE;
static char *error_string_array[ESDIF_NUM_ERRORS] = {
    "Everything's cool",
    (char *) NULL,  /* this will be set to strerror(errno). */
    "Bad SDIF header",
    "Frame header's size is too low for time tag and stream ID",
    "Seek failed while skipping over data",
"Unknown matrix data type encountered in SDIF_WriteFrame().",
(char *) NULL,   /* this will be set by SizeofSanityCheck() */
"Out of memory",
"Bad SDIF matrix",
"Bad SDIF matrix header"
};


/* this array must be kept in sync with the matrix data type codes in sdif.h
   and the library procedures SDIF_GetMatrixDataTypeSize() and
   SDIF_WriteFrame(). */
const unsigned int matrix_data_type_sizes[SDIF_NUM_MATRIX_TYPES] = {
    2,  /* SDIF_UNICODE */
    4,  /* SDIF_FLOAT32 */
    8,  /* SDIF_FLOAT64 */
    4,  /* SDIF_INT32 */
    4,  /* SDIF_UINT32 */
    1,  /* SDIF_BYTE */
};



/* prototypes for functions used only in this file. */
/* static */ void set_error_code(int code);
static int SizeofSanityCheck(void);

/* Global variables local to this file */
static void *(*my_malloc)(int numBytes);
static void (*my_free)(void *memory, int numBytes);


int SDIF_Init(void *(*MemoryAllocator)(int numBytes), 
              void (*MemoryFreer)(void *memory, int numBytes)) {
my_malloc = MemoryAllocator;
my_free = MemoryFreer;

if (!SizeofSanityCheck()) {
return 1;
}
return 0;
}


int
SDIF_GetLastErrorCode(void) {
    return error_code;
}


char *
SDIF_GetLastErrorString(void) {
    return error_string_array[error_code];
}

FILE *
SDIF_OpenRead(const char *filename) {

    FILE *result = NULL;

    if ((result = fopen(filename, "rb")) == NULL) {
        set_error_code(ESDIF_SEE_ERRNO);
        return NULL;
    }

    if (!SDIF_BeginRead(result)) {
        fclose(result);
        return NULL;
    }

    return result;
}

int
SDIF_BeginRead(FILE *input) {

    SDIF_GlobalHeader sgh;

    /* make sure the header is OK. */
    if (SDIF_Read1(sgh.SDIF, 4, input) != 4) goto lose;
    if (!SDIF_Str4Eq(sgh.SDIF, "SDIF")) goto lose;
    if (SDIF_Read4(&sgh.size, 1, input) != 1) goto lose;
    if (sgh.size % 8 != 0) goto lose;

    /* skip size bytes */
    if (fseek(input, sgh.size, SEEK_CUR) == 0) {
    return 1;
}

lose:
    set_error_code(ESDIF_BAD_SDIF_HEADER);
    return 0;

}

int
SDIF_CloseRead(FILE *f) {
    return fclose(f);
}


void
SDIF_FillGlobalHeader(SDIF_GlobalHeader *h) {
    assert(h != NULL);
    SDIF_Copy4Bytes(h->SDIF, "SDIF");
    h->size = 8;
    memset(h->reserved,0,8);
}

int
SDIF_ReadFrameHeader(SDIF_FrameHeader *fh, FILE *f) {

#ifdef LITTLE_ENDIAN
    if (SDIF_Read1(&(fh->frameType),4,f) != 4) {
    if (feof(f)) return 0;
    return -1;
    }
    if (SDIF_Read4(&(fh->size),1,f) != 1) return -1;
    if (SDIF_Read8(&(fh->time),1,f) != 1) return -1;
    if (SDIF_Read4(&(fh->streamID),1,f) != 1) return -1;
    if (SDIF_Read4(&(fh->matrixCount),1,f) != 1) return -1;
    return 1;
#else
	size_t amount_read;

amount_read = fread(fh, sizeof(*fh), 1, f);
if (amount_read == 1) return 1;
if (amount_read == 0) {
/* Now that fread failed, maybe we're at EOF. */
if (feof(f)) return 0;
}
return -1;
#endif

}


int
SDIF_SkipFrame(SDIF_FrameHeader *head, FILE *f) {

    /* The header's size count includes the 8-byte time tag, 4-byte
       stream ID and 4-byte matrix count that we already read. */
    int bytesToSkip = head->size - 16;

    if (bytesToSkip < 0) {
set_error_code(ESDIF_BAD_FRAME_HEADER);
return -1;
    }

    if (fseek(f, bytesToSkip, SEEK_CUR) != 0) {
set_error_code(ESDIF_SKIP_FAILED);
return -2;
    }

    return 1;

}

int
SDIF_ReadMatrixHeader(SDIF_MatrixHeader *m, FILE *f) {

#ifdef LITTLE_ENDIAN
    if (SDIF_Read1(&(m->matrixType),4,f) != 4) return -1;
    if (SDIF_Read4(&(m->matrixDataType),1,f) != 1) return -1;
    if (SDIF_Read4(&(m->rowCount),1,f) != 1) return -1;
    if (SDIF_Read4(&(m->columnCount),1,f) != 1) return -1;
    return 1;
#else
    return (fread(m, sizeof(*m), 1, f) == 1) ? 1 : 0;
#endif

}

int
SDIF_GetMatrixDataTypeSize(sdif_int32 d) {

    if ((d < 0) || (d > SDIF_NUM_MATRIX_TYPES)) return -1;
    return matrix_data_type_sizes[d];

}

int SDIF_GetMatrixDataSize(SDIF_MatrixHeader *m) {
int size;
size = SDIF_GetMatrixDataTypeSize(m->matrixDataType) *
   m->rowCount * m->columnCount;

    if ((size % 8) != 0) {
        size += (8 - (size % 8));
    }

return size;
}


int SDIF_SkipMatrix(SDIF_MatrixHeader *head, FILE *f) {
int size = SDIF_GetMatrixDataSize(head);

if (size < 0) {
set_error_code(ESDIF_BAD_MATRIX_HEADER);
return -1;
}

if (fseek(f, size, SEEK_CUR) != 0) {
set_error_code(ESDIF_SKIP_FAILED);
return -2;
} 

return 1;
}


int SDIF_ReadMatrixData(void *putItHere, FILE *f, SDIF_MatrixHeader *head) {
int datumSize = SDIF_GetMatrixDataTypeSize(head->matrixDataType);
int numItems = head->rowCount * head->columnCount;

#ifdef LITTLE_ENDIAN
switch (datumSize) {
case 1:
return (SDIF_Read1(putItHere, numItems, f) == numItems) ? 1 : 0;
break;
case 2:
return (SDIF_Read2(putItHere, numItems, f) == numItems) ? 1 : 0;
break;
case 4:
return (SDIF_Read4(putItHere, numItems, f) == numItems) ? 1 : 0;
break;
case 8:
return (SDIF_Read8(putItHere, numItems, f) == numItems) ? 1 : 0;
break;
default:
set_error_code(ESDIF_BAD_MATRIX_DATA_TYPE);
return 0;
}
return 1;
#else
return (fread(putItHere, datumSize, numItems, f) == numItems) ? 1 : 0;
#endif
}

SDIF_Frame
SDIF_CreateEmptyFrame(void) {

    SDIF_Frame f;

f = (*my_malloc)(sizeof(*f));
    if (f == NULL) {
    set_error_code(ESDIF_OUT_OF_MEMORY);
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


void SDIF_FreeFrame(SDIF_Frame f) {
SDIF_Matrix m, next;

if (f == 0) return;

m = f->matrices;
while (m != NULL) {
next = m->next;
SDIF_FreeMatrix(m);
m = next;
}

(*my_free)(f, sizeof(*f));
} 


SDIF_Matrix
SDIF_CreateEmptyMatrix(void) {

    SDIF_Matrix result;

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


void SDIF_FreeMatrix(SDIF_Matrix m) {
if (m->data != 0) {
(*my_free)(m->data, SDIF_GetMatrixDataSize(&(m->header)));
}
(*my_free)(m, sizeof(*m));
}



SDIF_Frame SDIF_ReadFrameContents(SDIF_FrameHeader *head, FILE *f, SDIF_Frame prev, SDIF_Frame next) {
/* The user has just read the header for this frame; now we have to read all the
   frame's matrices, put them in an SDIF_Matrix linked list, and then stuff everything
   into an SDIF_Frame. */
   
SDIF_Frame result;
SDIF_Matrix matrix;
int i, sz;
SDIF_Matrix *prevNextPtr;

result = (SDIF_Frame) (*my_malloc)(sizeof(*result));

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
matrix = SDIF_CreateEmptyMatrix();
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



#if 0
int
SDIF_AddMatrix(SDIF_Frame f, SDIF_Matrix m) {
    int sz;
struct SDIFMatrixStruct *p, *last;

    assert(f != NULL);
    assert(m != NULL);
    assert(m->next == NULL);

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
        last->next->matrix = m;
    }

    sz = SDIF_GetMatrixDataSize(&(m->header));

    f->header.size += sz;
    f->header.matrixCount++;
}
#endif


int
SDIF_Str4Eq(const char *ths, const char *that) {
    return ths[0] == that[0] && ths[1] == that[1] &&
ths[2] == that[2] && ths[3] == that[3];
}


void
SDIF_Copy4Bytes(char *target, const char *string) {
    target[0] = string[0];
    target[1] = string[1];
    target[2] = string[2];
    target[3] = string[3];
}


int
SDIF_Read1(void *block, size_t n, FILE *f) {
    return fread (block,1,n,f);
}


int
SDIF_Read2(void *block, size_t n, FILE *f) {
	unsigned int i;
	int t, result;
	char *b = (char *)block;

	result = fread(block,2,n,f);

#ifdef LITTLE_ENDIAN
	for (i=0;i < 2*n;i+=2) {
		t = b[i];
		b[i] = b[i+1];
		b[i+1] = t;
	}
#endif

    return result;

}


int
SDIF_Read4(void *block, size_t n, FILE *f) {
	int t1,t2, result;
	unsigned int i;
	char *b = (char *)block;

    result = fread(block,4,n,f);

#ifdef LITTLE_ENDIAN
	for (i=0;i < 4*n; i+=4 ) {
		t1 = b[i+3];
		t2 = b[i+2];
		b[i+2] = b[i+1];
		b[i+3] = b[i];
		b[i] = t1;
		b[i+1] = t2;
	}
#endif

	return result;
}


int
SDIF_Read8(void *block, size_t n, FILE *f) {
	int t[4], result;
	unsigned int i;
	char *b = (char *)block;
    
	result = fread(block,8,n,f);

#ifdef LITTLE_ENDIAN

	for (i=0; i < 8*n; i+=8) {
		t[0] = b[i+7];
		t[1] = b[i+6];
		t[2] = b[i+5];
		t[3] = b[i+4];
		b[i+4] = b[i+3];
		b[i+5] = b[i+2];
		b[i+6] = b[i+1];
		b[i+7] = b[i];
		b[i] = t[0];
		b[i+1] = t[1];
		b[i+2] = t[2];
		b[i+3] = t[3];
	}

#endif

	return result;

}



/* Some stuff for particular frame types */

/* 1TRC */

int
SDIF_ReadRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f) {
    return SDIF_Read4(row,4,f) == 4;
}


int
SDIF_Read1TRCVals(FILE *f,
  sdif_float32 *indexp, sdif_float32 *freqp,
  sdif_float32 *phasep, sdif_float32 *ampp) {

    SDIF_RowOf1TRC data;

#ifdef LITTLE_ENDIAN
    if (SDIF_Read4(&data, 4, f) != 1) return -1;
#else
    if (fread(&data, sizeof(data), 1, f) != 1) return -1;
#endif

    *indexp = data.index;
    *freqp = data.freq;
    *phasep = data.phase;
    *ampp = data.amp;

    return 0;

}


sdif_int32
SizeOf1TRCFrame(int numTracks) {

  /* 16 bytes for the time stamp, ID and matrix count, plus 16 bytes for
     the # rows, # columns, matrix type and matrix data type,
     plus four 4-byte floating point numbers (index, amp, freq, phase)
     for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

    return  16 + 16 + (4 * 4 * numTracks);

}


int
SDIF_ReadRowOf1RES(SDIF_RowOf1RES *row, FILE *f) {
    return SDIF_Read4(row,4,f) == 4;
}


sdif_int32
SDIF_SizeOf1RESFrame(int numResonances) {

  /* 16 bytes for the time stamp, ID and matrix count, plus 16 bytes for
     the # rows, # columns, matrix type and matrix data type,
     plus four 4-byte floating point numbers (freq, gain, bw phase)
     for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

    return  16 + 16 + (4 * 4 * numResonances);

}



/* static function definitions follow. */

/* static */ void
set_error_code(int code) {
    error_code = code;
    if (code == ESDIF_SEE_ERRNO) error_string_array[ESDIF_SEE_ERRNO] = strerror(errno);
}


static int SizeofSanityCheck(void) {
    int OK = 1;
    static char errorMessage[sizeof("sizeof(sdif_float64) is 999!!!")];

    if (sizeof(sdif_int32) != 4) {
    sprintf(errorMessage, "sizeof(sdif_int32) is %d!", sizeof(sdif_int32));
OK = 0;
    }

    if (sizeof(sdif_float32) != 4) {
sprintf(errorMessage, "sizeof(sdif_float32) is %d!", sizeof(sdif_float32));
OK = 0;
    }

    if (sizeof(sdif_float64) != 8) {
sprintf(errorMessage, "sizeof(sdif_float64) is %d!", sizeof(sdif_float64));
OK = 0;
    }

if (!OK) {
error_string_array[ESDIF_BAD_SIZEOF] = errorMessage;
}
return OK;
}
