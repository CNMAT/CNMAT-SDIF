/* 
Copyright (c) 1996, 1997, 1998, 1999, 2000.  The Regents of the University of
California (Regents).  All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

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

   Note that this module assumes sdif_float32 data.

   SDIF spec: http://www.cnmat.berkeley.edu/SDIF/   


   You must include sdif.h and sdif-mem.h before this file.
*/


/* Common to multiple frame/matrix types */

/* WrapPhase32 --
   Add n * 2 * pi to make the result in the range -pi to pi */
sdif_float32 WrapPhase32(sdif_float32 input_phase);

/* WrapPhase64 --
   Add n * 2 * pi to make the result in the range -pi to pi */
sdif_float64 WrapPhase64(sdif_float64 input_phase);


/****** 1HRM ******/

typedef struct {
    sdif_float32 partialnumber, freq, amp, phase;
} SDIF_RowOf1HRM;


/****** 1TRC ******/

typedef struct {
    sdif_float32 index, freq, amp, phase;
} SDIF_RowOf1TRC;



SDIFresult SDIF_WriteRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f);
SDIFresult SDIF_ReadRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f);
sdif_int32 SizeOf1TRCFrame(int numTracks);

/* Read a row of 1TRC data from an open file, writing results into pointers
   you pass as arguments.   */
SDIFresult SDIF_Read1TRCVals(FILE *f,
			     sdif_float32 *indexp, sdif_float32 *freqp,
			     sdif_float32 *ampp, sdif_float32 *phasep);

/* Write a row of 1TRC data to an open file. */
SDIFresult  SDIF_Write1TRCVals(FILE *f,
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
SDIFresult SDIF_WriteRowOf1RES(SDIF_RowOf1RES *row, FILE *f);
SDIFresult SDIF_ReadRowOf1RES(SDIF_RowOf1RES *row, FILE *f);


/****** 1FQ0 ******/

typedef struct {
    sdif_float32 freq, confidence;
} SDIF_RowOf1FQ0;

sdif_int32 SDIF_SizeOf1FQ0Frame(int numF0Estimates);
SDIFresult SDIF_WriteRowOf1FQ0(SDIF_RowOf1FQ0 *row, FILE *f);
SDIFresult SDIF_ReadRowOf1FQ0(SDIF_RowOf1FQ0 *row, FILE *f);
