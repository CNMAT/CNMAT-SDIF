/* 

Copyright (c) 1997,1998,1999, 2004.  The Regents of the University of California
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

 capital-F0tosdif.c

 converts "Capital F-zero" files (pitch,loudness,confidence) to SDIF

 Puts the amplitude estimate into the third matrix column.

 Amar Chaudhary, 1/13/98
 Matt Wright 10/12/99: new SDIF library
 Matt Wright 12/14/02: renamed because of OSX case insensitivity

*/

#include <stdio.h>
#include <string.h>
#include "sdif.h"



int main (int argc, char *argv[]) {
  SDIFresult r;
  int	i;
  FILE	*infile,*outfile;
  float	timeval,pitchval,confidence,amp;
  char	*extension, outfname[1024];
  SDIF_FrameHeader head;
  SDIF_MatrixHeader mhead;
  float pad = 0.0f;

  for (i = 1; i < argc; ++i){
    infile = fopen(argv[i],"rt");
    if (infile == NULL) {
      fprintf (stderr,"file not found: %s\n",argv[i]);
      return -1;
    }
    
    strcpy(outfname,argv[i]);
    extension = strrchr(outfname,'.');
    strcpy(extension,".sdif");
    
    if (r = SDIF_OpenWrite(outfname, &outfile)) {
      fprintf (stderr, "Error creating %s: %s\n",outfname,
	       SDIF_GetErrorString(r));
      fclose(infile);
      return -1;
    }
    
    SDIF_Copy4Bytes(head.frameType,"1FQ0");
    head.streamID = SDIF_UniqueStreamID();
    head.matrixCount = 1;

        
    mhead.rowCount = 1;
    mhead.columnCount = 3;
    SDIF_Copy4Bytes(mhead.matrixType,"1FQ0");
    mhead.matrixDataType = SDIF_FLOAT32;
    head.size = 16 + 32 + (1 * 3 * 4) + /*padding */ 4;

    while (fscanf(infile,"%f %f %f %f",&timeval,&pitchval,&amp,&confidence) == 4) {
	head.time = timeval;
	SDIF_WriteFrameHeader(&head, outfile);
	SDIF_WriteMatrixHeader(&mhead, outfile);
	SDIF_Write4(&pitchval,1,outfile);
	SDIF_Write4(&confidence,1,outfile);
	amp /= 32768.0f;
	SDIF_Write4(&amp,1,outfile);
	/* write out 4-bytes of padding */
	SDIF_Write4(&pad,1,outfile);
    }

    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
