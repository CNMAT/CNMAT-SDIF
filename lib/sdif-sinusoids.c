/* 
Copyright (c) 2000.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Matt Wright, The Center for New Music and Audio
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

#include <stdio.h>
#include "sdif.h"
#include "sdif-types.h"
#include "sdif-mem.h"

#include "sdif-sinusoids.h"

/* Get access to sdif-mem's malloc/free procedures */
extern void *(*my_malloc)(int numBytes);
extern void (*my_free)(void *memory, int numBytes);


sinusoids AllocSinusoids(int n) {
    sinusoids result;

    result = (sinusoids) (*my_malloc)(sizeof(*result));
    if (!result) return 0;

    result->n = n;
    if (n) {
	result->s = (Sinusoid *) (*my_malloc)(n*sizeof(Sinusoid));
    } else {
	result->s = 0;
    }
    return result;
}

void FreeSinusoids(sinusoids s) {
    if (s->n) {
	(*my_free)(s->s, s->n * sizeof(s->s));
    }
    (*my_free)(s, sizeof(*s));
}

void PrintSinusoids(sinusoids s) {
    int i;

    printf("%d Sines:\n", s->n);
    for (i=0; i<s->n; ++i) {
	printf(" i %f, f %f, a %f, p %f\n", s->s[i].index, s->s[i].freq,
	       s->s[i].amp, s->s[i].phase);
    }
}


int AnyNonZeroAmplitudes(sinusoids s) {
    int i;

    for (i=0; i<s->n; ++i) {
	if (s->s[i].amp != 0.0) return 1;
    }
    return 0;
}

/* Reading SDIF data into our data structures */


sinusoids MatrixToSinusoids(SDIFmem_Matrix matrix) {
    sinusoids result;
    SDIF_MatrixDataType type;
    int row, i;
    sdif_float32 *matrixData32;
    sdif_float64 *matrixData64;


    if (matrix->header.matrixDataType == SDIF_FLOAT32) {
	type = SDIF_FLOAT32;
	matrixData32 = (sdif_float32 *) matrix->data;
    } else if (matrix->header.matrixDataType == SDIF_FLOAT64) {
	type = SDIF_FLOAT64;
	 matrixData64 = (sdif_float64 *) matrix->data;
    } else {
	fprintf(stderr, "matrix data type 0x%x not allowed in a %c%c%c%c matrix\n",
		matrix->header.matrixDataType, matrix->header.matrixType[0],
		matrix->header.matrixType[1], matrix->header.matrixType[2],
		matrix->header.matrixType[3]);
	return 0;
    }

    result = AllocSinusoids(matrix->header.rowCount);
    if (!result) {
	fprintf(stderr, "malloc failed\n");
	return 0;
    }

    if (matrix->header.columnCount < 2) {
	fprintf(stderr, "Required column(s) missing\n");
	return 0;
    }

    if (type == SDIF_FLOAT32) {
	for (row = 0, i=0; row < matrix->header.rowCount; ++row, i+=matrix->header.columnCount) {
	    result->s[row].index = matrixData32[i];
	    result->s[row].freq = matrixData32[i+1];

	    if (matrix->header.columnCount >= 3) {
		result->s[row].amp = matrixData32[i+2];
	    } else {
		result->s[row].amp = 1.0;
	    }

	    if (matrix->header.columnCount >= 4) {
		result->s[row].phase = matrixData32[i+3];
	    } else {
		result->s[row].phase = 1.0;
	    }

	}
    } else {
	for (row = 0, i=0; row < matrix->header.rowCount; ++row, i+=matrix->header.columnCount) {
	    result->s[row].index = matrixData64[i];
	    result->s[row].freq = matrixData64[i+1];


	    if (matrix->header.columnCount >= 3) {
		result->s[row].amp = matrixData64[i+2];
	    } else {
		result->s[row].amp = 1.0;
	    }

	    if (matrix->header.columnCount >= 4) {
		result->s[row].phase = matrixData64[i+3];
	    } else {
		result->s[row].phase = 1.0;
	    }
	}
    }

    return result;
}


sinusoids FrameToSinusoids(SDIFmem_Frame frame) {
    SDIFmem_Matrix mp;

    if (SDIF_Char4Eq(frame->header.frameType, "1TRC")) {
	for (mp=frame->matrices; mp!=0; mp=mp->next) {
	    if (SDIF_Char4Eq(mp->header.matrixType, "1TRC")) {
		return MatrixToSinusoids(mp);
	    }
	}
	fprintf(stderr, "1TRC frame (time %f, streamID %d) has no 1TRC matrix!\n",
		frame->header.time, frame->header.streamID);
	return 0;
    } else if (SDIF_Char4Eq(frame->header.frameType, "1HRM")) {
	for (mp=frame->matrices; mp!=0; mp=mp->next) {
	    if (SDIF_Char4Eq(mp->header.matrixType, "1HRM")) {
		return MatrixToSinusoids(mp);
	    }
	}
	fprintf(stderr, "1HRM frame (time %f, streamID %d) has no 1HRM matrix!\n",
		frame->header.time, frame->header.streamID);
	return 0;

    } else {
	fprintf(stderr, "Frame type %c%c%c%c is not sinusoids. (Time %f, streamID %d)\n",
		frame->header.frameType[0], frame->header.frameType[1],
		frame->header.frameType[2], frame->header.frameType[3],
		frame->header.time, frame->header.streamID);
	return 0;
    }
}



