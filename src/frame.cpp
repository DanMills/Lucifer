/*frame.cpp is part of lucifer a laser show controller.

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
#include "frame.h"
#include <math.h>
// For rendering to a qpainter
#include <QtGui>

Frame::Frame(): geometry()
{
}

Frame::~Frame()
{
    clear();
}

unsigned int Frame::getPointCount() const
{
    return points_.size();
}

void Frame::clear()
{
    points_.clear();
}

bool Frame::isEmpty() const
{
    return (points_.size() == 0);
}

void Frame::addPoint(Point p)
{
    points_.push_back(p);
}

void Frame::reserve(size_t points)
{
    points_.reserve (points);
}

QPainter & Frame::render (QPainter &p,
                          const int start_x, const int start_y,
                          const int height, const int width) const
{
    assert (this);
    if (getPointCount()>0) {
        // F gets modified by having the geometry matrix applied
        Frame f = *this;
        static int theta = 0;
//	f.geometry.translate(0,0,30);
	//f.geometry.rotate (theta +=5,1,1,1);
	//f.geometry.lookAt(QVector3D(30.0,0,0),QVector3D(0,0,0),QVector3D(0,1,0));
	//std::out<< f.geometry;
	//f.scale (0.2,0.2,0.2);
//	f.geometry.translate(0,0,30);
//	f.geometry.perspective(60.0,1.8,-10,10);
        //f.geometry.ortho(-1,1,-1,1,2,-2);
        f.applyGeometry ();
        if (f.getPointCount() == 0) {
            // simplify might have rejected all the points as being outside the frustum
            return p;
        }
        QPen pen;
        pen.setWidth(1);
        QColor col;
        Point po = f.getPoint(0);
        if (!po.blanked) {
            pen.setColor (QColor(po));
            p.setPen (pen);
            p.drawPoint(((1.0 + po.x())* width/2.0) + start_x,((1.0-po.y())*height/2.0) + start_y);
        }
        for (unsigned int i = 0; i < f.getPointCount()-1; i++) {
            Point tp = f.getPoint(i);
            Point np = f.getPoint(i+1);
            if (!np.blanked) {
                pen.setColor (QColor(tp));
                p.setPen (pen);
                p.drawLine (((1.0 + tp.x())*width/2.0) + start_x,((1.0-tp.y())*height/2.0) + start_y,
                            ((1.0 + np.x())*width/2.0) + start_x,((1.0-np.y())*height/2.0) + start_y);
            }
        }
    }
    return p;
}

void Frame::applyGeometry()
{
    geometry.optimize();
    for (unsigned int i=0; i < getPointCount(); i++) {
        Point p = points_[i];
        QColor c = static_cast<QColor> (p);
        Point o (geometry.map(static_cast<QVector3D>(p)),c);
        o.blanked = p.blanked;
        points_[i] = o;
    }
    // set the geometry to the identity matrix
    geometry = QMatrix4x4();
}
