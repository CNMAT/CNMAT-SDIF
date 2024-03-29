/* 
Copyright (c) 2004.  The Regents of the University of
California (Regents).  All Rights Reserved.

Written by Ben "Jacobs", The Center for New Music and Audio
Technologies, University of California, Berkeley.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

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

/* sdif-interp-implem.h

  Public interface to implementation of common interpolator functions:
  - 
  
  Future features:
  -

  author: Ben "Jacobs"
  2004/06/22 (0.1.0) Implementation of initial feature set - bj

  NOTES
  
  Assume user has already included these headers:
  "sdif.h", "sdif-mem.h", "sdif-buf.h", "sdif-interp.h"
*/


/**********************************************/
/*                                            */
/* public interface to interpolator functions */
/*                                            */
/**********************************************/

SDIFinterp_InterpolatorFn LinearInterpolator;
SDIFinterp_InterpolatorFn LagrangeInterpolator;
