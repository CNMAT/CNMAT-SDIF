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

/* sdif-types.c

   Helper procedures for some of SDIF's standard frame and matrix types.

   SDIF spec: http://www.cnmat.berkeley.edu/SDIF/   

   Split off from sdif.[ch] on 10/11/99
   MW added lots of 1TRC stuff 6/27/2000 
*/


#include <stdio.h>
#include "sdif.h"
#include "sdif-types.h"
#include <math.h>



/* Stupid MSVC feature -AC 7/19/2000 */
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif


sdif_float32 WrapPhase32(sdif_float32 input_phase) {
    sdif_float32 r;

    r = fmod(input_phase, 2 * M_PI);

    if (r <= -M_PI) return r+2*M_PI;
    if (r > M_PI) return r-2*M_PI;
    return r;
}

sdif_float64 WrapPhase64(sdif_float64 input_phase) {
    sdif_float64 r;

    r = fmod(input_phase, 2 * M_PI);

    if (r <= -M_PI) return r+2*M_PI;
    if (r > M_PI) return r-2*M_PI;
    return r;
}

#if 0
static float
ReverseEngineerPhase(float desiredEndingPhase,
		     float constantFreq,
		     float time) {
    /* In a birth, we want to reach the nominal initial phase at the
       end of the interval, so assuming constant frequency over the
       interval, figure out what the initial phase needs to be so we'll
       reach the target. */

    float phaseChange = constantFreq * time * 2.0 * M_PI;

    return fmod(desiredEndingPhase - phaseChange, 2.0*M_PI);
}
#endif


/*****************************************/
/* Some stuff for particular frame types */
/*****************************************/


/* 1TRC */
SDIFresult SDIF_WriteRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f) {
    return SDIF_Write4(row,4,f);
}


SDIFresult SDIF_ReadRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f) {
    return SDIF_Read4(row,4,f);
}


SDIFresult SDIF_Read1TRCVals(FILE *f,
			     sdif_float32 *indexp, sdif_float32 *freqp,
			     sdif_float32 *ampp, sdif_float32 *phasep) {
    SDIF_RowOf1TRC data;

#ifdef LITTLE_ENDIAN
    SDIFresult r;
    if (r = SDIF_Read4(&data, 4, f)) return r;
#else
    if (fread(&data, sizeof(data), 1, f) != 1) return ESDIF_READ_FAILED;
#endif

    *indexp = data.index;
    *freqp = data.freq;
    *ampp = data.amp;
    *phasep = data.phase;

    return ESDIF_SUCCESS;
}


SDIFresult SDIF_Write1TRCVals(FILE *f,
			      sdif_float32 index, sdif_float32 freq,
			      sdif_float32 amp, sdif_float32 phase) {

    SDIFresult r;
    SDIF_RowOf1TRC data;

    data.index = index;
    data.freq = freq;
    data.amp = amp;
    data.phase = phase;

#ifdef LITTLE_ENDIAN
    if (r = SDIF_Write4(&data, 4, f)) return r;
#else
    if (fwrite (&data, sizeof(data), 1, f) != 1) return ESDIF_WRITE_FAILED;
#endif

    return ESDIF_SUCCESS;
}


sdif_int32 SizeOf1TRCFrame(int numTracks) {

  /* 16 bytes for the time stamp, ID and matrix count, plus the size of the
     matrix header, plus four 4-byte floating point numbers (index, freq, amp,
     phase) for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

    return  16 + sizeof(SDIF_MatrixHeader) + 
	(numTracks * sizeof(SDIF_RowOf1TRC));
}



/* 1RES */
SDIFresult SDIF_WriteRowOf1RES(SDIF_RowOf1RES *row, FILE *f) {
    return SDIF_Write4(row,4,f);
}


SDIFresult SDIF_ReadRowOf1RES(SDIF_RowOf1RES *row, FILE *f) {
    return SDIF_Read4(row,4,f);
}


sdif_int32
SDIF_SizeOf1RESFrame(int numResonances) {

  /* 16 bytes for the time stamp, ID and matrix count, plus the size of the
     matrix header, plus four 4-byte floating point numbers (freq, gain, bw
     phase) for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

    return  16 + sizeof(SDIF_MatrixHeader) + 
	(numResonances * sizeof(SDIF_RowOf1RES));
}


/* 1FQ0 */

sdif_int32 SDIF_SizeOf1FQ0Frame(int numF0Estimates) {
    return 16 + sizeof(SDIF_MatrixHeader) + 
	(numF0Estimates * sizeof(SDIF_RowOf1FQ0));
}

SDIFresult SDIF_WriteRowOf1FQ0(SDIF_RowOf1FQ0 *row, FILE *f) {
    return SDIF_Write4(row, 2, f);
}

SDIFresult SDIF_ReadRowOf1FQ0(SDIF_RowOf1FQ0 *row, FILE *f) {
        return SDIF_Read4(row,2,f);
}


