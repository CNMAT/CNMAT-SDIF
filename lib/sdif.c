/*
 * Copyright (c) 1997,1998 Regents of the University of California.
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


#include <assert.h>
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
    "Seek failed while skipping frame",
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


struct SDIFMatrixStruct {
    SDIF_MatrixHeader header;
    void *data;
};


typedef struct sdif_matrix_node {
    SDIF_Matrix matrix;
    struct sdif_matrix_node *next;
} sdif_matrix_node;


typedef struct frame_node {
    SDIF_Frame frame;
    struct frame_node *prev;
    struct frame_node *next;
} frame_node;


struct SDIFFrameStruct {
    SDIF_FrameHeader header;
    sdif_matrix_node *matrices;  /* linked list of matrices. */
};


struct SDIFStreamStruct {
    char	 type[4];
    sdif_int32	 id;
    sdif_float64 interval;
    sdif_uint32	 interp_num;
    frame_node   *head;
    frame_node   *tail;
    frame_node   *pos;
};


/* prototypes for functions used only in this file. */
static void set_error_code(int code);
static void SizeofSanityCheck(void);
static frame_node * new_frame_node(void);
static sdif_matrix_node * new_sdif_matrix_node(void);



SDIF_Stream
SDIF_CreateStream(char *type) {

    struct SDIFStreamStruct *s;

    assert(type != NULL);

    if ((s = malloc(sizeof(struct SDIFStreamStruct))) == NULL) return NULL;

    s->type[0] = type[0];
    s->type[1] = type[1];
    s->type[2] = type[2];
    s->type[3] = type[3];
    s->id = 0;
    s->interval = 0;
    s->interp_num = 0;
    s->head = s->tail = s->pos = NULL;

    return s;

}


void
SDIF_SetStreamId(SDIF_Stream s, sdif_int32 id) {

    s->id = id;

}


void
SDIF_AddFrame(SDIF_Stream s, SDIF_Frame f) {

    frame_node *f;

    /* write me ;) */

}


SDIF_Frame
SDIF_CreateFrame(void) {

    SDIF_Frame f;

    if ((f = malloc(sizeof(struct SDIFFrameStruct))) == NULL) return NULL;

    f->header.frameType[0] = '\0';
    f->header.frameType[1] = '\0';
    f->header.frameType[2] = '\0';
    f->header.frameType[3] = '\0';
    f->header.size = 16;
    f->header.time = 0;
    f->header.streamID = 1234;
    f->header.matrixCount = 0;
    f->matrices = NULL;

    return f;

}


void
SDIF_SetFrameType(SDIF_Frame f, char *type) {

    assert(type != NULL);

    f->header.frameType[0] = type[0];
    f->header.frameType[1] = type[1];
    f->header.frameType[2] = type[2];
    f->header.frameType[3] = type[3];

}


void
SDIF_SetFrameTime(SDIF_Frame f, sdif_float64 time) {

    f->header.time = time;

}


sdif_float64
SDIF_GetFrameTime(SDIF_Frame f) {

    return f->header.time;

}


SDIF_Matrix
SDIF_CreateMatrix(char *type, SDIF_MatrixDataType d,
		  int num_rows, int num_cols) {

    SDIF_Matrix m;

    assert(type != NULL);
    assert(SDIF_GetMatrixDataTypeSize(d) != -1);
    assert(num_rows > 0);
    assert(num_cols > 0);

    if ((m = malloc(sizeof(struct SDIFMatrixStruct))) == NULL) return NULL;
    if ((m->data = malloc(num_rows * num_cols * SDIF_GetMatrixDataTypeSize(d)))
	== NULL) return NULL;

    m->header.matrixType[0] = type[0];
    m->header.matrixType[1] = type[1];
    m->header.matrixType[2] = type[2];
    m->header.matrixType[3] = type[3];
    m->header.matrixDataType = d;
    m->header.rowCount = num_rows;
    m->header.columnCount = num_cols;

    return m;

}


void
SDIF_AddMatrix(SDIF_Frame f, SDIF_Matrix m) {

    /* make sure the matrix being added does not have the same type
       as a matrix already in the frame */

    int sz;
    sdif_matrix_node *p;

    assert(f != NULL);
    assert(m != NULL);

    p = f->matrices;
    if (p == NULL) {
	f->matrices = new_sdif_matrix_node();
	f->matrices->matrix = m;
    } else {
	while (p->next != NULL) p = p->next;
	p->next->matrix = m;
    }

    sz = SDIF_GetMatrixDataTypeSize(m->header.matrixDataType) *
	m->header.rowCount * m->header.columnCount;
    if ((sz % 8) != 0) {
	sz += (8 - (sz % 8));
    }

    f->header.size += sz;
    f->header.matrixCount++;

}


