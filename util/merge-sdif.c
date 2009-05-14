/*
Copyright (c) 1998.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

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


/* _sdif --

  A program to interleave multiple SDIF files into a single file.

  Sami Khoury
  11/6/1998

  Updated 9/22/99 by Matt Wright - fixed stream renumbering
  Updated 10/12/99 by Matt Wright - use new SDIF library

*/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sdif.h"


/* SDIFU_MergeHandles --

   Merges the SDIF data from "handles" into "outfp".  Each handle in
   "handles" must have already been opened with a call to OpenSDIFRead()
   or SDIF_BeginRead(), and "outfp" must have already been opened with
   a call to OpenSDIFWrite().  Returns 1 on succes. */

int SDIFU_MergeHandles(FILE *outfp, int num_handles, FILE **handles);



static void InitStreamRenumbering(void *(*MemoryAllocator)(int numBytes));
static sdif_int32 chooseOutStreamID(int inFileIndex, sdif_int32 inStreamID);

int num_inputFiles;


int
main(int argc, char **argv) {
    SDIFresult r;
    unsigned int i;
    char *outfile = NULL;
    FILE *outfp = NULL;
    FILE **input_files = NULL;

    /* check for proper number of arguments. */
    if (argc < 4) {
	fprintf(stderr, "Usage: %s <infile 1> <infile 2> ... <outfile>\n",
		argv[0]);
	return 1;
    }

    /* make sure that the output file does not already exist. */
    outfile = argv[argc-1];
    if ((outfp = fopen(outfile, "rb")) != NULL) {
	fprintf(stderr, "%s: file \"%s\" already exists.  Exiting...\n",
		argv[0], outfile);
	return 1;
    }

    /* make sure that each input file can be opened. */
    num_inputFiles = argc - 2;
    input_files = (FILE **) malloc(num_inputFiles * sizeof(FILE *));
    for (i=0; i < num_inputFiles; i++) {
	if (r = SDIF_OpenRead(argv[i+1], &(input_files[i]))) {
	    fprintf(stderr, "%s: error opening \"%s\": %s.  Exiting...\n",
		    argv[0], argv[i+1], SDIF_GetErrorString(r));
	    return 1;
	}
    }

    /* open the output file. */
    if (r = SDIF_OpenWrite(outfile, &outfp)) {
	fprintf(stderr, "%s: error opening \"%s\": %s.  Exiting...\n",
		argv[0], outfile, SDIF_GetErrorString(r));
	return 1;
    }

    printf("Merging %d SDIF files...\n", num_inputFiles);


    InitStreamRenumbering((void *(*)(int))malloc);

    /* do the merge. */
    if (SDIFU_MergeHandles(outfp, num_inputFiles, input_files) != 1) {
	fprintf(stderr, "%s: something bad happened.\n", argv[0]);
    }

    SDIF_CloseWrite(outfp);
    return 0;
}


