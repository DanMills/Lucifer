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

Frame::Frame()
{
};

Frame::~Frame()
{
    clear();
}

unsigned int Frame::getPointCount() const
{
    return points_.size();
}

Point Frame::getPoint (size_t pos) const
{
    assert (pos < points_.size());
    return points_[pos];
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
