/*
Copyright (c) 2000.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for educational, research, and not-for-profit purposes, without
fee and without a signed licensing agreement, is hereby granted, provided that
the above copyright notice, this paragraph and the following two paragraphs
appear in all copies, modifications, and distributions.  Contact The Office of
Technology Licensing, UC Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley,
CA 94720-1620, (510) 643-7201, for commercial licensing opportunities.

Written by Matt Wright and Amar Chaudhary, The Center for New Music and Audio
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

/* TASS:  Trivial Additive Synthesizer for SDIF
   by Matt Wright and Amar Chaudhary, 8/2000

   Output is written as raw (headerless) 16-bit samples in the
   machine's native byte order.  

   We recommend you use this program in conjunction with sox:

   sox -t raw -r 44100 -s -w outputfile outputfile.aiff

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sdif.h"
#include "sdif-mem.h"
#include "sdif-types.h"
#include "sdif-sinusoids.h"

#define SRATE 44100.0

/* Stupid MSVC feature -AC 7/19/2000 */
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif


/* Prototypes */
void *MyMalloc(int numBytes);
void MyFree(void *memory, int numBytes);


/* Globals to keep track of how bad we clipped */
static int numClips = 0;
static sdif_float32 worstClip = 0.0f;

void ComplainAboutClipping(float amplitude, int whichsamp, 
			   sdif_float64 begintime, sdif_float64 endtime) {
    ++numClips;
    if (fabs(amplitude) > fabs(worstClip)) {
	worstClip = amplitude;
    }
}

void ReportClipping(void) {
    if (numClips) {
	printf("Clipped %d samples in synthesis; the biggest was %f\n",
	       numClips, worstClip);
	printf("A gain of %f would produce a full-scale sound file with no clipping\n",
	       fabs(1.0/worstClip));
    }
}


/* The additive synthesis */

#define MAXPARTIALS 40000
static float PhaseBank[MAXPARTIALS];
    /* It's wrong to assume that SDIF 1TRC index values will be small
       enough to use directly as array indices.  Instead we should use
       a hash table or some other data structure.  Sorry.  */


void InitSynthesis () {
  int i;
  for (i = 0; i < MAXPARTIALS; ++i) {
    PhaseBank[MAXPARTIALS] = 0.0f;
  }
}

void FirstFrameSynthesis (sinusoids frame) {
  int i;
  for (i = 0; i < frame->n; ++i) {
    PhaseBank[(int)(frame->s[i].index)] = frame->s[i].phase;
  }

}

void AdditiveSynthesis(TwoFrames *tf, FILE *outf, sdif_float32 gain, int verbose) {
  double time = tf->endtime - tf->begintime;
  int numsamples = (int)(time*SRATE);
  double dt = 1.0/(double)SRATE;
  int i, j;
  sdif_float32 *floatSamples;
  short *samples;
  sdif_float32 scaleFactor, clipValue;

  if (time <= 0.0) {
    return;
  }

  samples = malloc(numsamples * sizeof(short));
  floatSamples = malloc(numsamples * sizeof(*floatSamples));

  if (floatSamples == 0) {
    fprintf(stderr, "Out of memory!\n");
    return;
  }

  if (verbose) {
    printf ("Synthesizing %d sinusoids from time %f to %f: \n",
	    tf->n,tf->begintime, tf->endtime);
  }

  for (i = 0; i < numsamples; ++i) {
    floatSamples[i] = 0.0f;
  }

  for (i = 0; i < tf->n; ++i) {
    float amp1,amp2,freq1,freq2;
    switch (tf->sbf[i].status) {
    case BAD_DEATH:
    case GOOD_DEATH:
    case NORMAL:
      freq1 = tf->sbf[i].before->freq;
      amp1 = tf->sbf[i].before->amp;
      if (tf->sbf[i].after) {
	freq2 = tf->sbf[i].after->freq;
	amp2 = tf->sbf[i].after->amp;
      } else {
	freq2 = freq1;
	amp2 = amp1;
      }
      {
	float damp = (amp2 - amp1) / (float)(numsamples);
	float dfreq = (freq2 - freq1) / (float)(numsamples);
	float ddphase = 2 * M_PI * dfreq / SRATE;
	float dphase = 2 * M_PI * freq1 / SRATE;
	float a = amp1;
	float phase = PhaseBank[(int)(tf->sbf[i].before->index)];
	for (j = 0; j < numsamples; ++j) {
	  floatSamples[j] += a * sinf(phase);
	  phase += dphase;
	  dphase += ddphase;
	  a += damp;
	}
	
	PhaseBank[(int)(tf->sbf[i].before->index)] = phase;
      }
      break;
    case BAD_BIRTH:
    case GOOD_BIRTH:
      PhaseBank[(int)(tf->sbf[i].after->index)] = tf->sbf[i].after->phase;
      break;
    }
  }

  /* Convert float samples to short */

  scaleFactor = gain * 32767.0f;
  clipValue = 1.0f / gain;

  for (j = 0; j < numsamples; ++j) {
    if (floatSamples[j] < -clipValue || floatSamples[j] > clipValue) {
	ComplainAboutClipping(floatSamples[j]*gain, j, tf->begintime, tf->endtime);
	floatSamples[j] =  (floatSamples[j] < 0) ? -clipValue: clipValue;
    }

    samples[j] = ((short) (scaleFactor * floatSamples[j]));
  }

  fwrite(samples,sizeof(short),numsamples,outf);
  free(samples);
  free(floatSamples);
}