int
SDIF_WriteFrame(FILE *sdif_handle, SDIF_Frame f) {

    int j, k, sz;
    sdif_matrix_node *p;

    assert(f != NULL);
    assert(sdif_handle != NULL);

    SDIF_WriteFrameHeader(&f->header, sdif_handle);
    p = f->matrices;
    while (p != NULL) {
	SDIF_WriteMatrixHeader(&p->matrix->header, sdif_handle);
	sz = SDIF_GetMatrixDataTypeSize(p->matrix->header.matrixDataType);
	for (j=0; j < p->matrix->header.rowCount; j++) {
	    for (k=0; k < p->matrix->header.columnCount; k++) {
		switch (sz) {
		    case 1:
		        SDIF_Write1(&p->matrix->data +
				    j*p->matrix->header.columnCount + k,
				    1, sdif_handle);
			break;
		    case 2:
		        SDIF_Write2(&p->matrix->data +
				    j*p->matrix->header.columnCount + k,
				    1, sdif_handle);
			break;
		    case 4:
			SDIF_Write4(&p->matrix->data +
				    j*p->matrix->header.columnCount + k,
				    1, sdif_handle);
			break;
		    case 8:
			SDIF_Write8(&p->matrix->data +
				    j*p->matrix->header.columnCount + k,
				    1, sdif_handle);
			break;
		    default:
			fprintf(stderr,
				"eh... unknown matrix data type encountered "
				"in SDIF_WriteFrame().  exiting...\n");
			exit(1);
			*((int *) 0) = 1;  /* matt did this. */
		}
	    }
	}
	p = p->next;
    }

    return 1;

}


int
SDIF_GetLastErrorCode(void) {
    return error_code;
}


char *
SDIF_GetLastErrorString(void) {
    return error_string_array[error_code];
}


void
SDIF_FillGlobalHeader(SDIF_GlobalHeader *h) {
    assert(h != NULL);
    SDIF_Copy4Bytes(h->SDIF, "SDIF");
    h->size = 8;
    memset(h->reserved,0,8);
}


int
SDIF_WriteGlobalHeader(SDIF_GlobalHeader *h, FILE *f) {
    assert(h != NULL);
    assert(f != NULL);
#ifdef LITTLE_ENDIAN
    if (write1(&(h->SDIF), 4, f) != 4) return -1;
    if (write4(&(h->size), 1, f) != 1) return -1;
    if (write1(&(h->reserved), 8, f) != 8) return -1;
    return 1;
#else
    return fwrite(h, sizeof(*h), 1, f) == 1;
#endif
}


FILE *
SDIF_OpenWrite(const char *filename) {

    SDIF_GlobalHeader h;
    FILE *result;

    SizeofSanityCheck();

    if ((result = fopen(filename, "wb")) == NULL) {
	set_error_code(ESDIF_SEE_ERRNO);
	return NULL;
    }
    SDIF_FillGlobalHeader(&h);
    if (SDIF_WriteGlobalHeader(&h, result) < 1) {
	fclose(result);
	set_error_code(ESDIF_SEE_ERRNO);
	return NULL;
    }

    return result;

}


int
SDIF_CloseWrite(FILE *f) {
    fflush(f);
    return fclose(f);
}


FILE *
SDIF_OpenRead(const char *filename) {

    FILE *result = NULL;

    SizeofSanityCheck();

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
    fseek(input, sgh.size, SEEK_CUR);

    return 1;

lose:
    set_error_code(ESDIF_BAD_SDIF_HEADER);
    return 0;

}


int
SDIF_CloseRead(FILE *f) {
    return fclose(f);
}


int
SDIF_ReadFrameHeader(SDIF_FrameHeader *fh, FILE *f) {

#ifdef LITTLE_ENDIAN
    if (SDIF_Read1(&(fh->frameType),4,f) != 4) return -1;
    if (SDIF_Read4(&(fh->size),1,f) != 1) return -1;
    if (SDIF_Read8(&(fh->time),1,f) != 1) return -1;
    if (SDIF_Read4(&(fh->streamID),1,f) != 1) return -1;
    if (SDIF_Read4(&(fh->matrixCount),1,f) != 1) return -1;
    return 1;
#else
    return fread(fh, sizeof(*fh), 1, f) == 1;
#endif

}


