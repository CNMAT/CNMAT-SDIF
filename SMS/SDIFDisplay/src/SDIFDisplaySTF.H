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

#ifndef __SDIFDisplaySTF__
#define __SDIFDisplaySTF__

#include "SDIFDisplay.H"

class SDIFStream;

class SDIFDisplaySTF:public SDIFDisplay
//D/ Class to display an SDIF Short-Term Fourier Transform stream.
{
protected:
	void ValuesAtTime(double* amp,double time,bool quick=0);
public:
	SDIFDisplaySTF(int X,int Y,int W,int H,SDIFStream* s);
	void draw(void);
};

#endif
