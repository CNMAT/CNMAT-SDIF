/* 

Copyright (c) 1997,1998,1999.  The Regents of the University of California
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

 readformat.h

 Routines to read ASCII or Binary format files

 Matt Wright, 1/27/97
*/

#include "readformat.h"
#include <stdio.h>
#include <stdlib.h>

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
    return 1;
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
    return 1;
}

