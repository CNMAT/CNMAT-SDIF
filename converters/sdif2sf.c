/*
Copyright (c) 2000.  The Regents of the University of California
(Regents). All Rights Reserved.

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

 sdif2sf.c: convert TDS streams in an SDIF file to sound files

 SDIF spec: http://www.cnmat.berkeley.edu/SDIF/

 version 0.1
*/

/* #define DEBUG */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "sdif.h"
#include "soundfiles.h"

#define MAX_STREAMS 1000

/* Globals for what the program is doing */
int firstTDSStream;

typedef struct {
    sdif_int32 streamid;
    char *soundfilename;
    SFWHandle sfwhandle;
    sdif_int16 *buffer;	    /* Sample values to be written */
    int n;		    /* # samples in buffer */
    sdif_float64 buftime;   /* Time tag of frame that samples in buffer came from. */
    int seenyet;	    /* Has any frames for this stream been seen yet? */
    sdif_float32 srate;
    int nchans;
} Stream;

Stream streams[MAX_STREAMS];

int numStreams = 0;




void InitStreams(void) {
    int i;

    for (i = 0; i < numStreams; ++i) {
	streams[i].seenyet = 0;
    }
}
					   
int WhichStream(sdif_int32 streamid) {
    int i;

    for (i = 0; i < numStreams; ++i) {
	if (streams[i].streamid = streamid) {
#ifdef DEBUG
	    printf("* StreamID %d is stream %d in my array.\n",
		   streamid, i);
#endif
	    return i;
	}
    }
    return -1;
}

void PrintStream(Stream s) {
    printf("Stream ID %d, soundfile \"%s\"\n", s.streamid, s.soundfilename);
    printf("  Buffer %x, n %d, buftime %f\n", s.buffer, s.n, s.buftime);
    printf("  Seenyet %d, srate %f, nchans %d\n", s.seenyet, s.srate, s.nchans);
}



