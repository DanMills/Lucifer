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

/* $Id: frame.cpp 3 2010-05-19 20:42:53Z dmills $ */

#include <assert.h>
#include "frame.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <math.h>


Frame::Frame():
        geometry(4,4)
{
    geometry = boost::numeric::ublas::identity_matrix<float>(4);
};

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



void Frame::scale (const float x, const float y, const float z)
{
    boost::numeric::ublas::matrix <float> m(4,4);
    m = boost::numeric::ublas::identity_matrix<float>(4);
    m (0,0) = x;
    m(1,1) = y;
    m(2,2) = z;
    geometry = boost::numeric::ublas::prod (geometry,m);
}


// This code based on that found in the Mesa software 3d library
void Frame::rotate (const float angle,float x,float y,float z)
{
    float s;
    float c;
    bool optimized;

    s = sinf(angle * M_PI/180.0f);
    c = cosf(angle * M_PI/180.0f);
    boost::numeric::ublas::matrix <float> m(4,4);
    m = boost::numeric::ublas::identity_matrix<float>(4);
    optimized = false;

    if (x == 0.0F) {
        if (y == 0.0F) {
            if (z != 0.0F) {
                optimized = true;
                /* rotate only around z-axis */
                m(0,0) = c;
                m(1,1) = c;
                if (z < 0.0F) {
                    m(0,1) = s;
                    m(1,0) = -s;
                } else {
                    m(0,1) = -s;
                    m(1,0) = s;
                }
            }
        }
        else if (z == 0.0F) {
            optimized = true;
            /* rotate only around y-axis */
            m(0,0) = c;
            m(2,2) = c;
            if (y < 0.0F) {
                m(0,2) = -s;
                m(2,0) = s;
            } else {
                m(0,2) = s;
                m(2,0) = -s;
            }
        }
    }
    else if (y == 0.0F) {
        if (z == 0.0F) {
            optimized = true;
            /* rotate only around x-axis */
            m(1,1) = c;
            m(2,2) = c;
            if (x < 0.0F) {
                m(1,2) = s;
                m(2,1) = -s;
            } else {
                m(1,2) = -s;
                m(2,1) = s;
            }
        }
    }

    if (!optimized) {
        const float mag = sqrtf(x * x + y * y + z * z);

        if (mag <= 1.0e-4) {
            /* no rotation, leave mat as-is */
            return;
        }

        x /= mag;
        y /= mag;
        z /= mag;


        /*
        *     Arbitrary axis rotation matrix.
        *
        *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
        *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
        *  (which is about the X-axis), and the two composite transforms
        *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
        *  from the arbitrary axis to the X-axis then back.  They are
        *  all elementary rotations.
        *
        *  Rz' is a rotation about the Z-axis, to bring the axis vector
        *  into the x-z plane.  Then Ry' is applied, rotating about the
        *  Y-axis to bring the axis vector parallel with the X-axis.  The
        *  rotation about the X-axis is then performed.  Ry and Rz are
        *  simply the respective inverse transforms to bring the arbitrary
        *  axis back to it's original orientation.  The first transforms
        *  Rz' and Ry' are considered inverses, since the data from the
        *  arbitrary axis gives you info on how to get to it, not how
        *  to get away from it, and an inverse must be applied.
        *
        *  The basic calculation used is to recognize that the arbitrary
        *  axis vector (x, y, z), since it is of unit length, actually
        *  represents the sines and cosines of the angles to rotate the
        *  X-axis to the same orientation, with theta being the angle about
        *  Z and phi the angle about Y (in the order described above)
        *  as follows:
        *
        *  cos ( theta ) = x / sqrt ( 1 - z^2 )
        *  sin ( theta ) = y / sqrt ( 1 - z^2 )
        *
        *  cos ( phi ) = sqrt ( 1 - z^2 )
        *  sin ( phi ) = z
        *
        *  Note that cos ( phi ) can further be inserted to the above
        *  formulas:
        *
        *  cos ( theta ) = x / cos ( phi )
        *  sin ( theta ) = y / sin ( phi )
        *
        *  ...etc.  Because of those relations and the standard trigonometric
        *  relations, it is pssible to reduce the transforms down to what
        *  is used below.  It may be that any primary axis chosen will give the
        *  same results (modulo a sign convention) using thie method.
        *
        *  Particularly nice is to notice that all divisions that might
        *  have caused trouble when parallel to certain planes or
        *  axis go away with care paid to reducing the expressions.
        *  After checking, it does perform correctly under all cases, since
        *  in all the cases of division where the denominator would have
        *  been zero, the numerator would have been zero as well, giving
        *  the expected result.
        */
        float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * s;
        ys = y * s;
        zs = z * s;
        one_c = 1.0F - c;

        /* We already hold the identity-matrix so we can skip some statements */
        m(0,0) = (one_c * xx) + c;
        m(0,1) = (one_c * xy) - zs;
        m(0,2) = (one_c * zx) + ys;
        /*    M(0,3) = 0.0F; */

        m(1,0) = (one_c * xy) + zs;
        m(1,1) = (one_c * yy) + c;
        m(1,2) = (one_c * yz) - xs;
        /*    M(1,3) = 0.0F; */

        m(2,0) = (one_c * zx) - ys;
        m(2,1) = (one_c * yz) + xs;
        m(2,2) = (one_c * zz) + c;
        /*    M(2,3) = 0.0F; */

        /*
        M(3,0) = 0.0F;
        M(3,1) = 0.0F;
        M(3,2) = 0.0F;
        M(3,3) = 1.0F;
        */
    }
    geometry = boost::numeric::ublas::prod (geometry,m);
}

