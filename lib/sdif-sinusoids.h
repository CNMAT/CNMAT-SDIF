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


typedef struct {
    sdif_float32 index;
    sdif_float32 freq;
    sdif_float32 amp;
    sdif_float32 phase;
} Sinusoid;

typedef struct {
    int n;
    Sinusoid *s;
    sdif_float64 t;
} *sinusoids;


typedef enum {
    NORMAL,	/* exists in both frames */
    GOOD_BIRTH,	/* not in current; 0 amp in next */
    BAD_BIRTH,	/* not in current; non-0 amp in next */
    GOOD_DEATH,	/* 0 amp in current; not in next */
    BAD_DEATH	/* non-0 amp in current; not in next */
} SineStatus;

typedef struct {
    Sinusoid *before;
    Sinusoid *after;
    SineStatus status;
} SinusoidBetweenFrames;

typedef struct {
    SinusoidBetweenFrames *sbf;
    int n;			    /* # elements in sbf */
    int size;			    /* Size of sbf array */
    sdif_float64 begintime, endtime; /* Time tags of the two frames */
} TwoFrames;

sinusoids AllocSinusoids(int n);
void FreeSinusoids(sinusoids s);
void PrintSinusoids(sinusoids s);
int AnyNonZeroAmplitudes(sinusoids s);
sinusoids FrameToSinusoids(SDIFmem_Frame frame);
char *SineStatusAsString(SineStatus s);
TwoFrames *MakeTwoFrames(sinusoids begin, sinusoids end);
void FreeTwoFrames(TwoFrames *x);
void PrintTwoFrames(TwoFrames *x);
