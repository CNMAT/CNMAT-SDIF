/*
Copyright (c) 1996. 1997, 1998, 1999.  The Regents of the University of California
(Regents). All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for educational, research, and not-for-profit purposes,
without fee and without a signed licensing agreement, is hereby granted,
provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and distributions.
Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for commercial
licensing opportunities.

Written by Matt Wright, Amar Chaudhary, and Sami Khoury, The Center for New
Music and Audio Technologies, University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
     SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.

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

 10/1/99 version 2.3 by Matt: minor fixes for public release.
 10/12/99 Version 2.4 by Matt: changed return value convention
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
static SDIF_ErrorCode error_code = ESDIF_NONE;
static char *error_string_array[] = {
    "Everything's cool",
    (char *) NULL,  /* this will be set to strerror(errno). */
    "Bad SDIF header",
    "Frame header's size is too low for time tag and stream ID",
    "fseek() failed while skipping over data",
    "Unknown matrix data type encountered in SDIF_WriteFrame().",
    (char *) NULL,   /* this will be set by SizeofSanityCheck() */
    "End of data",
    "Bad SDIF matrix header",
    "Obsolete SDIF file from an old version of SDIF",
    "I/O error: couldn't write",
    "I/O error: couldn't read"
};

/* prototypes for functions used only in this file. */
/* static */ void set_error_code(int code);
static int SizeofSanityCheck(void);


SDIFresult SDIF_Init(void) {
	if (!SizeofSanityCheck()) {
		return ESDIF_BAD_SIZEOF;
	}
	return ESDIF_SUCCESS;
}


SDIF_ErrorCode
SDIF_GetLastErrorCode(void) {
    return error_code;
}


char *
SDIF_GetLastErrorString(void) {
    return error_string_array[error_code];
}


FILE *
SDIF_OpenWrite(const char *filename) {
    FILE *result;

    if ((result = fopen(filename, "wb")) == NULL) {
	set_error_code(ESDIF_SEE_ERRNO);
	return NULL;
    }
    if (!SDIF_BeginWrite(result)) {
	fclose(result);
	return NULL;
    }
    return result;
}

SDIFresult SDIF_BeginWrite(FILE *output) {
    SDIF_GlobalHeader h;

    SDIF_FillGlobalHeader(&h);
    return SDIF_WriteGlobalHeader(&h, output);
}