int
SDIF_WriteFrameHeader(SDIF_FrameHeader *fh, FILE *f) {

#ifdef LITTLE_ENDIAN
    if (SDIF_Write1(&(fh->frameType),4,f) != 4) return -1;
    if (SDIF_Write4(&(fh->size),1,f) != 1) return -1;
    if (SDIF_Write8(&(fh->time),1,f) != 1) return -1;
    if (SDIF_Write4(&(fh->streamID),1,f) != 1) return -1;
    if (SDIF_Write4(&(fh->matrixCount),1,f) != 1) return -1;
#ifdef __WIN32__
    fflush(f);
#endif
    return 1;
#else
    return fwrite(fh, sizeof(*fh), 1, f) == 1;
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
	set_error_code(ESDIF_FRAME_SKIP_FAILED);
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
    return fread(m, sizeof(*m), 1, f) == 1;
#endif

}


int
SDIF_WriteMatrixHeader(SDIF_MatrixHeader *m, FILE *f) {

#ifdef LITTLE_ENDIAN
    if (SDIF_Write1(&(m->matrixType),4,f) != 4) return -1;
    if (SDIF_Write4(&(m->matrixDataType),1,f) != 1) return -1;
    if (SDIF_Write4(&(m->rowCount),1,f) != 1) return -1;
    if (SDIF_Write4(&(m->columnCount),1,f) != 1) return -1;
    return 1;
#else
    return fwrite(m, sizeof(*m), 1, f) == 1;
#endif

}


int
SDIF_GetMatrixDataTypeSize(sdif_int32 d) {

    if ((d < 0) || (d > SDIF_NUM_MATRIX_TYPES)) return -1;
    return matrix_data_type_sizes[d];

}



#if 0
int
SDIF_GetFrameFromHandle(FILE *sdif_handle, SDIF_Frame f) {

    int i, sz;

    assert(sdif_handle != NULL);
    assert(f != NULL);

    /* read the frame header. */
    if (SDIF_ReadFrameHeader(&f->frame_header, sdif_handle) != 1) return -1;

    /* allocate as many matrix headers as the frame contains matrices. */
    if (f->header.matrixCount > 0) {

	f->matrix_headers = (SDIF_MatrixHeader *)
	    malloc(f->frame_header.matrixCount * sizeof(SDIF_MatrixHeader));
	if (f->matrix_headers == NULL) return -1;

	f->matrix_data = (void **)
	    malloc(f->header.matrixCount * sizeof(void *));
	if (f->matrix_data == NULL) {
	    free(f->matrix_headers);
	    return -1;
	}

    }

    /* for each matrix, read in its header and data. */
    for (i=0; i < f->header.matrixCount; i++) {
	SDIF_ReadMatrixHeader(&f->matrix_headers[i], sdif_handle);
	sz = f->matrix_headers[i].rowCount * f->matrix_headers[i].columnCount *
	    SDIF_GetMatrixDataTypeSize((SDIF_MatrixDataType)
				       f->matrix_headers[i].matrixDataType);
	f->matrix_data[i] = (void *) malloc(sz);
	if (f->matrix_data[i] == NULL) {
	    /* free() everything and return -1 */
	    return -1;
	}
	/* read it in */
    }

    return 1;

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


#ifdef LITTLE_ENDIAN
#define BUFSIZE 4096
static	char	p[BUFSIZE];
#endif


int
SDIF_Write1(void *block, size_t n, FILE *f) {
    return fwrite (block,1,n,f);
}


int
SDIF_Write2(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
    short temp;
    char *q = block;
    int	i, m = 2*n;

    if ((n << 1) > BUFSIZE) {
	int num = BUFSIZE >> 1;
	int numWritten;
	numWritten = write2(block, num, f);
	numWritten += write2(((char *) block) + num, n-num, f);
	return numWritten;
    }

    for (i = 0; i < m; i += 2) {
	p[i] = q[i+1];
	p[i+1] = q[i];
    }

    return fwrite(p,2,n,f);
#else
    return fwrite (block,2,n,f);
#endif

}


int
SDIF_Write4(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
    char *q = block;
    int i, m = 4*n;

    if ((n << 2) > BUFSIZE) {
	int num = BUFSIZE >> 2;
	int numWritten;
	numWritten = write4(block, num, f);
	numWritten += write4(((char *) block) + num, n-num, f);
	return numWritten;
    }

    for (i = 0; i < m; i += 4) {
	p[i] = q[i+3];
	p[i+3] = q[i];
	p[i+1] = q[i+2];
	p[i+2] = q[i+1];
    }

    return fwrite(p,4,n,f);
#else
    return fwrite (block,4,n,f);
#endif

}


int
SDIF_Write8(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
    char *q = block;
    int i, m = 8*n;

    if ((n << 3) > BUFSIZE) {
	int num = BUFSIZE >> 3;
	int numWritten;
	numWritten = write8(block, num, f);
	numWritten += write8(((char *) block) + num, n-num, f);
	return numWritten;
    }

    for (i = 0; i < m; i += 8) {
	p[i] = q[i+7];
	p[i+7] = q[i];
	p[i+1] = q[i+6];
	p[i+6] = q[i+1];
	p[i+2] = q[i+5];
	p[i+5] = q[i+2];
	p[i+3] = q[i+4];
	p[i+4] = q[i+3];
    }

    return fwrite(p,8,n,f);
#else
    return fwrite (block,8,n,f);
#endif

}


int
SDIF_Read1(void *block, size_t n, FILE *f) {
    return fread (block,1,n,f);
}


int
SDIF_Read2(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
    short temp;
    char *q = block;
    int i, m = 2*n;
    int result;

    if ((n << 1) > BUFSIZE) {
	int num = BUFSIZE >> 1;
	int numread;
	numread = read2(block, num, f);
	numread += read2(((char *) block) + num, n-num, f);
	return numread;
    }

    result = fread(p,2,n,f);

    for (i = 0; i < m; i += 2) {
	q[i] = p[i+1];
	q[i+1] = p[i];
    }

    return result;
#else
    return fread(block,2,n,f);
#endif

}


int
SDIF_Read4(void *block, size_t n, FILE *f) {
#ifdef LITTLE_ENDIAN
    long temp;
    char *q = block;
    int i, m = 4*n;
    int result;

    if ((n << 2) > BUFSIZE) {
	int num = BUFSIZE >> 2;
	int numread;
	numread = read4(block, num, f);
	numread += read4(((char *) block) + num, n-num, f);
	return numread;
    }

    result = fread(p,4,n,f);

    for (i = 0; i < m; i += 4) {
	q[i] = p[i+3];
	q[i+3] = p[i];
	q[i+1] = p[i+2];
	q[i+2] = p[i+1];
    }

    return result;

#else
    return fread(block,4,n,f);
#endif

}


int
SDIF_Read8(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
    long temp;
    char *q = block;
    int i, m = 8*n;
    int result;

    if ((n << 3) > BUFSIZE) {
	int num = BUFSIZE >> 3;
	int numread;
	numread = read8(block, num, f);
	numread += read8(((char *) block) + num, n-num, f);
	return numread;
    }

    result = fread(p,8,n,f);

    for (i = 0; i < m; i += 8) {
	q[i] = p[i+7];
	q[i+7] = p[i];
	q[i+1] = p[i+6];
	q[i+6] = p[i+1];
	q[i+2] = p[i+5];
	q[i+5] = p[i+2];
	q[i+3] = p[i+4];
	q[i+4] = p[i+3];
    }

    return result;

#else
    return fread(block,8,n,f);
#endif

}



/* Some stuff for particular frame types */

/* 1TRC */
int
SDIF_WriteRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f) {
    return (SDIF_Write4(row,4,f) == 4);
}


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


