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

#include "SDIF++.H"
#include "SDIFDisplayTRC.H"
#include "SDIFDisplayColors.H"
#include <FL/fl_draw.H>

#include "SDIFDisplayColors.H"

SDIFDisplayTRC::SDIFDisplayTRC(int X,int Y,int W,int H,SDIFStream* s)
:SDIFDisplay(X,Y,W,H,s) {
	mColorMapping = new SDIFDisplayColors(0.3);
}

void SDIFDisplayTRC::ValuesAtTime(double *freq,double* amp,double time,
	bool quick)
{
	int i;
	
	while (mStream->Current()->NextInStream() &&
		time>mStream->Current()->NextInStream()->Time()) {
		mStream->Forward();
	}

	for (i=0;i<256;i++) {
		freq[i]=0;
		amp[i]=0;
	}

	if (!mStream->Current()) {
		return;
	}

	SDIFFrame* left		=	mStream->Current();
	SDIFFrame* right	= left->NextInStream();

	SDIFMatrix* leftm = left->Matrix(0);
	
	if (quick) {
		for (i=0;i<leftm->RowCount();i+=8) {
			sdif_int32 id;
			float f,a;
			leftm->Get(0,i,id);
			leftm->Get(1,i,f);
			leftm->Get(2,i,a);
			freq[id]=f;
			amp[id]=a;
		}

		return;
	}
	
	double wr = 0;
	if (right) {
		wr = (time - left->Time())/(right->Time() - left->Time());
	}
	double wl = 1.-wr;
	
	for (i=0;i<leftm->RowCount();i++) {
		sdif_int32 id;
		float f,a;
		leftm->Get(0,i,id);
		leftm->Get(1,i,f);
		leftm->Get(2,i,a);
		freq[id]=f;
		amp[id]=a*wl;
	}

	if (right==NULL) return;

	SDIFMatrix* rightm = right->Matrix(0);

	for (i=0;i<rightm->RowCount();i++) {
		sdif_int32 id;
		float f,a;
		rightm->Get(0,i,id);
		rightm->Get(1,i,f);
		rightm->Get(2,i,a);
		if (freq[id] && f!=0.) freq[id]=freq[id]*wl+f*wr;
		else freq[id]=f;
		amp[id]+=a*wr;
	}
}

void SDIFDisplayTRC::draw(void)
{
	fl_color(FL_BLACK);
	fl_rectf(x(),y(),w(),h());

	mStream->Start();

	double t= TimeOffset();
	double dt= DTime();

	double foffset = FreqOffset();
	double df = DFreq();
	
	int i;
	int cx=x();

	fl_push_clip(x(),y(),w(),h());

	double freq[256];
	double amp[256];

	fl_color(FL_WHITE);

	int prevy[256];
	for (i=0;i<256;i++) {
		prevy[i]=0x10000000;
	}
	
	for (cx=x();cx<x()+w();cx++) {
		ValuesAtTime(freq,amp,t);
		for (i=0;i<256;i++) {
			if (amp[i]) {
				fl_color(mColorMapping->Get(amp[i]));
				
				int cy = y()+h()-int((freq[i]-foffset)/df);
				if (prevy[i]!=0x10000000) {
					fl_yxline(cx-1,cy,prevy[i]);
				}else{
					fl_yxline(cx,cy,cy);
				}
				prevy[i]=cy;
			}else{
				prevy[i]=0x10000000;
			}
		}
		t+=dt;
	}
	fl_pop_clip();	
}
