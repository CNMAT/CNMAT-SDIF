/* 
Copyright (c) 2004.  The Regents of the University of
California (Regents).  All Rights Reserved.

Written by Ben "Jacobs" and Matt Wright, The Center for New Music and Audio
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

/* sdif-buf.h

  Public interface to functions for manipulating generic SDIF data in memory:
  - read an entire SDIF stream into memory, from an SDIF file on disk
    (one streamID per buffer instance)
  - traverse frame list in memory
  - retrieve frame from memory, by specified time / search direction
  - insert/replace individual frames in memory
  - write entire contents of memory (a single SDIF stream) to an SDIF file
  - dispose all SDIF data from memory
  
  Future features:
  - delete individual frames from memory
  - interface to matrix interpolation code
  - support for retrieving frames on demand (e.g. from disk or network)

  authors: Ben "Jacobs" and Matt Wright
  2004/03/19 (0.1.0) Refactored non max-specific code from SDIF-buffer.c (0.7.1) - bj
  2004/06/23 (0.1.1) cleanup - bj

  NOTES
  
  Return value/error message conventions: same as sdif.[ch], except
  for the constructor/lookup procedures, which return a pointer to an object
  or zero if out of memory / not found. 

  This SDIF library is neither reentrant nor thread-safe.

  Assume user has already included "sdif.h" and "sdif-mem.h"
*/


/**********************************/
/*                                */
/* public types, enums, constants */
/*                                */
/**********************************/

/* different modes of choosing which stream to read from an SDIF file 
*/
typedef enum 
{
	ESDIF_WHICH_STREAM_NUMBER,
	ESDIF_WHICH_ONLY_STREAM,
	ESDIF_WHICH_FIRST_STREAM,
	ESDIF_WHICH_ONLY_STREAM_TYPE,
	ESDIF_WHICH_FIRST_STREAM_TYPE,
	ESDIF_WHICH_END_OF_ENUM
} SDIFwhichStreamMode;

/* different modes of searching for a frame or matrix
*/
typedef enum 
{
  ESDIF_SEARCH_BACKWARDS = -1,  //  return frame at or just before the requested time, or NULL
  ESDIF_SEARCH_EXACT = 0,       //  return frame exactly at requested time, or NULL
  ESDIF_SEARCH_FORWARDS = 1,    //  return frame at or just after the requested time, or NULL
	ESDIF_SEARCH_END_OF_ENUM
} SDIFsearchMode;

/* different frame insertion modes
*/
typedef enum 
{
  ESDIF_INSERT_REPLACE,         //  if a frame exists at requested time, replace it
  ESDIF_INSERT_DONT_REPLACE     //  if a frame exists at requested time, fail
} SDIFinsertMode;

/*  specify desired action to handle IEEE NaN values found in source data
*/
typedef enum
{
  ESDIF_NAN_ACTION_ACCEPT = 0,   //  return the NaN without special action
  ESDIF_NAN_ACTION_KEEP_LOOKING, //  automatically extend search to next (or prev) matrix in list
  ESDIF_NAN_ACTION_SET_ZERO,     //  implicitly replace NaN with 0
  ESDIF_NAN_ACTION_FAIL,         //  give up immediately, return ESDIF_FAIL_ON_NAN
  ESDIF_NAN_ACTION_END
} SDIFactionOnNaN;

/*  return type to indicate result of NaN test on a value
*/
typedef enum
{
  ESDIF_NAN_TYPE_NO = 0,         //  value isn't a NaN
  ESDIF_NAN_TYPE_YES,            //  value is a NaN
  ESDIF_NAN_TYPE_END
} SDIFnanType;


/******************************************************/
/*                                                    */
/* public interface to SDIFbuf_Buffer instance fields */
/*                                                    */
/******************************************************/

typedef struct _SDIFbufBufferStruct *SDIFbuf_Buffer;
typedef struct _SDIFbufBufferStruct
{
  // SDIFbuf_Buffer public instance data 
	SDIFbuf_Buffer prev, next;    // doubly linked list of all SDIFbuf_Buffer instances
 	void *internal;               // private stuff (defined in "sdif-buf-private.h")
} SDIFbufBufferStruct;


