/*frame.h is part of lucifer a laser show controller.

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
/* $Id: frame.h 19 2010-07-03 16:53:57Z dmills $ */

#ifndef FRAME_INC
#define FRAME_INC

#include <vector>
#include <boost/shared_ptr.hpp>
#include "point.h"

class Frame
{
public:
    Frame();
    ~Frame();

	  Point getPoint (size_t pos) const;
    unsigned int getPointCount() const;
    void reserve (size_t points);
    void clear ();
    bool isEmpty() const;
    void addPoint (Point p);
private:
    std::vector<Point> points_;
};

typedef boost::shared_ptr<Frame> FramePtr;
typedef boost::shared_ptr<const Frame> ConstFramePtr;

#endif
