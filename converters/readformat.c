/*
 * Copyright(c) 1997 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 readformat.h

 Routines to read ASCII or Binary format files

 Matt Wright, 1/27/97
*/

#include "readformat.h"
#include <stdio.h>

struct eitherFormatStruct {
    formatType t;
    FILE *f;
};

eitherFormat OpenEitherFormat(char *filename, formatType t) {
    eitherFormat result = (eitherFormat) malloc(sizeof(*result));
    result->t = t;
    result->f = fopen(filename, "r");
    if (result->f == NULL) {
	fprintf(stderr, "Couldn't open \"%s\" for reading; skipping.\n", 
		filename);
	free(result);
	return NULL;
    }
    return result;
}

void CloseEitherFormat(eitherFormat e) {
    fclose(e->f);
    free(e);
}

int ReadFrameHeader(eitherFormat e, int *numTracksp, float *timep) {
    if (e->t == ASCII) {
	return fscanf(e->f, "%d%f", numTracksp, timep);
    } else {
	float data[2];

	if (fread(data, sizeof(*data), 2, e->f) != 2) {
	    if (feof(e->f)) return EOF;
	    fprintf(stderr, "Unexpected read error.\n");
	    return EOF;	/* Don't know what else to do. */
	}
	*numTracksp = (int) data[0];
	*timep = data[1];
    }
}

int ReadTrack(eitherFormat e, float *index, float *freq, float *phase, float *amp) {
    if (e->t == ASCII) {
	int indexi, result;
        /* Fields on a line of a format file:  index, freq, amp, phase. */
	result = fscanf(e->f, "%d%f%f%f", &indexi, freq, amp, phase);
	*index = (float) indexi;
	return result;
    } else {
	float data[4];
        if (fread(data, sizeof(*data), 4, e->f) != 4) {
	    fprintf(stderr, "Bad binary fmt file\n");
	    return 0;
	}
	*index = data[0];
	*freq = data[1];
	*amp = data[2];
	*phase = data[3];
    }
}

