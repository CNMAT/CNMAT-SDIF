/*
 * Copyright (c) 1997 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 sdif.h

 Utilities for formatting data into SDIF

 Matt Wright, 1/24/97
*/

#include <stdio.h>
#include "sdif.h"

void FillSDIFGlobalHeader(struct SDIFGlobalHeader *h, int4 size) {
    Copy4Bytes(h->FORM, "FORM");
    Copy4Bytes(h->SDIF, "SDIF");
    h->size = size;
}

void Copy4Bytes(char *target, const char *string) {
    target[0] = string[0];
    target[1] = string[1];
    target[2] = string[2];
    target[3] = string[3];
}

int str4eq(const char *this, const char *that) {
    return this[0] == that[0] && this[1] == that[1] &&
	this[2] == that[2] && this[3] == that[3];
}

FILE *OpenSDIFWrite(char *filename, int size) {
    struct SDIFGlobalHeader h;
    FILE *result = fopen(filename, "w");

    if (result == NULL) return NULL;
    FillSDIFGlobalHeader(&h, size);
    if (fwrite(&h, sizeof(h), 1, result) != 1) {
	fclose(result);
	return NULL;
    }
    return result;
}
    
int CloseSDIFWrite(FILE *f) {
    /* To do:  We could use fseek() and ftell() to compute the size, then go
       back and patch it in the header. */

    return fclose(f);
}

FILE *OpenSDIFRead(char *filename, int *sizep) {
    FILE *result = fopen(filename, "r");
    char buf[4];
    int size;

    if (result == NULL) return NULL;

    /* Now make sure the header is OK. */
    if (fread(buf, sizeof(*buf), 4, result) != 4) goto lose;
    if (!str4eq(buf, "FORM")) goto lose;
    if (fread(&size, sizeof(size), 1, result) != 1) goto lose;
    if (fread(buf, sizeof(*buf), 4, result) != 4) goto lose;
    if (!str4eq(buf, "SDIF")) goto lose;

    if (sizep != NULL) {
	*sizep = size;
    }
    return result;

lose:
    fprintf(stderr, "Bad SDIF header in file %s\n", filename);
    fclose(result);
    return NULL;
}

int CloseSDIFRead(FILE *f) {
    fclose(f);
}

#include <sys/time.h>

int8 GenUniqueSDIFFrameID(void) {
    int8 result;
    struct timeval t;

    if (gettimeofday(&t) != 0) {
	fprintf(stderr, "gettimeofday() failed---Unique ID may not be.");
	return 123456789;
    }

    result = t.tv_sec * 1000000 + t.tv_usec;
    return result;
}

int SkipSDIFFrame(struct SDIFFrameHeader *head, FILE *f) {
    /* The header's size count includes the 8-byte time tag and 8-byte
       ID that we already read. */
    int bytesToSkip = head->size - 16;

    if (bytesToSkip < 0) {
	fprintf(stderr, "Badly formed SDIF file: frame's size too low for time tag and ID.\n");
	return -1;
    }

    if (fseek(f, bytesToSkip, SEEK_CUR) != 0) {
	fprintf(stderr, "fseek failed while skipping unrecognized frame type\n");	
	return -2;
    }

    return 0;
}

int4 SizeOf1TRCFrame(int numTracks) {
    /* 16 bytes for the time stamp and ID, plus 8 bytes for the # rows and #
       columns, plus four 4-byte floating point numbers (index, freq, phase,
       amp) for each track appearing in this frame. */

    return 16 + 8 + (4 * 4 * numTracks);
}
