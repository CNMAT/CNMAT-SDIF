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

#include <math.h>

#include <Fl/Fl_Double_Window.H>
#include <Fl/Fl_Return_Button.H>
#include <Fl/Fl_Box.H>
#include <Fl/Fl.H>
#include <Fl/fl_file_chooser.H>
#include <Fl/filename.H>
#include <Fl/fl_ask.H>

#include "SDIFDisplayContainer.H"

SDIFDisplayContainer* d1;
SDIFDisplayContainer* d2;
SDIFDisplayContainer* d3;

void sync(SDIFDisplayContainer* d,float scale,float offset)
{
		d->hslider->scale(scale);
		d->hslider->offset(offset);
		d->display->HOffset(d->hslider->offset());
		d->display->HScale(d->hslider->scale());
		d->hslider->redraw();
		d->display->redraw();
}

void dccb(SDIFDisplayContainer* d)
{
	if (Fl::event_state(FL_CTRL)) {
		if (d!=d1) {
			sync(d1,d->hslider->scale(),d->hslider->offset());
		}
		if (d!=d2) {
			sync(d2,d->hslider->scale(),d->hslider->offset());
		}
		if (d!=d3) {
			sync(d3,d->hslider->scale(),d->hslider->offset());
		}
	}
}


void my_fl_alert_cb(Fl_Widget* w)
{
	w->window()->hide();
}

void my_fl_alert(char* str)
{
	Fl_Window* w = new Fl_Window(300,80,"SDIFDisplay Error");
	w->set_modal();
	Fl_Box* icon = new Fl_Box(5,5,40,40,"!");
	icon->box(FL_THIN_UP_BOX);
	icon->color(FL_WHITE);
	icon->labelfont(FL_TIMES_BOLD);
	icon->labelsize(24);
	icon->labelcolor(FL_BLUE);
	Fl_Box* box = new Fl_Box(50,5,245,40,str);
	box->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
	Fl_Button* b = new Fl_Return_Button(220,50,75,25,"OK");
	b->callback(my_fl_alert_cb);
	w->end();
	w->show();
	while (w->visible()) {
		Fl::wait();
	}
	delete w;
}

main(int argc,char** argv)
{
	char* filename = 0;
	
	if (argc>1) {
		filename = argv[1];
	}
	if (filename==0) {
		filename = fl_file_chooser("Select SDIF file","*.{sdf|sdif}","");
		if (filename==0 || strcmp(filename,"")==0 || filename_isdir(filename)) {
			my_fl_alert("No SDIF file selected, exiting\n");
			return 0;
		}
	}
	
	SDIFFile file(filename,read); 
	file.Read();
	if (!file.Ok()) {
		my_fl_alert("Error reading file\n");
		return 0;
	}

	Fl::own_colormap();
	
	SDIFStream streamSTF(file.First(),"1STF");
	SDIFStream streamTRC(file.First(),"1TRC");
	SDIFStream streamFQ0(file.First(),"1FQ0");

	Fl_Double_Window w(450,450,"SDIF");
	
	d1 = 
		new SDIFDisplayContainer(0,0,450,150,"Pitch",
			SDIFStandardTypeFQ0,&streamFQ0);
	d2 = 
		new SDIFDisplayContainer(0,150,450,150,"Sinusoids",
			SDIFStandardTypeTRC,&streamTRC);
	d3 = 
		new SDIFDisplayContainer(0,300,450,150,"Residual",
			SDIFStandardTypeSTF,&streamSTF);

	d1->callback((Fl_Callback*)dccb);
	d2->callback((Fl_Callback*)dccb);
	d3->callback((Fl_Callback*)dccb);

	w.add(d1);
	w.add(d2);
	w.add(d3);

	w.resizable(w);
	
	w.end();
	w.show();

	Fl::run();
	
	return 0;
}
