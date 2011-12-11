/*point.h is part of lucifer a laser show controller.

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

#ifndef POINT_INC
#define POINT_INC

#include <QVector3D>
#include <QColor>
#include <ostream>


/// \brief A point in 3d space that may be blanked and has a colour.
/// Used as a control point for various line types.
class	Point : public QVector3D, public QColor
{
public:
    bool blanked;
    Point (QVector3D v, QColor c) : QVector3D(v), QColor(c), blanked (true)
    {}
    friend std::ostream&  operator << (std::ostream&  s, const Point & p);
};

typedef Point Vector;

#endif
