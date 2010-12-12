/*loadilda.h is part of lucifer a laser show controller.

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

#ifndef LOAD_ILDA_INC
#define LOAD_ILDA_INC

#include <vector>
#include <string>
#include <istream>
#include <QtCore>

#include "point.h"
#include "framesource.h"
#include "staticframe.h"

class Ildaloader
{
public:
    Ildaloader ();
    ~Ildaloader ();
    FrameSourcePtr load (const QString filename, unsigned int &error, bool pangolin_ct = false);

private:

    // For old format 2 and 3d point data
    void readHeader(QDataStream & stream, std::string& name,
                    std::string & company, unsigned short& pointcount);
    StaticFramePtr  readFrameSection(QDataStream & stream, bool is3DFrame);
    bool readTrueColorSection(QDataStream & stream);
    StaticFramePtr readFormat5(QDataStream & stream, bool is3DFrame);
    bool readColorTable(QDataStream & stream);

    std::vector <QColor> palette_;
    bool trueColour;
    std::vector <QColor> colour;
};
#endif
