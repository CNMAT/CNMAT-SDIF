/*
 * Copyright (c) 1996, 1997, 1998 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 sdif.h

 API for working with SDIF.

 SDIF spec: http://www.cnmat.berkeley.edu/SDIF/

 See sdif.c for revision history.
 version 2.2

 #include <stdio.h> before this file for FILE *.

*/


#ifndef __SDIF_H
#define __SDIF_H


/* These numbers appear in the global header frame of any SDIF files
   created with this library */

#define SDIF_SPEC_VERSION 3
#define SDIF_LIBRARY_VERSION 1



/* Create 4-byte and 8-byte int and float typedefs. */

#ifdef __sgi
    typedef unsigned short sdif_unicode;
    typedef int                   sdif_int32;
    typedef unsigned int   sdif_uint32;
    typedef float           sdif_float32;
    typedef double           sdif_float64;
#elif defined(__WIN32__) || defined(_WINDOWS)
    #ifndef _WINDOWS_
    	#include <windows.h>
    #endif 
    typedef unsigned short sdif_unicode;
    typedef int                   sdif_int32;
    typedef unsigned int   sdif_uint32;
    typedef float           sdif_float32;
    typedef double           sdif_float64;
#elif defined(__LINUX__)
    typedef unsigned short sdif_unicode;
    typedef int                   sdif_int32;
    typedef unsigned int   sdif_uint32;
    typedef float           sdif_float32;
    typedef double           sdif_float64;
#else

    /* These won't necessarily be the right size on any conceivable
       platform, so you may need to change them by hand.  The call to
       SDIF_Init() performs a sanity check of the sizes of these types,
       so if they're wrong you'll find out about it. */

    typedef unsigned short sdif_unicode;
    typedef long           sdif_int32;
    typedef unsigned long  sdif_uint32;
    typedef float           sdif_float32;
    typedef double           sdif_float64;

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* update sdif.c to reflect any changes made to these error values. */
#define ESDIF_NONE 0
#define ESDIF_SEE_ERRNO 1
#define ESDIF_BAD_SDIF_HEADER 2
#define ESDIF_BAD_FRAME_HEADER 3
#define ESDIF_SKIP_FAILED 4
#define ESDIF_BAD_MATRIX_DATA_TYPE 5
#define ESDIF_BAD_SIZEOF 6
#define ESDIF_OUT_OF_MEMORY 7
#define ESDIF_BAD_MATRIX 8
#define ESDIF_BAD_MATRIX_HEADER 9
#define ESDIF_OBSOLETE_FILE_VERSION 10
#define ESDIF_NUM_ERRORS 11


/******* These data structures match the SDIF spec *******/

/* the header for the entire SDIF data. */
typedef struct {
    char  SDIF[4];          /* must be 'S', 'D', 'I', 'F' */
    sdif_int32 size;        /* size of header frame, not including SDIF or size. */
    sdif_int32 SDIFversion;
    sdif_int32 SDIFStandardTypesVersion;
} SDIF_GlobalHeader;


/* the header for each frame of SDIF data. */
typedef struct {
    char         frameType[4];        /* should be a registered frame type */
    sdif_int32   size;                /* # bytes in this frame, not including
                                         frameType or size */
    sdif_float64 time;                /* time corresponding to data chunk */
    sdif_int32   streamID;            /* frames that go together have the same ID */
    sdif_int32   matrixCount;         /* number of matrices in frame */
} SDIF_FrameHeader;


/* the header for each matrix of SDIF data. */
typedef struct {
    char matrixType[4];
    sdif_int32 matrixDataType;
    sdif_int32 rowCount;
    sdif_int32 columnCount;
} SDIF_MatrixHeader;



/* codes for data types used in matrices.
   these must be kept in sync with the array in sdif.c. */
typedef enum {
    SDIF_FLOAT32 = 0x0004,
    SDIF_FLOAT64 = 0x0108,
    SDIF_INT32 = 0x0104,
    SDIF_UINT32 = 0x0204,
    SDIF_UTF8 = 0x0301,
    SDIF_BYTE = 0x0401,
    SDIF_NO_TYPE = -1
} SDIF_MatrixDataType;

