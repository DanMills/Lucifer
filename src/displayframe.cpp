/*Displayframe.cpp is part of lucifer a laser show controller.

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

/* $Id: displayframe.cpp 22 2010-07-24 15:01:51Z dmills $ */

#include <QtGui>
#include <iostream>
#include "frame.h"
#include "displayframe.h"


DisplayFrame::DisplayFrame(QWidget* parent): QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setMinimumSize(50,50);
    bw = 0;
    iw = 0;
    bc = QColor (Qt::black);
    ic = QColor (Qt::red);
}

DisplayFrame::~DisplayFrame ()
{
    points.reset();
}

void DisplayFrame::clear()
{
    points = FramePtr();
    update();
}

void DisplayFrame::setIndicatorColour(QColor colour)
{
    ic = colour;
    update ();
}

void DisplayFrame::setBorderColour(QColor colour)
{
    bc = colour;
    update ();
}

void DisplayFrame::setIndicatorWidth(unsigned int width)
{
    iw = width;
    update ();
}

void DisplayFrame::setBorderWidth (unsigned int width)
{
    bw = width;
    update ();
}

void DisplayFrame::setFrame(ConstFramePtr frame)
{
    points = frame;
    update();
}

void DisplayFrame::paintEvent (QPaintEvent *)
{
    QPainter p(this);
    p.setBackground (Qt::black);
    p.setRenderHint(QPainter::HighQualityAntialiasing);


    const int h = height() - 2 * bw;
    const int w = width() - 2 * bw;
    // Border colour
    p.setBrush(QBrush(bc));
    p.drawRect(0,0,width(),height());
    // Indicator colour
    p.setBrush(QBrush(ic));
    p.drawRect(iw,iw,width()-2*iw,height()-2*iw);
    // Clear the display area
    p.setBrush(QBrush(Qt::black));
    p.drawRect(bw,bw,w,h);
    if (points && points->getPointCount()>0) {
        QPen pen;
        pen.setWidth(1);
        QColor col;
        Point po = points->getPoint(0);
        if (!po.blanked) {
            pen.setColor (QColor(po.r,po.g,po.b));
            p.setPen (pen);
            p.drawPoint(((1.0 + po.x)* w/2.0) + bw,((1.0-po.y)*h/2.0) + bw);
        }
        for (unsigned int i = 0; i < points->getPointCount()-1; i++) {
            Point tp = points->getPoint(i);
            Point np = points->getPoint(i+1);
            if (!np.blanked) {
                pen.setColor (QColor(tp.r,tp.g,tp.b));
                p.setPen (pen);
                p.drawLine (((1.0 + tp.x)* w/2.0) + bw,((1.0-tp.y)*h/2.0) + bw,
                            ((1.0 + np.x)*w/2.0) + bw,((1.0 - np.y)* h/2.0) + bw);
            }
        }
    }
}
