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

 SDIF spec: http://www.cnmat.berkeley.edu/SDIF

 Matt Wright, 12/4/96
 5/12/97 version 1.1 by Matt and Amar, incorporating little-endian issues
 1/12/98 version 1.2 by Amar, conforms to revised SDIF spec

*/

/* You need to #include <stdio.h> before this file, for FILE *. */


#ifndef __SDIF_H
#define __SDIF_H

/* Create 4-byte and 8-byte int and float typedefs. */

#ifdef __sgi
    typedef int int32;
    typedef float float32;
    typedef double float64;
#elif defined(__WIN32__) || defined(_WINDOWS)
    #ifndef _WINDOWS_
    #include <windows.h>
    #endif 
    typedef int	int32;
    typedef float float32;
    typedef double float64;
#elif defined(__LINUX__)
    typedef int int32;
    typedef float float32;
    typedef double float64;
#else
    /* These won't necessarily be the right size on any conceivable
       platform, so you may need to change them by hand.  All calls to
       OpenSDIFRead() and OpenSDIFWrite() perform a sanity check of the
       sizes of these types, so if they're wrong you'll find out about it. */

    typedef long int32;
    typedef float float32;
    typedef double float64;

#endif

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************/
/*********** The header for the entire SDIF data ***********/
/***********************************************************/

struct SDIFGlobalHeader {
    char  SDIF[4];     /* Should be 'S', 'D', 'I', 'F' */
    int32 size;	       /* # of reserved bytes, not including form or size. */
    char  reserved[8]; /* reserved for future use */
};



/* Writes 'SDIF' into the given SDIFHeader
   struct. */
void FillSDIFGlobalHeader(struct SDIFGlobalHeader *h);
int  WriteSDIFGlobalHeader(struct SDIFGlobalHeader *h, FILE *f);


/* Utility to open a new SDIF file + write the global header. */
FILE *OpenSDIFWrite(const char *filename);
int CloseSDIFWrite(FILE *f);

/* Utility to open an SDIF file for reading and parse the header.
   Prints an error message (to stderr) and returns NULL if the header
   is wrong.  Advances the file pointer to the beginning of the first
   frame.  If sizep is non-NULL, the size count from the SDIF header
   is written into *sizep. */
FILE *OpenSDIFRead(const char *filename);
int CloseSDIFRead(FILE *f);


/***********************************************************/
/********** The header for each frame of SDIF data *********/
/***********************************************************/

struct SDIFFrameHeader {
    char frameType[4];	 /* Should be a registered frame type */
    int32 size;		 /* # bytes in this frame, not including
			    frameType or size */
    float64 time;	 /* Time corresponding to data chunk */
    int32   streamID;		 /* Frames that go together have the same ID */
    int32   matrixCount;  /* Number of matrices in frame */
};

int WriteSDIFFrameHeader(struct SDIFFrameHeader *fh, FILE *f);
int ReadSDIFFrameHeader(struct SDIFFrameHeader *fh, FILE *f);


/* Return a unique streamID number.  (In this implementation, the number is based on
number of microseconds since 1970.) */
int32 GenUniqueSDIFFrameID(void);


/* Assuming that you just read an SDIFFrameHeader from an open file and don't
   want to deal with it (e.g., because your program doesn't recognize its
   frameType), call this procedure to skip over the frame data, so you'll be
   ready to read the next SDIFFrameHeader from the file.

   The arguments are a pointer to the SDIFFrameHeader you just read (which
   includes the size count) and the open FILE *.  Returns 0 if successful;
   otherwise it prints an error message to stderr and returns non-zero.*/

int SkipSDIFFrame(struct SDIFFrameHeader *head, FILE *f);


/***********************************************************/
/********* Some stuff for particular frame types: **********/
/***********************************************************/


/************* 1TRC ****************/

typedef struct {
    float32 index; float32 amp; float32 freq; float32 phase;
} RowOf1TRC;

int WriteRowOf1TRC (RowOf1TRC *row, FILE *f);
int ReadRowOf1TRC (RowOf1TRC *row, FILE *f);


/* Read a row of 1TRC data from an open file, writing results into pointers
   you pass as arguments.  Returns 0 if succesful, nonzero otherwise. */
int Read1TRCVals(FILE *f, float32 *indexp, float32 *freqp, float32 *phasep,
		 float32 *ampp);

/* Write a row of 1TRC data to an open file.  Returns 0 if succesful, nonzero
   otherwise. */
int Write1TRCVals(FILE *f, float32 index, float32 freq, float32 phase, float32 amp);


/* How big does the size count need to be in a frame of 1TRC? */
/* (Assuming that the frame contains one matrix) */
int32 SizeOf1TRCFrame(int numTracks);

/************* 1RES ****************/

typedef struct {
   float32 freq, amp, bandwidth, phase;
} RowOf1RES;

int32 SizeOf1RESFrame(int numResonances);
int WriteRowOf1RES (RowOf1RES *row, FILE *f);
int ReadRowOf1RES (RowOf1RES *row, FILE *f);


/***********************************************************/
/******************** Matrix Data **************************/
/***********************************************************/

typedef struct {
    char  matrixType[4];
    int32 matrixDataType;
    int32 rowCount;
    int32 columnCount;
} MatrixHeader;

int WriteMatrixHeader(MatrixHeader *m, FILE *f);
int ReadMatrixHeader (MatrixHeader *m, FILE *f);

/* codes for data types used in matrices: float32, float64 */

typedef enum {SDIF_FLOAT32 = 1, SDIF_FLOAT64 = 2} SDIFDataType;

/***********************************************************/
/******************* Utility Procedures ********************/
/***********************************************************/

/* Trivial utility function to check two 4-byte strings for equality.
   Returns zero if strings differ, nonzero if they're the same. */
int str4eq(const char *thisone, const char *thatone);

/* Trivial utility procedure to copy 4 bytes (e.g., "FORM") from a
   string to a 4-byte char array */
void Copy4Bytes(char *target, const char *string);


/***********************************************************/
/********* Abstract away big endian/little endian in *******/
/********* reading/writing 1, 2, 4, and 8 byte words. ******/
/***********************************************************/

#if defined(__WIN32__) || defined(_WINDOWS)
#define LITTLE_ENDIAN  1
#else 
    /* Insert other checks for your architecture here if it's little endian. */
#endif

/* These procedures are all just like fwrite() except that the size of the
   objects you're writing is determined by which function you call 
   instead of an explicit argument.  Also, they do little-endian
   conversion when necessary. */

int write1(void *block, size_t n, FILE *f);
int write2(void *block, size_t n, FILE *f);
int write4(void *block, size_t n, FILE *f);
int write8(void *block, size_t n, FILE *f);


/* Same, but like fread(). */

int read1(void *block, size_t n, FILE *f);
int read2(void *block, size_t n, FILE *f);
int read4(void *block, size_t n, FILE *f);
int read8(void *block, size_t n, FILE *f);

#ifdef __cplusplus
}
#endif

#endif /* __SDIF_H */

