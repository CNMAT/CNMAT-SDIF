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

 API for routines to read ASCII or Binary format files

 Matt Wright, 1/27/97
*/

typedef enum {BINARY, ASCII} formatType;

typedef struct eitherFormatStruct *eitherFormat;

eitherFormat OpenEitherFormat(char *filename, formatType t);
void CloseEitherFormat(eitherFormat e);
int ReadFrameHeader(eitherFormat e, int *numTracksp, float *timep);
int ReadTrack(eitherFormat e, float *index, float *freq, float *phase, float *amp);
