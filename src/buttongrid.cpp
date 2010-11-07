/*buttongrid.cpp is part of lucifer a laser show controller.

Copyrignt 2010 Dan Mills <dmills@exponent.myzen.co.uk>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 dated June, 1991.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* $Id: buttongrid.cpp 22 2010-07-24 15:01:51Z dmills $ */

#include <assert.h>
#include "log.h"
#include "buttongrid.h"
#include "framesource.h"
#include "screendisplay.h"

ButtonGrid::ButtonGrid (unsigned int num_x, unsigned int num_y, unsigned int id, QWidget* parent):
        QWidget(parent)
{
    num_x_ = num_x;
    num_y_ = num_y;
    id_=id;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout = new QGridLayout (this);
    layout->setHorizontalSpacing(5);
    layout->setVerticalSpacing(5);
    mapper = new QSignalMapper (this);
    connect (mapper,SIGNAL(mapped(int)),this,SLOT(stateChanged(int)));
    for (unsigned int x = 0; x < num_x_; x++) {
        for (unsigned int y = 0; y < num_y_; y++) {
            ScreenDisplay *s = new ScreenDisplay;
            connect (s,SIGNAL(modified()),this,SLOT(mod()));
            layout->addWidget(s, x,y);
            mapper->setMapping(s,y*num_x_ + x);
            connect (s,SIGNAL(stateChanged(bool)),mapper,SLOT(map()));
        }
    }
}

ButtonGrid::~ButtonGrid ()
{
    for (unsigned int x = 0; x < num_x_; x++) {
        for (unsigned int y = 0; y < num_y_; y++) {
            delete layout->itemAtPosition (x,y)->widget();
        }
    }
}

void ButtonGrid::clear(int except_x, int except_y)
{
    for (int x=0; x < (int)num_x_; x++) {
        for (int y=0; y < (int)num_y_; y++) {
            if ((x != except_x) || (y!= except_y)) {
                ScreenDisplay *s = at (x,y);
                if (s) {
                    s->resetSelected(false);
                }
            }
        }
    }
}

void ButtonGrid::stateChanged(int pos)
{
    unsigned int x = pos %num_x_;
    unsigned int y = pos /num_x_;
    ScreenDisplay *s = at(x,y);
    if (s) {
        emit clicked(x,y,id_,s->selected());
        //std::cerr << "Clicked at "<<id_ << ":" << x <<":"<<y<<":"<<s->selected()<<std::endl;
    }
}

void ButtonGrid::setState(unsigned int x, unsigned int y, bool selected)
{
    ScreenDisplay *s = at(x,y);
    if (s) {
        s->setSelected(selected);
    }
}

//Marshals modified signals from the ScreenDisplays
void ButtonGrid::mod () {
    emit modified();
}
void ButtonGrid::dimensions(int &x, int &y)
{
    x = num_x_;
    y = num_y_;
}


ScreenDisplay * ButtonGrid::at (unsigned int x, unsigned int y)
{
    if ((x >= num_x_) || (y >= num_y_)) return NULL;
    return (ScreenDisplay *) layout->itemAtPosition(x,y)->widget();
}

void ButtonGrid::save (QXmlStreamWriter *w)
{
    assert (w);
    w->writeStartElement("Grid");
    w->writeAttribute("X",QString().number(num_x_));
    w->writeAttribute("Y",QString().number(num_y_));
    for (unsigned int y=0; y <num_y_; y++) {
        for (unsigned x=0; x < num_x_; x++) {
            ScreenDisplay *s = at (x,y);
            if (s &&(s->data())) {
                // Output the co ordinates in the grid
                w->writeStartElement("Sequence");
                w->writeAttribute("Pos_X",QString().number(x));
                w->writeAttribute("Pos_Y",QString().number(y));
                // Call down into the framesources to output the contents
                FrameSourcePtr fs = s->data();
                assert (fs);
                fs->saveFrames (w);
                w->writeEndElement();
            }
        }
    }
    w->writeEndElement(); // Grid
}

ButtonGrid * ButtonGrid::load(QXmlStreamReader* e, unsigned int id, QWidget * parent)
{
    assert (e);
    assert (e->name().toString() == "Grid");
    int x=8,y=8;
    x=e->attributes().value("X").toString().toInt();
    y=e->attributes().value("Y").toString().toInt();
    slog()->infoStream()<<"Creating new button grid dimensions : " << x <<" : " << y;
    ButtonGrid *g = new ButtonGrid (x,y,id,parent);
    while ((!e->atEnd()) && (!e->isEndDocument())) {
        if (e->isEndElement()) {
            slog()->infoStream() << "ButtonGrid found end element : " << e->name().toString().toStdString();
            if (e->name().toString() == "Grid") {
                e->readNext();
                return g;
            }
        }
        while ((!e->atEnd()) && (!e->isStartElement()) && (e->name().toString() != "Sequence")) {
            slog()->infoStream() << "Grid loader looking for a sequence StartElement tag, got : "<<
            e->tokenType() << " : " << e->name().toString().toStdString();
            if ((e->isEndElement() && (e->name().toString()=="Grid"))) {
                e->readNext();
                return g;
            }
            e->readNext();
        }
        if (e->isStartElement() && (e->name() == "Sequence")) {
            // something to load into one of the buttons in the grid
            int px = 0;
            int py = 0;
            px = e->attributes().value("Pos_X").toString().toInt();
            py = e->attributes().value("Pos_Y").toString().toInt();
            slog()->infoStream()<<"Loading sequence at : " << px << " : " << py;
            e->readNextStartElement();
            assert (e->tokenType() == QXmlStreamReader::StartElement);
            FrameSourcePtr fs = FrameSource::loadFrames (e);
            ScreenDisplay *s = g->at(px,py);
            assert (s);
            s->source (fs);
        } else {
            e->readNextStartElement();
        }
    }
    return g;
}

