/* 


Copyright (c) 1997, 1998, 1999.  The Regents of the University of California
(Regents).  All Rights Reserved.


Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.


Written by Amar Chaudhary, The Center for New Music and Audio Technologies,
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
*/



/*
 res2sdif.c


 converts resonance text files (rows of frequency, amplitude, bandwidth) to SDIF
 Now supports the latest version of SDIF, as of 7/16/99


 Amar Chaudhary, 7/16/99
 updated by Matt Wright 9/22/99
 updated by Matt Wright 10/12/99 for new SDIF library
 updated by Amar Chaudhary 1/2001 to handle both linear and dB amplitudes
          use "-linear" option for linear amplitudes

*/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sdif.h"
#include "sdif-mem.h"
#include "sdif-types.h"


/* Stupid MSVC feature -AC 7/19/2000 */
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef stricmp
#define stricmp strcasecmp
#endif

SDIF_RowOf1RES res[4096];


int main (int argc, char *argv[]) {
  SDIFresult r;
  int   i,j;
  FILE  *infile,*outfile;
  float logamp,frequency,bandwidth;
  char  *extension, outfname[1024];
  SDIF_FrameHeader head;
  SDIF_MatrixHeader mhead;

  int newstyle = 0;

  for (i = 1; i < argc; ++i){
    if (!stricmp(argv[i],"-linear")) {
      newstyle = 1;
      continue;
    }
    infile = fopen(argv[i],"rt");
    if (infile == NULL) {
      fprintf (stderr,"can't open %s\n",argv[i]);
      return -1;
    }
    
    strcpy(outfname,argv[i]);
    extension = strrchr(outfname,'.');
    strcpy(extension,".res.sdif");
    
    if (r = SDIF_OpenWrite(outfname, &outfile)) {
      fprintf (stderr, "Error creating %s: %s\n", outfname, 
               SDIF_GetErrorString(r));
      fclose(infile);
      return -1;
    }
    
    SDIF_Copy4Bytes(head.frameType,"1RES");
    head.streamID = SDIF_UniqueStreamID();
    head.matrixCount = 1;
    head.time = 0.0;
    
    j = 0;
    while (fscanf(infile,"%f %f %f",&frequency,&logamp,&bandwidth) == 3) {
      res[j].freq = frequency;
      if (newstyle) {
	res[j].amp = logamp;
	res[j].decayrate = bandwidth;
      } else {
	res[j].amp = pow(10.0, logamp * 0.05);
	res[j].decayrate = M_PI * bandwidth;
      }
      res[j].phase = 0.0f;
      ++j;
    }
    
    if (j > 0) {
      mhead.rowCount = j;
      mhead.columnCount = 4;
      SDIF_Copy4Bytes(mhead.matrixType,"1RES");
      mhead.matrixDataType = SDIF_FLOAT32;


      head.size = SDIF_SizeOf1RESFrame(j);
      SDIF_WriteFrameHeader(&head,outfile);
      SDIF_WriteMatrixHeader(&mhead, outfile);
      SDIF_Write4(res,j*4,outfile);
    }
    
    fclose(infile);
    fclose(outfile);
  }


  return 0;
}
