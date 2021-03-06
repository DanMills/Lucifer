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

#include <assert.h>
#include "log.h"
#include "buttongrid.h"
#include "framesource.h"
#include "screendisplay.h"
#include <boost/make_shared.hpp>

ButtonGrid::ButtonGrid (EnginePtr engine_, unsigned int num_x, unsigned int num_y, unsigned int offset, QWidget* parent):
        QWidget(parent)
{
    num_x_ = num_x;
    num_y_ = num_y;
    offset_=offset;
    engine = engine_;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *l = new QVBoxLayout(this);
    layout = new QGridLayout ();
    statusbar = new QStatusBar(this);
    statusbar->setSizeGripEnabled(false);
    l->addLayout(layout);
    l->addWidget(statusbar);
    ppsLabel = new QLabel();
    framesLabel = new QLabel();
    statusbar->addPermanentWidget(new QLabel("PPS"));
    statusbar->addPermanentWidget(ppsLabel);
    statusbar->addPermanentWidget(new QLabel("Frames"));
    statusbar->addPermanentWidget(framesLabel);

    ppsLabel->setNum(00000);
    framesLabel->setNum(0000);

    layout->setHorizontalSpacing(5);
    layout->setVerticalSpacing(5);
    connect (&*engine,SIGNAL(setIndicator(uint,QColor)),this,SLOT(selectionChangedData(uint,QColor)));
    for (unsigned int x = 0; x < num_x_; x++) {
        for (unsigned int y = 0; y < num_y_; y++) {
            ScreenDisplay *s = new ScreenDisplay (engine,offset + y*num_x_ + x,this);
            layout->addWidget(s, x,y);
            frameSourceChanged(offset + x + (y * num_x_));
            connect (s,SIGNAL(message(QString,int)),this,SLOT(message(QString,int)));
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

void ButtonGrid::frameSourceChanged(long unsigned int pos)
{
    if ((pos >= offset_) && ((pos - offset_) <(num_x_ * num_y_))) {
        // it is one of ours
        pos -= offset_;
        unsigned int x = pos % num_x_;
        unsigned int y = pos / num_x_;
        ScreenDisplay *s = at (x,y);
        if (s) {
            PlaybackPtr pb = engine->getPlayback(pos+offset_);
            s->source(pb);
        }
    }
}

void ButtonGrid::message(QString text,int time)
{
    statusbar->showMessage(text,time);
}


void ButtonGrid::selectionChangedData(unsigned int pos, QColor col)
{
    if ((pos >= offset_) && ((pos - offset_) <(num_x_ * num_y_))) {
        // it is one of ours
        pos -= offset_;
        unsigned int x = pos % num_x_;
        unsigned int y = pos / num_x_;
        ScreenDisplay *s = at (x,y);
        if (s) {
            s->setIndicatorColour(col);
        }
    }
}