typedef enum {
    SDIF_FLOAT = 0,
    SDIF_INT = 1,
    SDIF_UINT = 2,
    SDIF_TEXT = 3,
    SDIF_ARBITRARY = 4
} SDIF_MatrixDataTypeHighOrder;


/* You must call this before any of the other SDIF procedures.  
   Returns 0 if OK; 1 if not */
int SDIF_Init(void);

/* SDIF_GetLastErrorCode and SDIF_GetLastErrorString --
   Returns the code, as defined above, or string representation of the
   most recent error encountered by the library. */
int SDIF_GetLastErrorCode(void);
char *SDIF_GetLastErrorString(void);

/* SDIF_OpenWrite --
   Opens "filename" for writing and writes the global SDIF header.  If
   there is a problem, NULL is returned.  A description of the problem
   will be available through the SDIF_GetLastError functions. */
FILE *SDIF_OpenWrite(const char *filename);

/* SDIF_CloseWrite --
   Returns the result of fclose() on "f". */
int SDIF_CloseWrite(FILE *f);

/* SDIF_OpenRead --
   Opens "filename" for reading and parses the header.  Sets the error
   values available through the SDIF_GetLastError functions and returns
   NULL if the header is wrong.  Advances the file pointer to the
   beginning of the first frame. */
FILE *SDIF_OpenRead(const char *filename);

/* SDIF_BeginRead --
   Same as for SDIF_OpenRead() except that it takes a FILE* already opened
   for binary reading. */
int SDIF_BeginRead(FILE *input);

/* SDIF_CloseRead --
   Returns the result of flcose() on "f". */
int SDIF_CloseRead(FILE *f);

/* SDIF_FillGlobalHeader --
   Writes "SDIF" into "h" and initializes the size and reserved members. */
void SDIF_FillGlobalHeader(SDIF_GlobalHeader *h);

/* SDIF_WriteGlobalHeader --
   Writes "h" to "f". */
int SDIF_WriteGlobalHeader(SDIF_GlobalHeader *h, FILE *f);

/* SDIF_ReadFrameHeader --
   Reads a frame header from "f" and writes it to "fh".  
   Returns 1 on success, 0 if EOF, or negative (and sets error code) if error */
int SDIF_ReadFrameHeader(SDIF_FrameHeader *fh, FILE *f);

/* SDIF_WriteFrameHeader --
   Writes "fh" to "f".  Returns 1 on success. */
int SDIF_WriteFrameHeader(SDIF_FrameHeader *fh, FILE *f);

/* SDIF_SkipFrame --

   Assuming that you just read an SDIF_FrameHeader and want to
   ignore the contents of the frame (e.g., because your program
   doesn't recognize its frameType), call this procedure to skip
   over the frame data, so you'll be ready to read the next
   SDIF_FrameHeader from the input.

   The arguments are a pointer to the SDIF_FrameHeader you just read
   (which includes the size count) and the open FILE *.  Returns 1 if
   successful; otherwise it sets the error string available through
   the SDIF_GetLastError functions and returns != 1. */
int SDIF_SkipFrame(SDIF_FrameHeader *head, FILE *f);

/* SDIF_ReadMatrixHeader --
   Fills "m" with the matrix header read from "f".  Returns 1 on success. */
int SDIF_ReadMatrixHeader(SDIF_MatrixHeader *m, FILE *f);

/* SDIF_WriteMatrixHeader --
   Writes "m" to "f".  Returns 1 on success. */
int SDIF_WriteMatrixHeader(SDIF_MatrixHeader *m, FILE *f);

/* SDIF_GetMatrixDataTypeSize --
   Returns the size in bytes of the data type indicated by "d" */
int SDIF_GetMatrixDataTypeSize(sdif_int32 d);

/* SDIF_GetMatrixDataSize --
   Returns the size in bytes of the matrix described by the given 
   SDIF_MatrixHeader, including possible byte padding. */
int SDIF_GetMatrixDataSize(SDIF_MatrixHeader *m);