/* Dealing with pairs of frames */

TwoFrames *MakeTwoFrames(sinusoids begin, sinusoids end) {
    TwoFrames *result;
    int i, j;

    result = (TwoFrames *)(*my_malloc)(sizeof(*result));
    if (!result) return result;

    /* We don't know how many unique indexes there are until
       we do the index matching, so we malloc() the upper bound,
       assuming that none of the indices in the two frames match. */
    result->size = (begin->n + end->n);
    result->sbf = (SinusoidBetweenFrames *) 
		    (*my_malloc)(result->size * sizeof(*(result->sbf)));
    
    if (! result->sbf) {
	(*my_free)(result, sizeof(*result));
	return 0;
    }

/*    printf("* Made a twoframes:  %p, sbf = %p, size = %d (*size = %d)\n",
	   result, result->sbf, result->size, result->size * sizeof(result->sbf));

*/

    result->begintime = begin->t;
    result->endtime = end->t;

    /* Load up all the sines in the first frame */
    for (i=0; i<begin->n; ++i) {
	result->sbf[i].before = &(begin->s[i]);
	result->sbf[i].after = 0;
    }

    result->n = begin->n;

    /* The matching: check each sine in the end frame for a match in the
       sines already in our result */

    for (j=0; j<end->n; ++j) {
	for (i=0; i<begin->n; ++i) {
	    if (result->sbf[i].before->index == end->s[j].index) {
		result->sbf[i].after = &(end->s[j]);
		goto matched;
	    }
	}
	/* Didn't find a match */
	result->sbf[result->n].before = 0;
	result->sbf[result->n].after = &(end->s[j]);
	++(result->n);

	matched:
	/* Move on to the next partial */
	;
    }


    /* Now figure out the status of each sine */

    for (i=0; i<result->n; ++i) {
	if (result->sbf[i].before == 0) {
	    if (result->sbf[i].after == 0) {
		fprintf(stderr, "Internal error!  before=after=0!\n");
		return 0;
	    }

	    if (result->sbf[i].after->amp == 0.0f) {
		result->sbf[i].status = GOOD_BIRTH;
	    } else {
		result->sbf[i].status = BAD_BIRTH;
	    }
	} else if (result->sbf[i].after == 0) {
	    if (result->sbf[i].before->amp == 0.0f) {
                result->sbf[i].status = GOOD_DEATH;
	    } else {
                result->sbf[i].status = BAD_DEATH;
	    }
	} else {
	    result->sbf[i].status = NORMAL;
	}
    }

    return result;
}

void FreeTwoFrames(TwoFrames *x) {
    (*my_free)(x->sbf, x->size*sizeof(*(x->sbf)));
    (*my_free)(x, sizeof(*x));
}


char *SineStatusAsString(SineStatus s) {
    if (s==NORMAL) return "NORMAL";
    if (s==GOOD_BIRTH) return "GOOD_BIRTH";
    if (s==BAD_BIRTH) return "BAD_BIRTH";
    if (s==GOOD_DEATH) return "GOOD_DEATH";
    if (s==BAD_DEATH) return "BAD_DEATH";
    return "Unknown";
}

void PrintTwoFrames(TwoFrames *x) {
    int i;

    printf("Two Frames: from time %f to time %f, %d partials\n",
	   x->begintime, x->endtime, x->n);

    for (i=0; i<x->n; ++i) {
	printf(" [%3d] %s: ", i, SineStatusAsString(x->sbf[i].status));
	switch (x->sbf[i].status) {
	    case GOOD_BIRTH:
	    case BAD_BIRTH:
		printf("i %.2f f %.2f, a %f, p %.2f\n",
		       x->sbf[i].after->index,
		       x->sbf[i].after->freq, x->sbf[i].after->amp,
		       x->sbf[i].after->phase);
		break;

	    case GOOD_DEATH:
	    case BAD_DEATH:
		printf("i %.2f f %.2f, a %f, p %.2f\n",
		       x->sbf[i].before->index,
		       x->sbf[i].before->freq, x->sbf[i].before->amp,
		       x->sbf[i].before->phase);
		break;

	    case NORMAL:
		printf("i %.2f f %.2f to %.2f, a %f to %f, p %.2f to %.2f\n",
		       x->sbf[i].before->index,
		       x->sbf[i].before->freq, x->sbf[i].after->freq,
		       x->sbf[i].before->amp, x->sbf[i].after->amp,
		       x->sbf[i].before->phase, x->sbf[i].after->phase);
		break;

	    default:
		printf(" --- Unrecognized type.\n");
	}
    }
    printf("...that's all.\n");
}

