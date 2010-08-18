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
#include <boost/numeric/ublas/matrix.hpp>
#include "point.h"

// For rendering to a qpainter
#include <QtGui>

class Frame
{
public:
    Frame();
    ~Frame();
		inline Point getPoint (size_t pos) const
		{
			assert (pos < points_.size());
			return points_[pos];
		};
    unsigned int getPointCount() const;
    void reserve (size_t points);
    void clear ();
    bool isEmpty() const;
    void addPoint (Point p);

		// Geometry operations
		void scale (const float x, const float y, const float z);
		void rotate (const float angle,float x,float y,float z);
		void translate (float x,float y,float z);

		// Rendering operations
		/// Renders a frame using a supplied QPainter.
		QPainter & render (QPainter& p,
											 const int start_x, const int start_y,
											 const int height, const int width) const;
		/// Renders a frame to a series of points possibly
		///doing point pulling and optimisation if appropriate.
		std::vector<Point> render (const Frame &f) const;
private:
    std::vector<Point> points_;
		mutable boost::numeric::ublas::matrix <float> geometry;
		/// Apply the geometry matrix to the frame
		void simplify ();
};

typedef boost::shared_ptr<Frame> FramePtr;
typedef boost::shared_ptr<const Frame> ConstFramePtr;

#endif
