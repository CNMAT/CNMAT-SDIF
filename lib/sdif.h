/*
 * Copyright (c) 1996, 1997 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 sdif.h

 API for formatting data into SDIF

 Matt Wright, 12/4/96
*/

/* You need to #include <stdio.h> before this file, for FILE *. */

/* Create 4-byte int and 4-byte and 8-byte float typedefs. */

#define SGI
#ifdef SGI
    typedef int int4;
    typedef float float4;
    typedef double float8;
    typedef long long int int8;
#endif

/***********************************************************/
/*********** The header for the entire SDIF data ***********/
/***********************************************************/

struct SDIFGlobalHeader {
    char FORM[4];   /* Should be 'F', 'O', 'R', 'M' */
    int4 size;	    /* # bytes of data, not including form or size. */
    char SDIF[4];   /* Should be 'S', 'D', 'I', 'F' */
};

#define SIZE_UNKNOWN 0xffffffff	 /* Value for size when unknown or not given */

/* Writes 'FORM', the given size, and 'SDIF' into the given SDIFHeader
   struct. */
void FillSDIFGlobalHeader(struct SDIFGlobalHeader *h, int4 size);

/* Utility to open a new SDIF file + write the global header. */
FILE *OpenSDIFWrite(char *filename, int size);
int CloseSDIFWrite(FILE *f);

/* Utility to open an SDIF file for reading and parse the header.
   Prints an error message (to stderr) and returns NULL if the header
   is wrong.  Advances the file pointer to the beginning of the first
   frame.  If sizep is non-NULL, the size count from the SDIF header 
   is written into *sizep. */
FILE *OpenSDIFRead(char *filename, int *sizep);
int CloseSDIFRead(FILE *f);


/***********************************************************/
/********** The header for each frame of SDIF data *********/
/***********************************************************/

struct SDIFFrameHeader {
    char frameType[4];	/* Should be a registered frame type */
    int4 size;		/* # bytes in this frame, not including
			   frameType or size */
    float8 time;	/* Time corresponding to data chunk */
    int8 ID;		/* Frames that go together have the same ID */
};

/* Return a unique ID number based on number of microseconds since 1970. */
int8 GenUniqueSDIFFrameID(void);


/* Assuming that you just read an SDIFFrameHeader from an open file and don't
   want to deal with it (e.g., because your program doesn't recognize its
   frameType), call this procedure to skip over the frame data, so you'll be
   ready to read the next SDIFFrameHeader from the file.

   The arguments are a pointer to the SDIFFrameHeader you just read (which
   includes the size count) and the open FILE *.  Returns 0 if successful;
   otherwise it prints an error message to stderr and returns non-zero.*/

int SkipSDIFFrame(struct SDIFFrameHeader *head, FILE *f);


/***********************************************************/
/******************** Matrix Data **************************/
/***********************************************************/

typedef struct {
    int4 numRows;
    int4 numCols;
} MatrixSize;

/***********************************************************/
/******************* Utility Procedures ********************/
/***********************************************************/


/* Trivial utility procedure to copy 4 bytes (e.g., "FORM") from a string to a
   4-byte char array */
void Copy4Bytes(char *target, const char *string);


/* Trivial utility function to check two 4-byte strings for equality.  
   Returns zero if strings differ, nonzero if they're the same. */
int str4eq(const char *this, const char *that);



/***********************************************************/
/********* Some stuff for particular frame types: **********/
/***********************************************************/

typedef struct {
    float index; float freq; float phase; float amp;
} RowOf1TRC;


/* How big does the size count need to be in a frame of 1TRC? */
int4 SizeOf1TRCFrame(int numTracks);
