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
#include "SDIFDisplayColors.H"
#include "SDIFDisplaySTF.H"
#include <FL/fl_draw.H>
#include <math.h>

SDIFDisplaySTF::SDIFDisplaySTF(int X,int Y,int W,int H,SDIFStream* s)
:SDIFDisplay(X,Y,W,H,s) {
	mColorMapping = new SDIFDisplayColors(0.5);
}

void SDIFDisplaySTF::ValuesAtTime(double* amp,double time,bool quick)
{
	int i;

	while (mStream->Current()->NextInStream() &&
		time>mStream->Current()->NextInStream()->Time()) {
		mStream->Forward();
	}

	for (i=0;i<256;i++) {
		amp[i]=0;
	}

	if (!mStream->Current()) {
		return;
	}

	SDIFFrame* left		=	mStream->Current();
	SDIFFrame* right	= left->NextInStream();

	SDIFMatrix* leftm = left->Matrix(1);
	
	if (quick) {
		for (i=0;i<leftm->RowCount();i+=8) {
			float re,im;
			leftm->Get(0,i,re);
			leftm->Get(1,i,im);
			float a = sqrt(re * re + im * im);
			amp[i]=a;
		}
		return;	
	}
	
	double wr = 0;
	if (right) {
		wr = (time - left->Time())/(right->Time() - left->Time());
	}
	double wl = 1.-wr;
	
	for (i=0;i<leftm->RowCount();i++) {
		float re,im;
		leftm->Get(0,i,re);
		leftm->Get(1,i,im);
		float a = sqrt(re * re + im * im);
		amp[i]=a*wl;
	}

	if (right==NULL) return;

	SDIFMatrix* rightm = right->Matrix(1);

	for (i=0;i<rightm->RowCount();i++) {
		float re,im;
		rightm->Get(0,i,re);
		rightm->Get(1,i,im);
		float a = sqrt(re * re + im * im);
		amp[i]+=a*wr;
	}
}

void SDIFDisplaySTF::draw(void)
{
	int i;

	mStream->Start();
	
	int n=mStream->Current()->Matrix(1)->RowCount();
	
	double t= TimeOffset();
	double dt= DTime();
	
	double dv = (h()/float(n))/mVScale;

	fl_color(FL_BLACK);
	fl_rectf(x(),y(),w(),h());
	
	int cx=x();

	fl_push_clip(x(),y(),w(),h());

	double amp[256];

	fl_color(FL_WHITE);

	for (cx=x();cx<x()+w();cx++) {
		ValuesAtTime(amp,t);
		for (i=0;i<n;i++) {
			int k;
			k=i+int(float(n)*mVOffset);
			if (k>=0 && k<n && amp[k]) {
				if (amp[k]<0) amp[k]=0;
				if (amp[k]>1) amp[k]=1;

				fl_color(mColorMapping->Get(amp[k]));

				int cy1 = y()+h()-int(i*dv);
				int cy2 = y()+h()-int((i+1)*dv);
				fl_yxline(cx,cy1,cy2);
			}
		}
		t+=dt;
	}
	fl_pop_clip();	
}