int
SDIFU_MergeHandles(FILE *outfp, int num_handles, FILE **handles) {
    int i, idx, num_remaining = num_handles;
    SDIF_FrameHeader *headers;
    int *eof;
    SDIFresult r;
    sdif_float64 earliestTime;
    char *frameBuf;
    int frameBufSize;
    int frameSize;

    assert(num_handles >= 0);

    frameBufSize = 0;

    headers = malloc(num_handles*sizeof(*headers));
    eof = malloc(num_handles*sizeof(*eof));

    /* First, read the frame header from each input file */
    for (i = 0; i < num_handles; ++i) {
	eof[i] = 0;
	r = SDIF_ReadFrameHeader(&(headers[i]), handles[i]);
	if (r == ESDIF_END_OF_DATA) {
	    /* Weird; an empty file.  Oh well. */
	    SDIF_CloseRead(handles[i]);
	    eof[i] = 1;
	    --num_remaining;
	} else if (r) {
	    /* Problem: bomb out */
	    fprintf(stderr, "problem reading frame header: %s\n", 
		    SDIF_GetErrorString(r));
	    return 0;
	}
    }
	

    /* repeatedly find the most imminent frame among all the input
       files and write it and its data to the output file until all
       the input files have been exhausted. */

    while (num_remaining > 0) {
	/* find which file's current stream has the lowest time tag */
	idx = -1;
	for (i=0; i < num_handles; i++) {
	    if (eof[i]) continue;

	    if ((idx == -1) || (headers[i].time < earliestTime)) {
		idx = i;
		earliestTime = headers[i].time;
	    }
	}

	if (idx == -1) {
	    fprintf(stderr, "merge-sdif: internal error.  idx shouldn't be -1\n");
	    return 0;
	}



	/* (Possibly) change the streamID */
	headers[idx].streamID = chooseOutStreamID(idx, headers[idx].streamID);
	printf("this one: %d\n", headers[idx].streamID);

	/* Write this frame to the output with the new streamID. */

	if (r = SDIF_WriteFrameHeader(&(headers[idx]), outfp)) {
	    fprintf(stderr, "merge-sdif: write error: %s\n",
		    SDIF_GetErrorString(r));
	    return 0;
	}
	frameSize = headers[idx].size-16;
	if (frameBufSize < frameSize) {
	    if (frameBufSize > 0)  free(frameBuf);
	    frameBuf = malloc(frameSize);
	    frameBufSize = frameSize;
	}
	fread(frameBuf, 1, frameSize, handles[idx]);
	fwrite(frameBuf, 1, frameSize, outfp);



	/* Read the next header from this file */
	r = SDIF_ReadFrameHeader(&(headers[idx]), handles[idx]);
        if (r == ESDIF_END_OF_DATA) {
            /* End of file. */
            SDIF_CloseRead(handles[idx]);
            eof[idx] = 1;
            --num_remaining;
	}
    }

    return 1;
}


typedef struct {
	int inStream[128];
	int outStream[128];
	int numStreams;
} t_inputFile;

t_inputFile *inputFiles;
int positiveStreamCount, negativeStreamCount;

static void InitStreamRenumbering(void *(*MemoryAllocator)(int numBytes)) {
	// don't need the memory allocator--just there so we have the 
	// same signature as the old functions
	inputFiles = (t_inputFile *)malloc(num_inputFiles * sizeof(t_inputFile));
	positiveStreamCount = negativeStreamCount = 0;
	int i;
	for(i = 0; i < num_inputFiles; i++){
		inputFiles[i].numStreams = 0;
	}
}

static sdif_int32 chooseOutStreamID(int inFileIndex, sdif_int32 inStreamID){
	t_inputFile *inFile = &(inputFiles[inFileIndex]);
	int i;
	for(i = 0; i < inFile->numStreams; i++){
		if(inFile->inStream[i] == inStreamID){
			printf("already seen this stream %d %d->%d\n", inFileIndex, inStreamID, inFile->outStream[i]);
			return inFile->outStream[i];
		}
	}
	inFile->inStream[inFile->numStreams] = inStreamID;
	inFile->outStream[inFile->numStreams] = inStreamID < 0 ? --negativeStreamCount : ++positiveStreamCount;
	printf("new stream: file num: %d, in: %d, out: %d,  num: %d\n", inFileIndex, inFile->inStream[inFile->numStreams], inFile->outStream[inFile->numStreams], inFile->numStreams + 1);

	return inFile->outStream[inFile->numStreams++];
}


#ifdef EASY
static void InitStreamRenumbering(void *(*MemoryAllocator)(int numBytes)) {
}

static sdif_int32 chooseOutStreamID(int inFileIndex, sdif_int32 inStreamID) {
    return inStreamID;
}
#elif HARD

/* Data structure for renumbering Stream IDs (if necessary) when merging.
   Here's the strategy: we'll keep track of each stream in any of the input
   files.  Each one has a file that it came from, an original streamID,
   and an output streamID.  Ideally all the original streamIDs will be unique
   and all the output streamIDs will be unchanged.  But if any two input streams
   (from different files) have the same original stream IDs we have to rename
   one of them.

   We'll have two hash tables, one indexed by original streamID and one
   indexed by ouput streamID.  Each time we see an input streamID we'll
   look it up in the first table to tell us if we've already seen it or
   a "conflicting" stream with the same streamID from another file.

   To resolve conflicts, we need to use the table indexed by output streamID,
   to try each candidate output streamID until we find one that's not taken.

*/

