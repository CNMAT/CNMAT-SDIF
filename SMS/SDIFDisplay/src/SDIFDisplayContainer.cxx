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

#include "SDIFDisplayContainer.H"
#include <Fl/Fl_Slider.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Box.H>
#include "ZoomSlider.H"
#include "SDIFDisplayTRC.H"
#include "SDIFDisplaySTF.H"
#include "SDIFDisplayFQ0.H"
#include "SDIFDisplayColors.H"

SDIFDisplayContainer::SDIFDisplayContainer(
	int X,int Y,int W,int H,char* title,
	SDIFStandardType t,SDIFStream* s)
:Fl_Group(X,Y,W,H){

	hslider = new ZoomSlider(X,Y+H-20,W-20,20);
	hslider->type(FL_HORIZONTAL);

	{
		Fl_Group* g = new Fl_Group(X+W-20,Y+20,20,H-20);

		vslider = new ZoomSlider(X+W-20,Y+20,20,H-60);

		vzoomextra = new Fl_Button(X+W-20+2,Y+H-40+2,16,16);
		vzoomextra->box(FL_THIN_UP_BOX);
		vzoomextra->type(FL_TOGGLE_BUTTON);

		g->resizable(vslider);
		g->end();
	}
	display = 0;

	switch (t) {
	case SDIFStandardTypeSTF:
		display = new SDIFDisplaySTF(X,Y+20,W-20,H-40,s);
		break;
	case SDIFStandardTypeFQ0:
		display = new SDIFDisplayFQ0(X,Y+20,W-20,H-40,s);
		break;
	case SDIFStandardTypeTRC:
		display = new SDIFDisplayTRC(X,Y+20,W-20,H-40,s);
		break;
	}

	{
		Fl_Group* g = new Fl_Group(X,Y,W,20);

		Fl_Box* tb = new Fl_Box(X,Y+4,100,16,title);
		tb->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
		tb->labelsize(12);

		Fl_Box* b = new Fl_Box(X+100,Y+4,W-200,16);
		g->resizable(b);
	
		if (display->ColorMapping()) {
			colorsslider = new Fl_Slider(X+W-100,Y+4,100,16);
			colorsslider->box(FL_THIN_DOWN_BOX);
			colorsslider->type(FL_HORIZONTAL);
			colorsslider->callback((Fl_Callback*)colcb,this);
			colorsslider->value(display->ColorMapping()->value());
		}else{
			colorsslider=0;
		}	

		g->end();
	}

	hslider->callback((Fl_Callback*)hcb,this);
	vslider->callback((Fl_Callback*)vcb,this);
	vzoomextra->callback((Fl_Callback*)vzcb,this);

	resizable(display);

	end();
}

void SDIFDisplayContainer::hcb(ZoomSlider* s,SDIFDisplayContainer* d)
{
	d->display->HOffset(s->offset());
	d->display->HScale(s->scale());
	d->display->redraw();

	d->do_callback();
}

void SDIFDisplayContainer::vcb(ZoomSlider* s,SDIFDisplayContainer* d)
{
	d->display->VOffset(s->offset());
	d->display->VScale(
		(d->vzoomextra->value()?d->vslider->scale()*d->vslider->scale():1)*
		d->vslider->scale());
	d->display->redraw();
}

void SDIFDisplayContainer::vzcb(Fl_Button* b,SDIFDisplayContainer* d)
{	
	d->display->VOffset(d->vslider->offset());
	d->display->VScale(
		(d->vzoomextra->value()?d->vslider->scale()*d->vslider->scale():1)*
		d->vslider->scale());
	d->vslider->redraw();
	d->display->redraw();
}

void SDIFDisplayContainer::colcb(Fl_Slider* s,SDIFDisplayContainer* d)
{
	d->display->ColorMapping()->value(s->value());
	d->display->redraw();
}
