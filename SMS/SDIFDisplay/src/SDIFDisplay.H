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

#ifndef __SDIFDisplay__
#define __SDIFDisplay__

#include <FL/Fl_Widget.H>
#include "SDIF++.H"

class SDIFDisplayColors;

class SDIFDisplay:public Fl_Widget
/*D* Base class for the different SDIF Display classes. */
{
protected:
	SDIFStream* mStream; //D/ Pointer to the SDIF Stream which data is displayed.

	float mHOffset; //D/ Horizontal offset for scrolling and zooming.
	float mHScale;  //D/ Horizontal scale for scrolling and zooming.
	float mVOffset; //D/ Vertical offset for scrolling and zooming.
	float mVScale;  //D/ Vertical scale for scrolling and zooming.

	SDIFDisplayColors* mColorMapping; //D/ Pointer to the color map.
public:
	SDIFDisplay(int X,int Y,int W,int H,SDIFStream* s); 
	//D/ Create a new SDIFDisplay
	
	virtual void draw(void){}
	
	void HScale(float v);
	void HOffset(float v); 
	void VScale(float v);
	void VOffset(float v); 

	SDIFDisplayColors* ColorMapping() { return mColorMapping; }

	double TimeOffset();
	double DTime(void);
};

inline void SDIFDisplay::HScale(float v)
{	
	//D/ Set horizontal scale.
	mHScale = v; 
} 
inline void SDIFDisplay::HOffset(float v) 
{
	//D/ Set horizontal offset.
	mHOffset = v;
} 
inline void SDIFDisplay::VScale(float v)
{
	//D/ Set vertical scale.
	mVScale = v;
}
inline void SDIFDisplay::VOffset(float v) 
{	
	//D/ Set vertical offset.
	mVOffset = v;	
}


inline double SDIFDisplay::TimeOffset()
{
	// Convert the horizontal offset into a time value to access the stream.
	return mStream->First()->Time() + 
	(mStream->Last()->Time()-mStream->First()->Time())*mHOffset;
}

inline double SDIFDisplay::DTime(void)
{
	// Convert the horizontal scale into a time increment to access the stream.
	return 	(mStream->Last()->Time()-mStream->First()->Time())*
		mHScale/double(w());
}

#endif