sdif_int16 *ReadTDSFrame(FILE *f, SDIF_FrameHeader *fh, sdif_float32 *srate,
			 int *nchans, int *nsamps) {
    int i, m, saw1TDS, sawITDS;
    SDIF_MatrixHeader mh;
    SDIFresult r;
    sdif_int16 *result;

    saw1TDS=sawITDS=0;

    for (m = 0; m < fh->matrixCount; ++m) {
	r=SDIF_ReadMatrixHeader(&mh, f);

#ifdef DEBUG
	printf("** Read matrix of type %c%c%c%c: %d rows, %d cols\n",
	       mh.matrixType[0],mh.matrixType[1],mh.matrixType[2],mh.matrixType[3],
	       mh.rowCount, mh.columnCount);
#endif

	if (r) {
	    fprintf(stderr, "Read error: %s\n", SDIF_GetErrorString(r));
	    return 0;
	}

	if (SDIF_Char4Eq(mh.matrixType, "1TDS")) {
	    int numSamples = mh.rowCount * mh.columnCount;

	    if (saw1TDS) {
		fprintf(stderr, "Frame has more than one 1TDS matrix!\n");
		return 0;
	    }
	    saw1TDS = 1;

	    *nchans = mh.columnCount;
	    *nsamps = numSamples;
	    result = malloc(numSamples * sizeof(*result));
	    

	    if (!result) {
		fprintf(stderr, "Couldn't allocate %d bytes.\n", 
			numSamples * sizeof(*result));
		return 0;
	    }


	    if (mh.matrixDataType == SDIF_FLOAT32) {
		sdif_float32 f32;
		sdif_float32 scaleFactor = -((sdif_float32) SHRT_MIN);
		    
		for (i = 0; i < numSamples; ++i) {
		    r=SDIF_Read4(&f32, 1, f);
		    if (r) fprintf(stderr, "Read error: %s\n", SDIF_GetErrorString(r));
		    result[i] = (sdif_int16) (scaleFactor * f32);

		    if (i%1000 ==0) {
			printf("* i=%d. read float %f, scaled to %f -> %d\n", i, f32,
			       (scaleFactor * f32), result[i]);
		    }

		}

	    } else if (mh.matrixDataType == SDIF_FLOAT64) {
		sdif_float64 f64;
		sdif_float64 scaleFactor = 1.0f / -((sdif_float64) SHRT_MIN);

		for (i = 0; i < numSamples; ++i) {
		    r=SDIF_Read8(&f64, 1, f);
		    if (r) fprintf(stderr, "Read error: %s\n", SDIF_GetErrorString(r));
		    result[i] = (sdif_int16) (scaleFactor * f64);
		}

	    } else if (mh.matrixDataType == SDIF_INT16) {
		/* The easy case */

		r = SDIF_Read2(result, numSamples, f);
		if (r) fprintf(stderr, "Read error: %s\n", SDIF_GetErrorString(r));

	    } else if (mh.matrixDataType == SDIF_INT32) {
		sdif_int32 i32;


                for (i = 0; i < numSamples; ++i) {
                    r=SDIF_Read4(&i32, 1, f);
		    if (r) fprintf(stderr, "Read error: %s\n", SDIF_GetErrorString(r));
		    result[i] = (sdif_int16) (i32 >> 16);
		}

	    } else if (mh.matrixDataType == SDIF_INT64) {
		fprintf(stderr, "Sorry, this sdif library doesn't support 64-bit ints.\n");
		free(result);
		return 0;
	    } else {
		fprintf(stderr, "Error: 1TDS matrixDataType %x is illegal.\n",
			mh.matrixDataType);
		free(result);
		return 0;
	    }

	} else if (SDIF_Char4Eq(mh.matrixType, "ITDS")) {
	    int matrixSize;

	    if (sawITDS) {
		fprintf(stderr, "Frame has more than one 1TDS matrix!\n");
		return 0;
	    }
	    sawITDS = 1;


	    if (mh.matrixDataType != SDIF_FLOAT32) {
		fprintf(stderr, "Error: ITDS matrixDataType is %x instead of SDIF_FLOAT32\n", 
			mh.matrixDataType);
		return 0;
	    }

	    matrixSize = mh.rowCount * mh.columnCount;

	    if (matrixSize == 0) {
		fprintf(stderr, "Error: No data in ITDS matrix!\n");
		return 0;
	    }

	    r = SDIF_Read4(srate, 1, f);
	    if (r) {
		fprintf(stderr, "Read error: %s\n", SDIF_GetErrorString(r));
		return 0;
	    }

	    if (matrixSize > 1) {
		/* There's some other junk we don't know about in the ITDS 
		   matrix, so skip it. */
		r = SkipBytes(f, (matrixSize-1) * sizeof(sdif_float32));
		if (r) {
		    fprintf(stderr, "Error skipping bytes: %s\n", SDIF_GetErrorString(r));
		    return 0;
		}
	    }
	    /* Now skip padding */
	    r = SkipBytes(f, SDIF_PaddingRequired(&mh));
	    if (r) {
		fprintf(stderr, "Error skipping bytes: %s\n", SDIF_GetErrorString(r));
		return 0;
	    }
	} else {
	    /* Matrix type we don't recognize, so ignore. */
	    r=SDIF_SkipMatrix(&mh, f);
	    if (r) {
		fprintf(stderr, "Error skipping matrix: %s\n", SDIF_GetErrorString(r));
		return 0;
	    }
	}
    }

    if (!saw1TDS) {
	fprintf(stderr, "1TDS frame (stream %d, time %f)  missing required 1TDS matrix!\n",
		fh->streamID, fh->time);
	return 0;
    }
    if (!sawITDS) {
	fprintf(stderr, "1TDS frame (stream %d, time %f) missing required ITDS matrix!\n",
		fh->streamID, fh->time);
	return 0;
    }

#ifdef DEBUG
    { int n=*nsamps;
	printf("Read a 1TDS frame: %x, %d samples\n", result, n);
	printf("\t[%d, %d, %d, %d, %d, ..., %d, %d, %d, %d, %d]\n",
	   result[0], result[1], result[2], result[3], result[4],
	   result[n-5], result[n-4], result[n-3], result[n-2],
	   result[n-1]);
    }
#endif


    return result;
}