SDIFresult SDIF_CloseWrite(FILE *f) {
    fflush(f);
    if (fclose(f) == 0) {
	return SUCCESS;
    } else {
	return FAILURE;
    }
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


SDIFresult SDIF_BeginRead(FILE *input) {
    SDIF_GlobalHeader sgh;

    /* make sure the header is OK. */
    if (!SDIF_Read1(sgh.SDIF, 4, input)) goto lose;
    if (!SDIF_Str4Eq(sgh.SDIF, "SDIF")) goto lose;
    if (!SDIF_Read4(&sgh.size, 1, input)) goto lose;
    if (sgh.size % 8 != 0) goto lose;
    if (sgh.size < 8) goto lose;
    if (!SDIF_Read4(&sgh.SDIFversion, 1, input)) goto lose;
    if (!SDIF_Read4(&sgh.SDIFStandardTypesVersion, 1, input)) goto lose;

    if (sgh.SDIFversion < 3) {
	set_error_code(ESDIF_OBSOLETE_FILE_VERSION);
	return FAILURE;
    }

    if (sgh.SDIFStandardTypesVersion < 1) {
	set_error_code(ESDIF_OBSOLETE_FILE_VERSION);
	return FAILURE;
    }

    /* skip size-8 bytes.  (We already read the first two version numbers,
       but maybe there's more data in the header frame.) */

    if (sgh.size == 8) {
	return SUCCESS;
    }

    if (fseek(input, sgh.size-8, SEEK_CUR) == 0) {
	    return SUCCESS;
    }

lose:
    set_error_code(ESDIF_BAD_SDIF_HEADER);
    return FAILURE;
}

SDIFresult SDIF_CloseRead(FILE *f) {
    if (fclose(f) == 0) {
	return SUCCESS;
    } else {
	return FAILURE;
    }
}

void SDIF_FillGlobalHeader(SDIF_GlobalHeader *h) {
    assert(h != NULL);
    SDIF_Copy4Bytes(h->SDIF, "SDIF");
    h->size = 8;
    h->SDIFversion = SDIF_SPEC_VERSION;
    h->SDIFStandardTypesVersion = SDIF_LIBRARY_VERSION;
}


SDIFresult SDIF_WriteGlobalHeader(SDIF_GlobalHeader *h, FILE *f) {
    assert(h != NULL);
    assert(f != NULL);
#ifdef LITTLE_ENDIAN
    if (!write1(&(h->SDIF), 4, f)) return FAILURE;
    if (!write4(&(h->size), 1, f)) return FAILURE;
    if (!write4(&(h->SDIFversion), 1, f)) return FAILURE;
    if (!write4(&(h->SDIFStandardTypesVersion), 1, f)) return FAILURE;
    return SUCCESS;
#else
    if (fwrite(h, sizeof(*h), 1, f) == 1) {
	return SUCCESS;
    } else {
	return FAILURE;
    }
#endif
}


SDIFresult SDIF_ReadFrameHeader(SDIF_FrameHeader *fh, FILE *f) {
	size_t amount_read;

#ifdef LITTLE_ENDIAN
    if (SDIF_Read1(&(fh->frameType),4,f) != 4) {
    	if (feof(f)) {
	    set_error_code(ESDIF_END_OF_DATA);	    
	}
	return FAILURE;
    }
    if (!SDIF_Read4(&(fh->size),1,f)) return FAILURE;
    if (!SDIF_Read8(&(fh->time),1,f)) return FAILURE;
    if (!SDIF_Read4(&(fh->streamID),1,f)) return FAILURE;
    if (!SDIF_Read4(&(fh->matrixCount),1,f)) return FAILURE;
    return SUCCESS;
#else
	amount_read = fread(fh, sizeof(*fh), 1, f);
	if (amount_read == 1) return SUCCESS;
	if (amount_read == 0) {
		/* Now that fread failed, maybe we're at EOF. */
		if (feof(f)) {
		    set_error_code(ESDIF_END_OF_DATA);
		}
	}
	return FAILURE;
#endif /* LITTLE_ENDIAN */
}


SDIFresult SDIF_WriteFrameHeader(SDIF_FrameHeader *fh, FILE *f) {

#ifdef LITTLE_ENDIAN
    if (!SDIF_Write1(&(fh->frameType),4,f)) return FAILURE;
    if (!SDIF_Write4(&(fh->size),1,f)) return FAILURE;
    if (!SDIF_Write8(&(fh->time),1,f)) return FAILURE;
    if (!SDIF_Write4(&(fh->streamID),1,f)) return FAILURE;
    if (!SDIF_Write4(&(fh->matrixCount),1,f)) return FAILURE;
#ifdef __WIN32__
    fflush(f);
#endif
    return SUCCESS;
#else
    if (fwrite(fh, sizeof(*fh), 1, f) == 1) {
	return SUCCESS;
    } else {
	return FAILURE;
    }
#endif

}

SDIFresult SDIF_SkipFrame(SDIF_FrameHeader *head, FILE *f) {
    /* The header's size count includes the 8-byte time tag, 4-byte
       stream ID and 4-byte matrix count that we already read. */
    int bytesToSkip = head->size - 16;

    if (bytesToSkip < 0) {
		set_error_code(ESDIF_BAD_FRAME_HEADER);
		return FAILURE;
    }

    if (fseek(f, bytesToSkip, SEEK_CUR) != 0) {
		set_error_code(ESDIF_SKIP_FAILED);
		return FAILURE;
    }

    return SUCCESS;
}

SDIFresult SDIF_ReadMatrixHeader(SDIF_MatrixHeader *m, FILE *f) {
#ifdef LITTLE_ENDIAN
    if (!SDIF_Read1(&(m->matrixType),4,f)) return FAILURE;
    if (!SDIF_Read4(&(m->matrixDataType),1,f)) return FAILURE;
    if (!SDIF_Read4(&(m->rowCount),1,f)) return FAILURE;
    if (!SDIF_Read4(&(m->columnCount),1,f)) return FAILURE;
    return SUCCESS;
#else
    if (fread(m, sizeof(*m), 1, f) == 1) {
	return SUCCESS;
    } else {
	return FAILURE;
    }
#endif

}


SDIFresult SDIF_WriteMatrixHeader(SDIF_MatrixHeader *m, FILE *f) {
#ifdef LITTLE_ENDIAN
    if (!SDIF_Write1(&(m->matrixType),4,f)) return FAILURE;
    if (!SDIF_Write4(&(m->matrixDataType),1,f)) return FAILURE;
    if (!SDIF_Write4(&(m->rowCount),1,f)) return FAILURE;
    if (!SDIF_Write4(&(m->columnCount),1,f)) return FAILURE;
    return SUCCESS;
#else
    return (fwrite(m, sizeof(*m), 1, f) == 1) ? SUCCESS : FAILURE;
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


SDIFresult SDIF_SkipMatrix(SDIF_MatrixHeader *head, FILE *f) {
	int size = SDIF_GetMatrixDataSize(head);
	
	if (size < 0) {
		set_error_code(ESDIF_BAD_MATRIX_HEADER);
		return FAILURE;
	}
	
	if (fseek(f, size, SEEK_CUR) != 0) {
		set_error_code(ESDIF_SKIP_FAILED);
		return FAILURE;
	}		

	return SUCCESS;
}


SDIFresult 
SDIF_ReadMatrixData(void *putItHere, FILE *f, SDIF_MatrixHeader *head) {
    size_t datumSize = (size_t) SDIF_GetMatrixDataTypeSize(head->matrixDataType);
    size_t numItems = (size_t) (head->rowCount * head->columnCount);
    
#ifdef LITTLE_ENDIAN
    switch (datumSize) {
        case 1:
            return SDIF_Read1(putItHere, numItems, f);
            break;
        case 2:
            return SDIF_Read2(putItHere, numItems, f);
            break;
        case 4:
            return SDIF_Read4(putItHere, numItems, f);
            break;
        case 8:
            return SDIF_Read8(putItHere, numItems, f);
            break;
        default:
            set_error_code(ESDIF_BAD_MATRIX_DATA_TYPE);
            return FAILURE;
    }
    /* This is never reached */
    return FAILURE;
#else
    return (fread(putItHere, datumSize, numItems, f) == numItems) ? SUCCESS : FAILURE;
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


SDIFresult SDIF_Write1(void *block, size_t n, FILE *f) {
    if (fwrite (block,1,n,f) == n) {
	return SUCCESS;
    } else {
	set_error_code(ESDIF_WRITE_FAILED);
	return FAILURE;
    }
}


SDIFresult SDIF_Write2(void *block, size_t n, FILE *f) {
#ifdef LITTLE_ENDIAN
    char *q = block;
    int	i, m = 2*n;

    if ((n << 1) > BUFSIZE) {
	/* Too big for buffer */
	int num = BUFSIZE >> 1;
	if (!write2(block, num, f)) return FAILURE;
	return write2(((char *) block) + num, n-num, f)
    }

    for (i = 0; i < m; i += 2) {
	p[i] = q[i+1];
	p[i+1] = q[i];
    }

    if (fwrite(p,2,n,f)==n) {
	return SUCCESS;
    } else {
	set_error_code(ESDIF_WRITE_FAILED);
	return FAILURE;
    }
#else
    if (fwrite (block,2,n,f) == n) {
	return SUCCESS;
    } else {
	set_error_code(ESDIF_WRITE_FAILED);
	return FAILURE;
    }
#endif
}



SDIFresult SDIF_Write4(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
    char *q = block;
    int i, m = 4*n;

    if ((n << 2) > BUFSIZE) {
	int num = BUFSIZE >> 2;
	if (!write4(block, num, f)) return FAILURE;
	return write4(((char *) block) + num, n-num, f);
    }

    for (i = 0; i < m; i += 4) {
	p[i] = q[i+3];
	p[i+3] = q[i];
	p[i+1] = q[i+2];
	p[i+2] = q[i+1];
    }

    if (fwrite(p,4,n,f) == n) {
	return SUCCESS;
    } else {
        set_error_code(ESDIF_WRITE_FAILED);
        return FAILURE;
    }
#else
    if (fwrite(block,4,n,f) == n) {
        return SUCCESS;
    } else {
        set_error_code(ESDIF_WRITE_FAILED);
        return FAILURE;
    }
#endif

}



SDIFresult SDIF_Write8(void *block, size_t n, FILE *f) {
#ifdef LITTLE_ENDIAN
    char *q = block;
    int i, m = 8*n;

    if ((n << 3) > BUFSIZE) {
	int num = BUFSIZE >> 3;
	if (!write8(block, num, f)) return FAILURE;
	return write8(((char *) block) + num, n-num, f);
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

    if (fwrite(p,8,n,f) == n) {
        return SUCCESS;
    } else {
        set_error_code(ESDIF_WRITE_FAILED);
        return FAILURE;
    }
#else
    if (fwrite(block,8,n,f) == n) {
        return SUCCESS;
    } else {
        set_error_code(ESDIF_WRITE_FAILED);
        return FAILURE;
    }
#endif
}


xxx Need to set error codes on failues of these procedures...

int
SDIF_Read1(void *block, size_t n, FILE *f) {
    if (fread (block,1,n,f) == n) {
	return SUCCESS;
    } else {
	set_error_code(ESDIF_READ_FAILED);
	return FAILURE;
    }
}


int
SDIF_Read2(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
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
    return (int) fread(block,2,n,f);
#endif

}


int
SDIF_Read4(void *block, size_t n, FILE *f) {
#ifdef LITTLE_ENDIAN
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
    return (int)fread(block,4,n,f);
#endif

}


int
SDIF_Read8(void *block, size_t n, FILE *f) {

#ifdef LITTLE_ENDIAN
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
    return (int) fread(block,8,n,f);
#endif
}


/* static function definitions follow. */

/* static */ void
set_error_code(int code) {
    error_code = code;
    if (code == ESDIF_SEE_ERRNO)
	error_string_array[ESDIF_SEE_ERRNO] = strerror(errno);
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