/**********************************************/
/*                                            */
/* public interface to SDIFbuf_Buffer methods */
/*                                            */
/**********************************************/

/*----------------------*/
/* Class Initialization */
/*----------------------*/

/* SDIFbuf_Init --
   You must call this before any of the other procedures in this library. Be sure to
   call SDIFmem_Init() first, to set up the user-provided memory alloc/free functions.
*/
SDIFresult SDIFbuf_Init(void);


/*-------------- */
/* Class Methods */
/*---------------*/

/* SDIFbuf_Create (create an SDIFbuf_Buffer instance) --
   Returns NULL if out of memory.
*/
SDIFbuf_Buffer SDIFbuf_Create(void);


/* SDIFbuf_Free (free an SDIFbuf_Buffer instance)
*/
void SDIFbuf_Free(SDIFbuf_Buffer buf);


/* SDIFbuf_FreeAll (free all SDIFbuf_Buffer instances in list) --
*/
void SDIFbuf_FreeAll(void);


/* SDIFbuf_GetFirstBuffer (return first SDIFbuf_Buffer instance in list) --
   Returns NULL if no instances in list.
*/
SDIFbuf_Buffer SDIFbuf_GetFirstBuffer(void);


/* SDIFbuf_GetPrevFrame (return previous frame in list) --
   Returns NULL if no preceding frame.
*/
SDIFmem_Frame SDIFbuf_GetPrevFrame(SDIFmem_Frame f);


/* SDIFbuf_GetPrevFrame (return next frame in list) --
   Returns NULL if no next frame.
*/
SDIFmem_Frame SDIFbuf_GetNextFrame(SDIFmem_Frame f);


/* SDIFbuf_GetFrameNearby 
   (return frame closest to requested time, searching from given frame) --
*/
SDIFmem_Frame SDIFbuf_GetFrameNearby(SDIFmem_Frame f, 
                                     sdif_float64 time, 
                                     SDIFsearchMode direction
                                     );


/* SDIFbuf_GetMatrixInFrame (return instance of specified matrix in given frame) --
   Returns NULL if matrix not found.
*/
SDIFmem_Matrix SDIFbuf_GetMatrixInFrame(SDIFmem_Frame f, const char *matrixType);


/* SDIFbuf_GetPrevMatrix (return previous instance of specified matrix in frame list) --
   Returns NULL if no preceding instance found.
*/
SDIFmem_Matrix SDIFbuf_GetPrevMatrix(SDIFmem_Frame f,
                                     const char *matrixType,
                                     sdif_float64 *timeFound
                                     );


/* SDIFbuf_GetNextMatrix (return next instance of specified matrix in frame list) --
   Returns NULL if no following instance found.
*/
SDIFmem_Matrix SDIFbuf_GetNextMatrix(SDIFmem_Frame f,
                                     const char *matrixType,
                                     sdif_float64 *timeFound
                                     );


/* SDIFbuf_GetMatrixNearby
   (return matrix instance closest to requested time, searching from given frame) --
   Returns NULL if no matrix found.
*/
SDIFmem_Matrix SDIFbuf_GetMatrixNearby(SDIFmem_Frame f, 
                                       const char *matrixType, 
                                       sdif_float64 time, 
                                       SDIFsearchMode direction,
                                       sdif_float64 *timeFound
                                       );


/* SDIFbuf_GetValueInFrame (obtain specified matrix cell in given frame) --
   Returns ESDIF_NOT_AVAILABLE if matrix not found.
   Might return ESDIF_FAILING_ON_NAN or ESDIF_NOT_AVAIL according to specified action on NaN.
*/
SDIFresult SDIFbuf_GetValueInFrame(SDIFmem_Frame f,
                                   const char *matrixType,
                                   sdif_int32 column,
                                   sdif_int32 row,
                                   SDIFactionOnNaN actionNaN,
                                   sdif_float64 *valueFound
                                   );


