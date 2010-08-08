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

/* $Id: point.h 22 2010-07-24 15:01:51Z dmills $ */

#ifndef POINT_INC
#define POINT_INC

#include <iostream>
#include <QtGui>

class Point
{
public:
    float x;
    float y;
    float z;
    //QColor colour;
		unsigned char r;
		unsigned char g;
		unsigned char b;
    bool blanked;

    //friend std::ostream& operator<< (std::ostream &out, Point &p);
    //friend std::istream& operator>> (std::istream &in, Point &p);
};
#endif