int
SDIF_Write1TRCVals(FILE *f,
		   sdif_float32 index, sdif_float32 freq,
		   sdif_float32 phase, sdif_float32 amp) {

    SDIF_RowOf1TRC data;

    data.index = index;
    data.freq = freq;
    data.phase = phase;
    data.amp = amp;

#ifdef LITTLE_ENDIAN
    if (write4(&data, 4, f) != 1) return -1;
#else
    if (fwrite (&data, sizeof(data), 1, f) != 1) return -1;
#endif

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


/* 1RES */
int
SDIF_WriteRowOf1RES(SDIF_RowOf1RES *row, FILE *f) {
    return (SDIF_Write4(row,4,f) == 4);
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

static void
set_error_code(int code) {
    error_code = code;
    if (code == 1) error_string_array[1] = strerror(errno);
}


static void
SizeofSanityCheck(void) {

    int OK = 1;

    if (sizeof(sdif_int32) != 4) {
	fprintf(stderr, "sizeof(sdif_int32) is %d!!!\n",
		sizeof(sdif_int32));
	OK = 0;
    }

    if (sizeof(sdif_float32) != 4) {
	fprintf(stderr, "sizeof(sdif_float32) is %d!!!\n",
		sizeof(sdif_float32));
	OK = 0;
    }

    if (sizeof(sdif_float64) != 8) {
	fprintf(stderr, "sizeof(sdif_float64) is %d!!!\n",
		sizeof(sdif_float64));
	OK = 0;
    }

    if (!OK) exit(1);

}


static frame_node *
new_frame_node(void) {

    frame_node *f;

    if ((f = malloc(sizeof(frame_node))) == NULL) return NULL;

    f->frame = NULL;
    f->prev = f->next = NULL;

    return f;

}


static sdif_matrix_node *
new_sdif_matrix_node(void) {

    sdif_matrix_node *m;

    if ((m = malloc(sizeof(sdif_matrix_node))) == NULL) return NULL;

    m->matrix = NULL;
    m->next = NULL;

    return m;

}
