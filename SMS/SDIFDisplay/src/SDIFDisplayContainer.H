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

#ifndef __SDIFDISPLAYCONTAINER__
#define __SDIFDISPLAYCONTAINER__

#include <Fl/Fl_Group.H>

enum SDIFStandardType
{
	SDIFStandardTypeSTF = 1,
	SDIFStandardTypeTRC = 2,
	SDIFStandardTypeFQ0 = 3
};

#include <Fl/Fl_Slider.H>
#include <Fl/Fl_Button.H>
#include "SDIFDisplay.H"
#include "SDIF++.H"
#include "ZoomSlider.H"

class SDIFDisplayContainer:public Fl_Group
{
/*D* 
	Widget Group Class that contains an SDIFDisplay, ZoomSliders, and 
	a slider to control the color-scale, and the callbacks necesary
	to interface between them.
*/
public:
	ZoomSlider* hslider;
	ZoomSlider* vslider;
	Fl_Slider* colorsslider;
	Fl_Button *vzoomextra;
	SDIFDisplay* display;
protected:
	static void hcb(ZoomSlider* s,SDIFDisplayContainer* d);
	static void vcb(ZoomSlider* s,SDIFDisplayContainer* d);
	static void vzcb(Fl_Button* b,SDIFDisplayContainer* d);
	static void colcb(Fl_Slider* s,SDIFDisplayContainer* d);
public:
	SDIFDisplayContainer(
		int X,int Y,int W,int H,char* title,
		SDIFStandardType t,SDIFStream* s);
	/*D* Construct a SDIFDisplayContainer. You pass the type of
			 data that will be displayed, and a pointer to the stream.
	*/
};

#endif
