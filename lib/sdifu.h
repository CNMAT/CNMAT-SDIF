/* 

  Copyright (c) 1998, 1999.  The Regents of the University of California
  (Regents).  All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Matt Wright, The Center for New Music and Audio Technologies,
University of California, Berkeley.

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

  sdifu.h --

  Utility procedures for working with the SDIF library.

  Sami Khoury, 11/5/1998
  updated 9/22/99 Matt Wright (moved merging out of util library)
  6/14/00 Matt Wright moved 1TRC synthesis out of util library

*/

#ifndef __SDIFU_H
#define __SDIFU_H


#include <stdio.h>

#include "sdif.h"


#ifdef __cplusplus
extern "C" {
#endif


/* update sdifu.c to reflect any changes made to these error values. */
#define ESDIFU_NONE 0
#define ESDIFU_SEE_ERRNO 1
#define ESDIFU_SEE_ESDIF 2
#define ESDIFU_BAD_SINUSOID_BIRTH 3
#define ESDIFU_BAD_SINUSOID_DEATH 4
#define ESDIFU_NUM_ERRORS 5


typedef struct streamIndexNode {
    sdif_int32 id;	/* the stream id which this node is representing. */
    char frameType[4];	/* the SDIFFrameHeader frameType for this stream id. */
    sdif_float64 first;	/* the time of the first frame in this stream. */
    sdif_float64 last;	/* the time of the last frame in this stream. */
    struct streamIndexNode *next;
} streamIndexNode;


/* 1VID stuff. */
#define VII_SUSTAIN ((sdif_uint32) 0)
#define VII_REFRESH ((sdif_uint32) 1)



#if 0
typedef int (SDIFFrameToSamplesFunc) (int num_samples, sdif_float32 *samples,
				      SDIFFrame *current, SDIFFrame *next,
				      void *state);
#endif


typedef char * (*SDIFU_SanityCheckFunc) (FILE *handle, SDIF_FrameHeader *sfh);



SDIFU_SanityCheckFunc SDIFU_GetSanityCheckFunc(char frame_type[4]);


/* SDIFU_GetIndexFromFile --

   Returns a linked list indexing the contents of "f".  Each SDIF stream
   contained in "f" will have its own streamIndexNode.  To access this
   index for each stream, simply traverse the list until next == NULL. */

streamIndexNode * SDIFU_GetIndexFromFile(const char *f);


/* SDIFU_GetIndexFromHandle --

   Same as SDIFU_GetIndexFromFile(), but takes a filehandle previously
   opened by OpenSDIFRead() or SDIF_BeginRead() as input. */

streamIndexNode * SDIFU_GetIndexFromHandle(FILE *f);


/* SDIFU_DestroyStreamIndex --

   Pass the return value of SDIFU_GetIndexFrom{File,Handle}() to this
   function to reclaim the storage used by the list of streamIndexNodes. */

void SDIFU_DestroyStreamIndex(streamIndexNode *index);


/* SDIFU_GetIndexStringFromFile --

   Returns a string indexing the contents of "f".  The string is
   allocated using malloc(), and the caller is responsible for
   free()'ing the returned storage.

   An example index string is:

	streamID:910124817,frameType:1PIC,startTime:0,stopTime:0.14;

   Names and values are separated by colons, name-value pairs are
   separated by commas, and a stream descriptions are separated by
   semicolons.

   SDIFU_GetIndexStringFromFile() returns NULL on error.  Call
   SDIFU_GetLastErrorAsString() for a string description of the error. */

char * SDIFU_GetIndexStringFromFile(const char *f);


/* SDIFU_GetIndexStringFromHandle --

   Same as SDIFU_GetIndexStringFromFile(), but takes a filehandle
   previously opened by OpenSDIFRead() or SDIF_BeginRead() as input. */

char * SDIFU_GetIndexStringFromHandle(FILE *f);




/* SDIFU_SamplesToTDSFrame --

   Converts "samples" into an SDIF TDS frame and writes it "outfp".

   "samples_rate" is the sample width, in bits.  Just kidding.

   "samples" is an array of "num_channels" arrays, each with
   "num_samples_per_channel" floats in the range [-1, 1]. */

int SDIFU_SamplesToTDSFrame(FILE *outfp,
			    sdif_int32 stream_id,
			    sdif_float32 time_stamp,
			    sdif_float32 sample_rate,
			    int num_channels,
			    int num_samples_per_channel,
			    sdif_float32 **samples);


/* SDIFU_ImageToVIDFrame --

   Converts "current_image", which should be an array of "height"*"width"
   sdif_int32's into an SDIF VID frame, using "previous_image" to generate
   the sustain and refresh blocks.  "previous_image" can be NULL, in which
   case all of "current_image" is written out. */

int SDIFU_ImageToVIDFrame(FILE *outfp,
			  sdif_int32 stream_id,
			  sdif_float32 time_stamp,
			  sdif_int32 height, sdif_int32 width,
			  sdif_uint32 *previous_image,
			  sdif_uint32 *current_image);



/* SDIFU_GetLastErrorCode and SDIFU_GetLastErrorString --

   Returns the code, as defined above, or string representation of the
   most recent error encoutered by the library. */

int SDIFU_GetLastErrorCode(void);
char * SDIFU_GetLastErrorString(void);


#ifdef __cplusplus
}
#endif


#endif  /* __SDIFU_H */
