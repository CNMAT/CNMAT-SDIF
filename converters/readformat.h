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

 API for routines to read ASCII or Binary format files

 Matt Wright, 1/27/97
*/

typedef enum {BINARY, ASCII} formatType;

typedef struct eitherFormatStruct *eitherFormat;

eitherFormat OpenEitherFormat(char *filename, formatType t);
void CloseEitherFormat(eitherFormat e);
int ReadFrameHeader(eitherFormat e, int *numTracksp, float *timep);
int ReadTrack(eitherFormat e, float *index, float *freq, float *phase, float *amp);
