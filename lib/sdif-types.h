/* 
Copyright (c) 1996, 1997, 1998, 1999.  The Regents of the University of
California (Regents).  All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for educational, research, and not-for-profit purposes, without
fee and without a signed licensing agreement, is hereby granted, provided that
the above copyright notice, this paragraph and the following two paragraphs
appear in all copies, modifications, and distributions.  Contact The Office of
Technology Licensing, UC Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley,
CA 94720-1620, (510) 643-7201, for commercial licensing opportunities.

Written by Matt Wright and Sami Khoury, The Center for New Music and Audio
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

/* sdif-types.h

   Helper procedures for some of SDIF's standard frame and matrix types.

   SDIF spec: http://www.cnmat.berkeley.edu/SDIF/   
*/



/****** 1TRC ******/

typedef struct {
    sdif_float32 index, freq, amp, phase;
} SDIF_RowOf1TRC;

int SDIF_WriteRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f);
int SDIF_ReadRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f);
sdif_int32 SizeOf1TRCFrame(int numTracks);

/* Read a row of 1TRC data from an open file, writing results into pointers
   you pass as arguments.  Returns 0 if succesful, nonzero otherwise. */
int SDIF_Read1TRCVals(FILE *f,
                      sdif_float32 *indexp, sdif_float32 *freqp,
                      sdif_float32 *ampp, sdif_float32 *phasep);

/* Write a row of 1TRC data to an open file.  Returns 0 if succesful, nonzero
   otherwise. */
int SDIF_Write1TRCVals(FILE *f,
                       sdif_float32 index, sdif_float32 freq,
                       sdif_float32 amp, sdif_float32 phase);

/* How big does the size count need to be in a frame of 1TRC? */
/* (Assuming that the frame contains one matrix) */
sdif_int32 SDIF_SizeOf1TRCFrame(int numTracks);


/****** 1RES ******/

typedef struct {
   sdif_float32 freq, amp, decayrate, phase;
} SDIF_RowOf1RES;

sdif_int32 SDIF_SizeOf1RESFrame(int numResonances);
int SDIF_WriteRowOf1RES(SDIF_RowOf1RES *row, FILE *f);
int SDIF_ReadRowOf1RES(SDIF_RowOf1RES *row, FILE *f);