void WriteSamplesOrDie(SFWHandle wh, short *samples, int n) {
    Boolean b;
    int nFrames = n/SFWNumChannels(wh);


#ifdef DEBUG
    printf("* WriteSamplesOrDie(%p, %p, %d)\n", wh, samples, n);
    if (n >= 5) {
	printf("*   [%d, %d, %d, %d, %d, ..., %d, %d, %d, %d, %d]\n",
	       samples[0], samples[1], samples[2], samples[3], samples[4],
	       samples[n-5], samples[n-4], samples[n-3], samples[n-2],
	       samples[n-1]);
    }
#endif

    b=SFWriteInterleavedShorts(wh, nFrames, samples, 1);

    if (b == FALSE) {
	fprintf(stderr, "Error writing samples: %s\n",
		SFGetLastErrorAsString());
	exit(-4);
    }

#ifdef WRITE_ONCE_AND_FINISH
    printf("Just wrote for the first time; finishing.\n");

    SFCloseWrite(wh);

    printf("Wrote the file as soon as the first samples were ready.\n");

    exit(0);

#endif

}

void ReadFile(FILE *f) {
    int onlyFirstFrame = 0;

    SDIF_FrameHeader fh;    
    SDIFresult r;
    int whichStream, i;
    Stream *s;

    while (!(r = SDIF_ReadFrameHeader(&fh, f))) {

	if (!SDIF_Char4Eq("1TDS", fh.frameType)) {
	    SDIF_SkipFrame(&fh, f);
	    continue;
	}

	if (firstTDSStream) {
	    /* We just saw the first 1TDS stream */
	    streams[0].streamid = fh.streamID;
	    firstTDSStream = 0;
	    s = &(streams[0]);
	    goto first_frame;
	} else {
	    whichStream = WhichStream(fh.streamID);
	    if (whichStream == -1) {
		/* Not a stream we care about. */
		SDIF_SkipFrame(&fh, f);
	    } else {
		s = &(streams[whichStream]);

		if (!(s->seenyet)) {
		  first_frame:
		    /* First frame of this stream */
		    s->seenyet = 1;

		    s->buffer = 
		      ReadTDSFrame(f, &fh, &(s->srate), &(s->nchans), &(s->n));
		    if (s->buffer == 0) exit(-2);

		    s->buftime = fh.time;

#ifdef DEBUG
		    printf("** First frame of stream %d has %d channels, %f srate, %d samples->\n",
			   s->streamid, s->nchans,
			   s->srate, s->n);
#endif

		    s->sfwhandle = 
		      SFOpenWrite(s->soundfilename, AIFF, s->nchans, s->srate);

#ifdef DEBUG
		    printf("Opened soundfile %s, got handle %x\n",
			   s->soundfilename, s->sfwhandle);
#endif

		    if (onlyFirstFrame) {
			printf("Only first frame: I read it, so now I'm going to try to finish up.\n");
			WriteSamplesOrDie(s->sfwhandle, s->buffer, s->n);
			printf("* wrote\n");
			SFCloseWrite(s->sfwhandle);

			printf("Made sound file out of first 1TDS frame.  Quitting.\n");
			exit(0);
		    }
			
		} else {
		    /* We saw this frame already, so now we get to
		       worry about changes in the number of samples 
		       or the sampling rate, and possible time overlap
		       or underlap of 1TDS chunks in adjacent
		       frames */

		    sdif_int16 *newsamples;
		    sdif_float32 newsrate;
		    int newnchans, newn;
		    sdif_float64 bufendtime;

		    newsamples = ReadTDSFrame(f, &fh, &newsrate, &newnchans, &newn);
		    if (newsamples == 0) exit(-2);

		    if (newsrate != s->srate) {
			fprintf(stderr, "Stream %d starts with srate %f but the frame at time %f has srate %f!\n",
				fh.streamID, s->srate, fh.time, newsrate);
			exit(-3);
		    }

		    if (newnchans != s->nchans) {
			fprintf(stderr, "Stream %d starts with %d channels but the frame at time %f has %d channels!\n",
				fh.streamID, s->nchans, fh.time, newnchans);
			exit(-3);
		    }

		    bufendtime = s->buftime + (s->srate * s->n);
		    if (fh.time < bufendtime) {
			int nframesFromBefore, j;
			/* Overlap */

			/* First write samples from the previous frame
                           that come before this frame */
			nframesFromBefore = (fh.time - s->buftime) * s->srate;
			WriteSamplesOrDie(s->sfwhandle, s->buffer, 
					  nframesFromBefore );

			/* Now add the overlapping samples into the new buffer */
			for (i= nframesFromBefore, j=0; i<s->n; ++i, ++j) {
			    newsamples[j] += s->buffer[i];
			}
			
			free(s->buffer);
			s->buffer = newsamples;
			s->n = newn;
			s->buftime = fh.time;
			
		    } else if (fh.time == bufendtime) {
			/* The usual case */

			WriteSamplesOrDie(s->sfwhandle, s->buffer, s->n);

			free(s->buffer);
			s->buffer = newsamples;
			s->n = newn;
			s->buftime = fh.time;
		    } else {
			/* Underlap */

			/* Write all old samples */
			WriteSamplesOrDie(s->sfwhandle, s->buffer, s->n);

			/* Write a buncha zeroes */
                        if (!SFWriteZeroes(s->sfwhandle,
					   (fh.time-bufendtime)*s->srate)) {
                            fprintf(stderr, "Error writing zero samples: %s\n",
                                    SFGetLastErrorAsString());
                            exit(-4);
			}

                        free(s->buffer);
                        s->buffer = newsamples;
                        s->n = newn;
                        s->buftime = fh.time;
		    }
		}
	    }
	}
    }

    if (r != ESDIF_END_OF_DATA) {
	fprintf(stderr, "Error reading frame header: %s\n",
		SDIF_GetErrorString(r));
	exit(-7);
    }

    /* OK, done reading the SDIF file, so now we finish writing and
       close all the sound files. */


    printf("Done reading SDIF file\n");

    for (i = 0; i < numStreams; ++i) {
	s = &(streams[i]);
	PrintStream(*s);
	if (!s->seenyet) {
	    printf("Warning: Never saw stream %d, didn't write %s\n",
		   s->streamid, s->soundfilename);
	} else {
	    /* Finish writing samples */

	    WriteSamplesOrDie(s->sfwhandle, s->buffer, s->n);

	    printf("Wrote last samples\n");

	    free(s->buffer);

	    printf("Freed buffer\n");

	    /* Close the file */
	    SFCloseWrite(s->sfwhandle);

	    printf("Closed file\n");
	}
    }
}