/* SDIFbuf_GetValueInMatrix (obtain specified cell in given matrix) --
   Might return ESDIF_FAILING_ON_NAN or ESDIF_NOT_AVAIL according to specified action on NaN.
*/
SDIFresult SDIFbuf_GetValueInMatrix(SDIFmem_Matrix m,
                                    sdif_int32 column,
                                    sdif_int32 row,
                                    SDIFactionOnNaN actionNaN,
                                    sdif_float64 *valueFound
                                    );


/* SDIFbuf_GetPrevValue 
   (obtain closest previous instance of specified matrix cell value in frame list) --
   Returns NULL if no preceding instance found.
*/
SDIFresult SDIFbuf_GetPrevValue(SDIFmem_Frame f, 
                                const char* matrixType,
                                sdif_int32 column,
                                sdif_int32 row,
                                sdif_float64 time,
                                SDIFactionOnNaN actionNaN,
                                sdif_float64 *valueFound,
                                sdif_float64 *timeFound
                                );


/* SDIFbuf_GetNextValue 
   (obtain closest following instance of specified matrix cell value in frame list) --
   Returns NULL if no following instance found.
*/
SDIFresult SDIFbuf_GetNextValue(SDIFmem_Frame f, 
                                const char* matrixType,
                                sdif_int32 column,
                                sdif_int32 row,
                                sdif_float64 time,
                                SDIFactionOnNaN actionNaN,
                                sdif_float64 *valueFound,
                                sdif_float64 *timeFound
                                );


/* SDIFbuf_GetValueNearby
   (obtain a cell value from matrix instance closest to requested time, searching from given frame) --
   Returns ESDIF_NOT_AVAILABLE if no suitable matrix cell found.
*/
SDIFresult SDIFbuf_GetValueNearby(SDIFmem_Frame f, 
                                  const char* matrixType,
                                  sdif_int32 column,
                                  sdif_int32 row,
                                  sdif_float64 time,
                                  SDIFsearchMode direction,
                                  SDIFactionOnNaN actionNaN,
                                  sdif_float64 *valueFound,
                                  sdif_float64 *timeFound
                                  );


/* SDIFbuf_GetNeighborValues
   (obtain cell values from matrix instances close to requested time, searching from given frame) --
   NOTE: attempts to get an equal number of values before + after the requested time,
   but will accept more values on either side if that's all we can get.
   Sets countOut to indicate the number of values actually found (might be 0).
*/
SDIFresult SDIFbuf_GetNeighborValues(SDIFmem_Frame nearbyFrame,
                                     const char* matrixType,
                                     sdif_int32 column,
                                     sdif_int32 row,
                                     sdif_float64 time,
                                     sdif_int32 count,
                                     SDIFactionOnNaN actionOnNaN,
                                     sdif_float64 *t,
                                     sdif_float64 *v,
                                     sdif_int32 *countOut
                                     );


/*----------------- */
/* Instance Methods */
/*------------------*/

/* SDIFbuf_Clear (release SDIF data contained within an SDIFbuf_Buffer instance)
*/
void SDIFbuf_Clear(SDIFbuf_Buffer buf);


/* SDIFbuf_ReadStream (read specified SDIF file/stream into an SDIFbuf_Buffer instance)
   file specified by filename/path
   NOTE: in base code, only mode == STREAM_NUMBER is implemented
*/
SDIFresult SDIFbuf_ReadStream(SDIFbuf_Buffer b, 
                              char *filename, 
                              SDIFwhichStreamMode mode, 
                              sdif_int32 arg
                              );


/* SDIFbuf_ReadStreamFromOpenFile
   (read specified SDIF file/stream into an SDIFbuf_Buffer instance)
   file specified by open filehandle
   NOTE: in base code, only mode == STREAM_NUMBER is implemented
*/
SDIFresult SDIFbuf_ReadStreamFromOpenFile(SDIFbuf_Buffer b, 
					  FILE *f, 
					  SDIFwhichStreamMode mode, 
					  sdif_int32 arg
					  );


/* SDIFbuf_GetFirstFrame (return first frame of an SDIFbuf_Buffer instance)
*/
SDIFmem_Frame SDIFbuf_GetFirstFrame(SDIFbuf_Buffer b);


/* SDIFbuf_GetLastFrame (return first frame of an SDIFbuf_Buffer instance)
*/
SDIFmem_Frame SDIFbuf_GetLastFrame(SDIFbuf_Buffer b);


