/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************
 *   Maarten de Boer <maarten.deboer@iua.upf.es>, 1999                     *
 *   Music Technology Group                                                *
 *   Institut de l'Audiovisual, Universitat Pompeu Fabra, Barcelona, Spain *
 *   http://www.iua.upf.es/mtg/                                            *
 ***************************************************************************/

#ifndef __SDIFDisplayFQ0__
#define __SDIFDisplayFQ0__

#include "SDIFDisplay.H"

class SDIFStream;

class SDIFDisplayFQ0:public SDIFDisplay
{
//D/ Class to display an SDIF Fundamental Frequency Estimates stream.
public:
	SDIFDisplayFQ0(int X,int Y,int W,int H,SDIFStream* s);
	void draw(void);

	void ValuesAtTime(double *freq,double* confidence,double time,bool quick=0);

	double FreqOffset() {
		return 1000.*(mVOffset);
	}
	
	double DFreq(void) {
		return 	1000.*(mVScale)/double(h());
	}

};

#endif
