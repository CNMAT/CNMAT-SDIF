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

#include "SDIFDisplayColors.H"
#include <math.h>
#include <FL/Fl.h>

int SDIFDisplayColors::inited_=0;

SDIFDisplayColors::SDIFDisplayColors(float v)
{
	InitColors();
	value_=v;
	CalcColorMap();
}

void SDIFDisplayColors::CalcColorMap(void)
{
	//D/ Calculate the conversion color map, depending and the scaling value.
	int i;
	float v = 0.34-value_/3.;
	for (i=0;i<NCOLORMAPPING;i++) {
		colormapping[i]=int(pow(float(i)/float(NCOLORMAPPING),v)*float(NCOLORS));
	}
}

void SDIFDisplayColors::InitColors(void)
{
	//D/ Create the colors in the FLTK colormap
	if (inited_) return;
	inited_=1;

	int ColorsIndex[6];

	ColorsIndex[0] = 0;
	ColorsIndex[1] = 12;
	ColorsIndex[2] = 25;
	ColorsIndex[3] = 37;
	ColorsIndex[4] = 50;
	ColorsIndex[5] = 64;

	Fl::set_color((Fl_Color)(FIRST_INDEX+ColorsIndex[0]),0,0,0);
	Fl::set_color((Fl_Color)(FIRST_INDEX+ColorsIndex[1]),15,17,16);
	Fl::set_color((Fl_Color)(FIRST_INDEX+ColorsIndex[2]),80,100,153);
	Fl::set_color((Fl_Color)(FIRST_INDEX+ColorsIndex[3]),90,180,100);
	Fl::set_color((Fl_Color)(FIRST_INDEX+ColorsIndex[4]),224,224,44);
	Fl::set_color((Fl_Color)(FIRST_INDEX+ColorsIndex[5]),255,155,80);

	int n = 0;
	int nDif = 0;
	for(int k=0; k<NCOLORS; k+=nDif,n++)
	{
		nDif = ColorsIndex[n+1] - ColorsIndex[n];
		for(int i=1; i<nDif; i++)
		{
			uchar R,G,B;
			uchar R1,G1,B1;
			uchar R2,G2,B2;
			Fl::get_color((Fl_Color) (FIRST_INDEX + k), R1, G1, B1);
			Fl::get_color((Fl_Color) (FIRST_INDEX + k+nDif), R2, G2, B2);

			float factor = (float)i / nDif;
			R = R1 + uchar((R2 - R1)*factor);
			G = G1 + uchar((G2 - G1)*factor);
			B = B1 + uchar((B2 - B1)*factor);

			Fl::set_color((Fl_Color)(FIRST_INDEX+k+i),R,G,B);
		}
	}
}