void Frame::translate (float x,float y,float z)
{
    boost::numeric::ublas::matrix <float> m(4,4);
    m = boost::numeric::ublas::identity_matrix<float>(4);
    m(3,0) = x;
    m(3,1) = y;
    m(3,2) = z;
    geometry = boost::numeric::ublas::prod (geometry,m);
}

void Frame::projection (float x, float y, float z)
{
    assert (z != 0.0);
    boost::numeric::ublas::matrix <float> m(4,4);
		m = boost::numeric::ublas::identity_matrix<float>(4);
    m(3,3) = 0;
    m(3,2) = 1.0/z;
    m(0,3) = x;
    m(1,3) = y;
    geometry = boost::numeric::ublas::prod (geometry,m);
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
        f.rotate (theta +=5,1,1,1);
				//f.scale (0.5,0.5,0.5);
        //f.projection (0,0,2);
        f.simplify ();
        if (f.getPointCount() == 0) {
            // simplify might have rejected all the points as being outside the frustum
            return p;
        }
        QPen pen;
        pen.setWidth(1);
        QColor col;
        Point po = f.getPoint(0);
        if (!po.blanked) {
            pen.setColor (QColor(po.r,po.g,po.b));
            p.setPen (pen);
            p.drawPoint(((1.0 + po.v[Point::X])* width/2.0) + start_x,((1.0-po.v[Point::Y])*height/2.0) + start_y);
        }
        for (unsigned int i = 0; i < f.getPointCount()-1; i++) {
            Point tp = f.getPoint(i);
            Point np = f.getPoint(i+1);
            if (!np.blanked) {
                pen.setColor (QColor(tp.r,tp.g,tp.b));
                p.setPen (pen);
                p.drawLine (((1.0 + tp.v[Point::X])*width/2.0) + start_x,((1.0-tp.v[Point::Y])*height/2.0) + start_y,
                            ((1.0 + np.v[Point::X])*width/2.0) + start_x,((1.0-np.v[Point::Y])*height/2.0) + start_y);
            }
        }
    }
    return p;
}

void Frame::simplify ()
{
    assert (this);
    boost::numeric::ublas::matrix <float> mat (4,4);
    mat = geometry;
    ///TODO - Bypass this if the matrix is allready an identity
    for (unsigned int i=0; i < getPointCount(); ++i) {
        /// Each 3d point gets multiplied by the geometry matrix to give the output point.
        points_[i].v = boost::numeric::ublas::prod (points_[i].v,mat);
				float s = 1.0f/points_[i].v[3];
				points_[i].v[Point::X] *= 2.0/(2.0 - points_[i].v[Point::Z]);
				points_[i].v[Point::Y] *= 2.0/(2.0 - points_[i].v[Point::Z]);
    }
}