/* SDIF_SkipMatrix --
   Assuming that you just read an SDIF_MatrixHeader and want to ignore the
   contents of this matrix (e.g., because your program doesn't recognize its
   matrixType), call this procedure to skip over the matrix data.  It will leave
   the file pointer pointing at the next matrix after the one you skipped, 
   or pointing at the next frame header if the one we skipped was the last one
   in the frame.  Returns 1 on success. */
int SDIF_SkipMatrix(SDIF_MatrixHeader *head, FILE *f);

/* SDIF_ReadMatrixData --
   Assuming that you just read an SDIF_MatrixHeader and want to read the
   matrix data itself into a block of memory that you've allocated, call
   this procedure to do so.  Handles big/little endian issues. Returns 1 
   on success. */
int SDIF_ReadMatrixData(void *putItHere, FILE *f, SDIF_MatrixHeader *head);


/* Each call to this procedure returns a unique stream ID: 1, 2, 3... */
sdif_int32 SDIF_UniqueStreamID(void);


/* SDIF_Str4Eq --
   Checks two 4-byte strings for equality.  Returns zero if strings
   differ, nonzero if they're the same. */
int SDIF_Str4Eq(const char *thisone, const char *thatone);


/* SDIF_Copy4Bytes --
   Copies 4 bytes (e.g., "SDIF") from a string to a 4-byte char array. */
void SDIF_Copy4Bytes(char *target, const char *string);



/* SDIF_Read and SDIF_Write --

   Abstract away big endian/little endian in reading/writing 1, 2, 4,
   and 8 byte words.

   These procedures are all just like fwrite() and fread() except that
   the size of the objects you're writing is determined by which function
   you call instead of an explicit argument.  Also, they do little-endian
   conversion when necessary. */

#if defined(__WIN32__) || defined(_WINDOWS)
#define LITTLE_ENDIAN  1
#else 
    /* Insert other checks for your architecture here if it's little endian. */
#endif

int SDIF_Write1(void *block, size_t n, FILE *f);
int SDIF_Write2(void *block, size_t n, FILE *f);
int SDIF_Write4(void *block, size_t n, FILE *f);
int SDIF_Write8(void *block, size_t n, FILE *f);

int SDIF_Read1(void *block, size_t n, FILE *f);
int SDIF_Read2(void *block, size_t n, FILE *f);
int SDIF_Read4(void *block, size_t n, FILE *f);
int SDIF_Read8(void *block, size_t n, FILE *f);


/*****************************************/
/* Some stuff for particular frame types */
/*****************************************/


/****** 1TRC ******/

typedef struct {
    sdif_float32 index, freq, amp, phase;
} SDIF_RowOf1TRC;

int SDIF_WriteRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f);
int SDIF_ReadRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f);
sdif_int32 SizeOf1TRCFrame(int numTracks);

/* Read a row of 1TRC data from an open file, writing results into pointers
   you pass as arguments.  Returns 0 if succesful, nonzero otherwise. */
int SDIF_Read1TRCVals(FILE *f,
                      sdif_float32 *indexp, sdif_float32 *freqp,
                      sdif_float32 *ampp, sdif_float32 *phasep);

/* Write a row of 1TRC data to an open file.  Returns 0 if succesful, nonzero
   otherwise. */
int SDIF_Write1TRCVals(FILE *f,
                       sdif_float32 index, sdif_float32 freq,
                       sdif_float32 amp, sdif_float32 phase);

/* How big does the size count need to be in a frame of 1TRC? */
/* (Assuming that the frame contains one matrix) */
sdif_int32 SDIF_SizeOf1TRCFrame(int numTracks);


/****** 1RES ******/

typedef struct {
   sdif_float32 freq, amp, decayrate, phase;
} SDIF_RowOf1RES;

sdif_int32 SDIF_SizeOf1RESFrame(int numResonances);
int SDIF_WriteRowOf1RES(SDIF_RowOf1RES *row, FILE *f);
int SDIF_ReadRowOf1RES(SDIF_RowOf1RES *row, FILE *f);


#ifdef __cplusplus
}
#endif

#endif /* __SDIF_H */