/* SDIFbuf_GetFrame (return frame closest to specified time in an SDIFbuf_Buffer instance)
*/
SDIFmem_Frame SDIFbuf_GetFrame(SDIFbuf_Buffer b, sdif_float64 time, SDIFsearchMode direction);


/* SDIFbuf_InsertFrame (insert frame into an SDIFbuf_Buffer instance)
*/
SDIFresult SDIFbuf_InsertFrame(SDIFbuf_Buffer b, SDIFmem_Frame newf, SDIFinsertMode replace);


/* SDIFbuf_GetMatrix
   (return matrix instance closest to specified time in an SDIFbuf_Buffer instance)
*/
SDIFmem_Matrix SDIFbuf_GetMatrix(SDIFbuf_Buffer b, 
                                 const char *matrixType, 
                                 sdif_float64 time, 
                                 SDIFsearchMode direction,
                                 sdif_float64 *timeFound
                                 );


/* SDIFbuf_GetValue
   (obtain a cell value from matrix instance closest to requested time in an SDIFbuf_Buffer instance) --
   Returns ESDIF_NOT_AVAILABLE if no suitable matrix cell found.
*/
SDIFresult SDIFbuf_GetValue(SDIFbuf_Buffer b,
                            const char* matrixType,
                            sdif_int32 column,
                            sdif_int32 row,
                            sdif_float64 time,
                            SDIFsearchMode direction,
                            SDIFactionOnNaN actionNaN,
                            sdif_float64 *valueFound,
                            sdif_float64 *timeFound
                            );

/* SDIFbuf_GetMinTime (get timestamp of earliest frame in an SDIFbuf_Buffer instance)
*/
SDIFresult SDIFbuf_GetMinTime(SDIFbuf_Buffer b, sdif_float64 *time);


/* SDIFbuf_GetMaxTime (get timestamp of latest frame in an SDIFbuf_Buffer instance)
*/
SDIFresult SDIFbuf_GetMaxTime(SDIFbuf_Buffer b, sdif_float64 *time);


/* SDIFbuf_GetMinMatrixTime 
   (get timestamp of earliest frame in an SDIFbuf_Buffer instance containing the specified matrix)
*/
SDIFresult SDIFbuf_GetMinMatrixTime(SDIFbuf_Buffer b, const char *matrixType, sdif_float64 *time);


/* SDIFbuf_TimeShiftToZero 
   (adjust time of all buffer frames so that first frame is at time == 0)
*/
SDIFresult SDIFbuf_GetMaxMatrixTime(SDIFbuf_Buffer b, const char *matrixType, sdif_float64 *time);


/* SDIFbuf_GetMaxMatrixTime 
   (get timestamp of latest frame in an SDIFbuf_Buffer instance containing the specified matrix)
*/
SDIFresult SDIFbuf_TimeShiftToZero(SDIFbuf_Buffer b);


/* SDIFbuf_GetMaxNumColumns
   Of all matrices in b of type matrixType, what's the largest number of columns that any contains?
   Returns ESDIF_END_OF_DATA if buffer is empty, ESDIF_BAD_MATRIX_DATA_TYPE if buffer has no matrices
   of the given type.
 */
SDIFresult SDIFbuf_GetMaxNumColumns(SDIFbuf_Buffer b, const char *matrixType, sdif_int32 *result);


/* SDIFbuf_GetColumnRanges
   What are the minimum and maximum values for each column across all rows, in all matrices
   of the given type, across the entire stream?  Checks only the given number of columns.  */

SDIFresult SDIFbuf_GetColumnRanges(SDIFbuf_Buffer b, const char *matrixType, sdif_int32 ncols, 
								   sdif_float64 *column_mins, sdif_float64 *column_maxes);
								   

/* SDIFbuf_GetColumnRange
    Same thing, but for a given column instead of all the first n columns */							   
SDIFresult SDIFbuf_GetColumnRange(SDIFbuf_Buffer b, const char *matrixType, sdif_int32 column, 
								   sdif_float64 *min, sdif_float64 *max);