void ReadSinusoids(FILE *inf, FILE *outf, sdif_int32 streamID, sdif_float32 gain) {
    SDIFresult r;
    SDIF_FrameHeader fh;
    SDIFmem_Frame mframe;
    sinusoids prev, now;
    sdif_float64 time;

    prev = 0;

    while (!(r = SDIF_ReadFrameHeader(&fh, inf))) {
	if (fh.streamID != streamID) {
	    SDIF_SkipFrame(&fh, inf);
	    continue;
	}
	
	r=SDIFmem_ReadFrameContents(&fh, inf, &mframe);
	if (r) {
	    fprintf(stderr, "Error %d reading frame contents: %s\n",
		    r, SDIF_GetErrorString(r));
	    return;
	}

	now = FrameToSinusoids(mframe);
	time = mframe->header.time;

	SDIFmem_FreeFrame(mframe);

	if (!now) {
	    printf("* FrameToSinusoids returned 0...\n");
	    continue;
	}

	now->t = time;

	/* printf("* Read some sinusoids:\n");
	PrintSinusoids(now); */


	if (!prev) {
	    /* First frame of sinusoidal data */
	  FirstFrameSynthesis(now);
	  if (AnyNonZeroAmplitudes(now)) {
	    fprintf(stderr, "Warning: first frame of sine data has nonzero amplitudes.\n"
		    "Not ramping or smoothing; expect a click at the beginning.\n");
	  }
	} else {
	  TwoFrames *tf = MakeTwoFrames(prev, now);

/*
	    printf("Made a TwoFrames struct out of prev and now:\n");
	    printf("prev:\n");
	    PrintSinusoids(prev);
	    printf("\n\nnow:\n");
	    PrintSinusoids(now);
	    printf("\n\n");
*/	    

	  AdditiveSynthesis(tf, outf, gain, 0);
	  FreeTwoFrames(tf);
	  FreeSinusoids(prev);
	}

	prev = now;
	now = 0;
    }

    if (r != ESDIF_END_OF_DATA) {
	fprintf(stderr, "Error reading frame header: %s\n",
		SDIF_GetErrorString(r));
	return;
    }

    if (!prev) {
	fprintf(stderr, "Warning: stream %ld of SDIF file had no sinusoidal data!\n",
		streamID);
	return;
    }

    if (AnyNonZeroAmplitudes(prev)) {
	fprintf(stderr, "Warning: last frame of sine data has nonzero amplitudes.\n"
		"Not ramping or smoothing; expect a click at the end.\n");
    }
    FreeSinusoids(prev);

}


void *MyMalloc(int numBytes) {
    void * r = malloc(numBytes);
/*    printf("MyMalloc(%d): %p\n", numBytes, r); */
    return r;
}

void MyFree(void *memory, int numBytes) {
/*     printf("MyFree(%p, %d)\n", memory, numBytes); */
    free(memory);
}

    

int main(int argc, char *argv[]) {
    char *infile, *outfile;
    sdif_int32 streamID;
    FILE *inf, *outf;
    SDIFresult r;
    sdif_float32 gain;

    if (r = SDIF_Init()) {
	fprintf(stderr, "Error initializing SDIF: %s\n", 
		SDIF_GetErrorString(r));
	return -3;
    }

    if (r = SDIFmem_Init(MyMalloc, MyFree)) {
	fprintf(stderr, "Error initializing SDIFmem: %s\n", 
		SDIF_GetErrorString(r));
	return -4;
    }

    if (argc!=4 && argc !=5) goto usage;

    infile = argv[1];
    streamID = atoi(argv[2]);
    outfile = argv[3];

    if (argc == 5) {
	gain = atof(argv[4]);
    } else {
	gain = 1.0;
    }

    InitSynthesis();

    printf("Synthesizing stream %d from %s  with gain %f to create %s\n",
	   streamID, infile, gain, outfile);


    r = SDIF_OpenRead(infile, &inf);
    if (r) {
	fprintf(stderr, "%s: Couldn't open SDIF file %s: %s\n",
		argv[0], infile, SDIF_GetErrorString(r));
	goto usage;
    }


    /* try to open the output file.  don't overwrite anything. */
    if ((outf = fopen(outfile, "r")) != NULL) {
	fprintf(stderr, "%s: file \"%s\" already exists.  Exiting...\n",
		argv[0], outfile);
	return 1;
    }

    /*fclose(outf);*/
    
    /* Just open the file for now - restore SDIF_OpenWrite later - AC */
    outf = fopen(outfile,"wb");
    /*
    r = SDIF_OpenWrite(outfile, &outf);
    if (r != ESDIF_SUCCESS) {
	fprintf(stderr, "%s: error opening \"%s\": %s\n",
		argv[0], outfile, SDIF_GetErrorString(r));
	return 1;
    }
    */

    ReadSinusoids(inf, outf, streamID, gain);
    ReportClipping();


    if (r = SDIF_CloseWrite(outf)) {
	fprintf(stderr, "%s: error closing \"%s\": %s\n",
		argv[0], outfile, SDIF_GetErrorString(r));
    }
    if (r = SDIF_CloseRead(inf)) {
	fprintf(stderr, "%s: error closing \"%s\": %s\n",
		argv[0], infile, SDIF_GetErrorString(r));
    }

    return 0;

usage:
    fprintf(stderr, "Usage: %s [1TRC_input_SDIF_file] [stream] [1TDS_output_SDIF_file] [optional gain]\n",
	    argv[0]);
    return -2;
}