void main(int argc, char **argv) {
    int i;
    SDIFresult r;
    FILE *sdif_fp;


    if (argc < 3) goto usage;

    /* First arg must be name of SDIF file */
    r = SDIF_OpenRead(argv[1], &sdif_fp);
    if (r) {
	fprintf(stderr, "%s: Couldn't open SDIF file %s: %s\n",
		argv[0], argv[1], SDIF_GetErrorString(r));
	goto usage;
    }

    if (strcmp(argv[2], "-stream") == 0) {  
	firstTDSStream = 0;
	for (i = 2; i < argc; i += 3) {
	    if (strcmp(argv[i], "-stream") != 0) goto usage;
	    
	    if (numStreams == MAX_STREAMS) {
		fprintf(stderr, "You want me to write more than %d sound files; give me a break!\n",
			MAX_STREAMS);
		goto usage;
	    }

	    streams[numStreams].streamid = atoi(argv[i+1]);
	    streams[numStreams].soundfilename = argv[i+2];
	    ++numStreams;
	}
    } else {
	if (argc != 3) goto usage;
	firstTDSStream = 1;
	streams[0].soundfilename = argv[2];
	numStreams = 1;
    }

    if (firstTDSStream) {
	printf("Looking for the first TDS stream, will write %s\n",
	     streams[0].soundfilename);
    } else {
	printf("Streams and their resulting sound files:\n");
	for (i = 0; i < numStreams; ++i) {
	    printf("  stream %d -> %s\n", streams[i].streamid,
		   streams[i].soundfilename);
	}
    }

    InitStreams();

    ReadFile(sdif_fp);

    return;

usage:
    fprintf(stderr, "Usage: %s <sdiffilename> <soundfilename>\n"
	    "%s <sdiffilename> -stream <streamnum> <soundfilename> [...]\n",
	    argv[0], argv[0]);
}


#if 0

Things to worry about:

    Gaps in the time axis have implicit zero samples

    Frames that overlap in the time axis are added

    Is lookahead therefore bounded?

#endif
