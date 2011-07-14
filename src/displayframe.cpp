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

#include <QtGui>
#include <iostream>
#include "frame.h"
#include "displayframe.h"

int QFixedAspectWidget::heightForWidth(int w) const
{
    return w;
}

DisplayFrame::DisplayFrame(QWidget* parent): QFixedAspectWidget(parent)
{
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
		if (points){
			points->render (p,bw,bw,h,w);
		}
}
