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
#include "SDIFDisplayFQ0.H"
#include "SDIFDisplayColors.H"
#include <FL/fl_draw.H>

SDIFDisplayFQ0::SDIFDisplayFQ0(int X,int Y,int W,int H,SDIFStream* s)
:SDIFDisplay(X,Y,W,H,s)
{
	mColorMapping = new SDIFDisplayColors(0.5);
}

void SDIFDisplayFQ0::ValuesAtTime(double *freq,double* confidence,double time,
	bool quick)
{
	int i;
	
	while (mStream->Current()->NextInStream() &&
		time>mStream->Current()->NextInStream()->Time()) {
		mStream->Forward();
	}

	for (i=0;i<256;i++) {
		freq[i]=0;
		confidence[i]=0;
	}

	if (!mStream->Current()) {
		return;
	}

	SDIFFrame* left		=	mStream->Current();
	SDIFFrame* right	= left->NextInStream();

	SDIFMatrix* leftm = left->Matrix(0);
	
	if (quick) {
		for (i=0;i<leftm->RowCount() && i<256;i+=8) {
			float f,a=1;
			leftm->Get(0,i,f);
			if (leftm->ColumnCount()>1) {
				leftm->Get(1,i,a);
			}
			freq[i]=f;
			confidence[i]=a;
		}

		return;
	}
	
	double wr = 0;
	if (right) {
		wr = (time - left->Time())/(right->Time() - left->Time());
	}
	double wl = 1.-wr;
	
	for (i=0;i<leftm->RowCount() && i<256;i++) {
		float f,a=1;
		leftm->Get(0,i,f);
		if (leftm->ColumnCount()>1) {
			leftm->Get(1,i,a);
		}
		freq[i]=f;
		confidence[i]=a*wl;
	}

	if (right==NULL) return;

	SDIFMatrix* rightm = right->Matrix(0);

	for (i=0;i<rightm->RowCount() && i<256;i++) {
		float f,a=1;
		rightm->Get(0,i,f);
		if (rightm->ColumnCount()>1) {
			rightm->Get(1,i,a);
		}
		if (freq[i] && f!=0.) freq[i]=freq[i]*wl+f*wr;
		else freq[i]=f;
		confidence[i]+=a*wr;
	}
}

void SDIFDisplayFQ0::draw(void)
{
	int i;
	
	fl_color(FL_BLACK);
	fl_rectf(x(),y(),w(),h());

	mStream->Start();

	double t= TimeOffset();
	double dt= DTime();

	double foffset = FreqOffset();
	double df = DFreq();
	
	int cx=x();

	fl_push_clip(x(),y(),w(),h());

	double freq[256];
	double confidence[256];

	fl_color(FL_WHITE);

	int prevy[256];
	for (i=0;i<256;i++) {
		prevy[i]=0x10000000;
	}
	
	for (cx=x();cx<x()+w();cx++) {
		ValuesAtTime(freq,confidence,t);
		for (i=0;i<256;i++) {
			if (confidence[i]) {
				fl_color(mColorMapping->Get(confidence[i]));
				
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
