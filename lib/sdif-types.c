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

/* sdif-types.c

   Helper procedures for some of SDIF's standard frame and matrix types.

   SDIF spec: http://www.cnmat.berkeley.edu/SDIF/   

   Split off from sdif.[ch] on 10/11/99
*/


/*****************************************/
/* Some stuff for particular frame types */
/*****************************************/


/* 1TRC */
int
SDIF_WriteRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f) {
    return (SDIF_Write4(row,4,f) == 4);
}


int
SDIF_ReadRowOf1TRC(SDIF_RowOf1TRC *row, FILE *f) {
    return SDIF_Read4(row,4,f) == 4;
}


int
SDIF_Read1TRCVals(FILE *f,
		  sdif_float32 *indexp, sdif_float32 *freqp,
                      sdif_float32 *ampp, sdif_float32 *phasep) {
    SDIF_RowOf1TRC data;

#ifdef LITTLE_ENDIAN
    if (SDIF_Read4(&data, 4, f) != 1) return -1;
#else
    if (fread(&data, sizeof(data), 1, f) != 1) return -1;
#endif

    *indexp = data.index;
    *freqp = data.freq;
    *ampp = data.amp;
    *phasep = data.phase;

    return 0;

}


int
SDIF_Write1TRCVals(FILE *f,
		   sdif_float32 index, sdif_float32 freq,
		   sdif_float32 amp, sdif_float32 phase) {

    SDIF_RowOf1TRC data;

    data.index = index;
    data.freq = freq;
    data.amp = amp;
    data.phase = phase;

#ifdef LITTLE_ENDIAN
    if (write4(&data, 4, f) != 1) return -1;
#else
    if (fwrite (&data, sizeof(data), 1, f) != 1) return -1;
#endif

    return 0;

}


sdif_int32
SizeOf1TRCFrame(int numTracks) {

  /* 16 bytes for the time stamp, ID and matrix count, plus 16 bytes for
     the # rows, # columns, matrix type and matrix data type,
     plus four 4-byte floating point numbers (index, freq, amp, phase)
     for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

    return  16 + 16 + (4 * 4 * numTracks);

}


/* 1RES */
int
SDIF_WriteRowOf1RES(SDIF_RowOf1RES *row, FILE *f) {
    return (SDIF_Write4(row,4,f) == 4);
}


int
SDIF_ReadRowOf1RES(SDIF_RowOf1RES *row, FILE *f) {
    return SDIF_Read4(row,4,f) == 4;
}


sdif_int32
SDIF_SizeOf1RESFrame(int numResonances) {

  /* 16 bytes for the time stamp, ID and matrix count, plus 16 bytes for
     the # rows, # columns, matrix type and matrix data type,
     plus four 4-byte floating point numbers (freq, gain, bw phase)
     for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

    return  16 + 16 + (4 * 4 * numResonances);

}

