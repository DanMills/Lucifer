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

#ifndef FRAME_INC
#define FRAME_INC

#include <vector>
#include <boost/shared_ptr.hpp>
#include "point.h"
#include <qmatrix4x4.h>

class QPainter;


/// \brief A set of Points and a geometry matrix comprising a single laser frame. 
/// The geometry member supports the usual 4*4 affine operations as well as more general 
/// matrix operators. 
class Frame
{
public:
    Frame();
    ~Frame();
    /// \brief Return a Point from the frame.
    /// @param[in] pos is the index of the point to return.
    /// @return A Point stucture.
    inline Point getPoint (size_t pos) const
    {
        assert (pos < points_.size());
        return points_[pos];
    };
    /// \brief Set a point in the frame.
    /// @param[in] pos the index of the point to set.
    /// @param[in] p is the point to store.
    /// @return the stored point.
    inline Point setPoint (size_t pos, Point p)
    {
      assert (pos < points_.size());
      points_[pos] = p;
      return p;
    }
    /// @return the number of points in this frame.
    unsigned int getPointCount() const;
    /// \brief Reseve space in the points storage structure.
    /// @param[in] points is the total number of points to reserve space for.
    void reserve (size_t points);
    /// \brief Clear out all the points.
    void clear ();
    /// \brief Check if the frame contains any points.
    /// @return true if there are no points stored, else false.
    bool isEmpty() const;
    /// \brief Add a point to the end of the points list.
    /// @param[in] p is the point to add.
    void addPoint (Point p);
    // Rendering operations
    /// \brief Renders a frame using a supplied QPainter.
    /// @param[in,out] p is the QPainter that the frame will be rendered onto.
    /// @param[in] start_x is the x coordinate of the lefthand edge relative to the qpainter.
    /// @param[in] start_y is the y coordinate of the bottom of the area being painted.
    /// @param[in] height is the height of the image being painted.
    /// @param[in] width is the width of the image being painted.
    /// @return the reference to p.
    QPainter & render (QPainter& p,
                       const int start_x, const int start_y,
                       const int height, const int width) const;
    /// \brief Renders a frame to a series of points possibly doing point pulling and optimisation if appropriate.
    /// @param[in] f is the frame to render to a point list.
    /// @return a vector of Point structures representing the output from the frame.
    std::vector<Point> render (const Frame &f) const;
    /// \brief The geometery matrix, this has methods for the usual affine operations.
    mutable QMatrix4x4 geometry;
private:
    std::vector<Point> points_;
    /// Apply the geometry matrix to the frame
    void applyGeometry ();
};

typedef boost::shared_ptr<Frame> FramePtr;
typedef boost::shared_ptr<const Frame> ConstFramePtr;

#endif
