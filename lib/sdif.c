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



 Matt Wright, 12/4/96

 5/12/97 version 1.1 by Matt and Amar, incorporating little-endian issues

 1/12/98 version 1.2 by Amar, conforms to revised SDIF spec

 11/5/1998 version 1.3 by Sami, changed error reporting, included stdio.h,
        and added SDIF_BeginRead().

 11/13/1998 version 2.0 by Sami, renamed functions and types to use "sdif"
        and "SDIF" prefixes, changed internal error reporting, added
        documentation in sdif.h, and incremented major version because
        programs which included sdif.h will no longer compile without changes.

 11/16/1998 version 2.0.1 by Sami, added sdif_int32, sdif_uint32, and
        sdif_unicode typedefs.  Added SDIF_GetMatrixDataTypeSize().

 12/1/1998 version 2.0.2 by Sami, created SDIF_Matrix, SDIF_Frame, and
        SDIF_Stream types and a set of functions for creating and manipulating
        them.  This was done as the foundation for continued work on SDIF
        streaming.

 1/25/99 version 2.1 by Matt: added SDIF_Init() to allow memory allocation
    in Max/MSP; lots of other little fixes.

 9/20/99 version 2.2 by Matt: Incorporating changes to the format
    from my 6/99 visit to IRCAM.  Moved memory stuff to sdif-mem.[ch].

*/


#define REALASSERT
#ifdef REALASSERT
#include <assert.h>
#else
#define assert(x) /* Do nothing */
#endif

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
    "Bad SDIF matrix header",
    "Obsolete SDIF file from an old version of SDIF"
};

/* prototypes for functions used only in this file. */
/* static */ void set_error_code(int code);
static int SizeofSanityCheck(void);



int SDIF_Init(void) {
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
SDIF_OpenWrite(const char *filename) {

    SDIF_GlobalHeader h;
    FILE *result;

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
    if (sgh.size < 8) goto lose;
    if (SDIF_Read4(&sgh.SDIFversion, 1, input) != 1) goto lose;
    if (SDIF_Read4(&sgh.SDIFStandardTypesVersion, 1, input) != 1) goto lose;

    if (sgh.SDIFversion < 3) {
	set_error_code(ESDIF_OBSOLETE_FILE_VERSION);
	return 0;
    }

    if (sgh.SDIFStandardTypesVersion < 1) {
	set_error_code(ESDIF_OBSOLETE_FILE_VERSION);
	return 0;
    }


    /* skip size-8 bytes.  (We already read the first two version numbers,
       but maybe there's more data in the header frame.) */

    if (sgh.size == 8) {
	return 1;
    }

    if (fseek(input, sgh.size-8, SEEK_CUR) == 0) {
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
    h->SDIFversion = SDIF_SPEC_VERSION;
    h->SDIFStandardTypesVersion = SDIF_LIBRARY_VERSION;
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


int
SDIF_ReadFrameHeader(SDIF_FrameHeader *fh, FILE *f) {
	size_t amount_read;

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
    if (d < 0) return 0;

    /* The low order byte now encodes the DataTypeSize. */
    return d & 0xff;
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


sdif_int32 SDIF_UniqueStreamID(void) {
  static int id;
  return ++id;
}



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


/*****************************************/
/* Some stuff for particular frame types */
/*****************************************/


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
                      sdif_float32 *ampp, sdif_float32 *phasep) {
    SDIF_RowOf1TRC data;

#ifdef LITTLE_ENDIAN
    if (SDIF_Read4(&data, 4, f) != 1) return -1;
#else
    if (fread(&data, sizeof(data), 1, f) != 1) return -1;
#endif

    *indexp = data.index;
    *freqp = data.freq;
    *ampp = data.amp;
    *phasep = data.phase;

    return 0;

}


int
SDIF_Write1TRCVals(FILE *f,
		   sdif_float32 index, sdif_float32 freq,
		   sdif_float32 amp, sdif_float32 phase) {

    SDIF_RowOf1TRC data;

    data.index = index;
    data.freq = freq;
    data.amp = amp;
    data.phase = phase;

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
     plus four 4-byte floating point numbers (index, freq, amp, phase)
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
