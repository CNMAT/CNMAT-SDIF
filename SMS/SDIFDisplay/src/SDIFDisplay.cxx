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

#include "SDIFDisplay.H"
#include "SDIF++.H"

SDIFDisplay::SDIFDisplay(int X,int Y,int W,int H,SDIFStream* s)
:Fl_Widget(X,Y,W,H) {
	mStream = s;
	
	mHOffset = 0;
	mHScale = 1;
	mVOffset = 0;
	mVScale = 1;

	mColorMapping = 0;
}