typedef struct streamRenameRecordStruct {
    int inFileIndex;      /* Index of input file in **handles array */
    sdif_int32 inStreamID; /* Original streamID in input file */
    sdif_int32 outStreamID; /* streamID of this stream in output file */
    struct streamRenameRecordStruct *nextIn;   /* hash bucket on inID */
    struct streamRenameRecordStruct *nextOut;  /* hash bucket on outID */
} streamRenameRecord;

#define TABLE_SIZE 127 /* A reasonable prime number */

static streamRenameRecord *inTable[TABLE_SIZE];
static streamRenameRecord *outTable[TABLE_SIZE];

static void *(*my_malloc)(int numBytes);
static void DoubleInsert(int inFileIndex,sdif_int32 inStreamID, sdif_int32 outStreamID);
static sdif_int32 FindUnusedStreamID(sdif_int32 inStreamID);
static int hashFunction(sdif_int32 streamID);


static void InitStreamRenumbering(void *(*MemoryAllocator)(int numBytes)) {
    int i;
    for (i = 0; i < TABLE_SIZE; ++i) {
	inTable[i] = 0;
	outTable[i] = 0;
    }

    my_malloc = MemoryAllocator;
}

static sdif_int32 chooseOutStreamID(int inFileIndex, sdif_int32 inStreamID) {
    streamRenameRecord *p;
    sdif_int32 newID;
    int hashBucket;

    printf("* chooseOutStreamID(%ld, %ld): ", inFileIndex, inStreamID);

    
    hashBucket = hashFunction(inStreamID);

    /* First, see if we've already seen this stream */
    for (p = inTable[hashBucket]; p!=NULL; p=p->nextIn) {
	if (p->inStreamID == inStreamID && p->inFileIndex == inFileIndex) {
	    /* We've seen this stream already */
	    printf("already saw: %ld\n", p->outStreamID);
	    return p->outStreamID;
	}
    }

    /* If not, maybe we've seen this stream ID from another file */
    for (p = inTable[hashBucket]; p!=NULL; p=p->nextIn) {
        if (p->inStreamID == inStreamID) {
	    /* Conflict! */
	    newID = FindUnusedStreamID(inStreamID);
	    DoubleInsert(inFileIndex, inStreamID, newID);
	    printf("conflict! already saw %d/%d (assigned to %d); new ID %d\n", 
		   p->inFileIndex, p->inStreamID, p->outStreamID, newID);
	    return newID;
	}
    }
    /* We've never seen this stream, or any other with the same streamID */
    DoubleInsert(inFileIndex, inStreamID, inStreamID);
    printf("no problem; use %ld\n", inStreamID);
    return inStreamID;
}


static void DoubleInsert(int inFileIndex,sdif_int32 inStreamID, sdif_int32 outStreamID) {
    streamRenameRecord *result;
    int inPos, outPos;


    result = (*my_malloc)(sizeof(*result));
    result->inFileIndex = inFileIndex;
    result->inStreamID = inStreamID;
    result->outStreamID = outStreamID;

    inPos = hashFunction(inStreamID);
    result->nextIn = inTable[inPos];
    inTable[inPos] = result;

    outPos = hashFunction(outStreamID);
    result->nextOut = outTable[outPos];
    outTable[outPos] = result;
}


static sdif_int32 FindUnusedStreamID(sdif_int32 inStreamID) {
    sdif_int32 try;
    streamRenameRecord *p;

    for (try = inStreamID + 1; 1; ++try) {
	for (p = outTable[hashFunction(try)]; p != 0; p=p->nextOut) {
		printf("\n\ninStreamID = %d, outStreamID = %d\n\n", try, p->outStreamID);
	    if (p->outStreamID == try) {
		goto tryagain;
	    }
	}
	return try;
    tryagain:
	break;
    }
}

	
static int hashFunction(sdif_int32 streamID) {
    /* "Open addressing" */
    return streamID  % TABLE_SIZE;
}
#endif
