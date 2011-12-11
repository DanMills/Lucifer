/*loadilda.cpp is part of lucifer a laser show controller.

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

// Loads an ILDA file as a show fragment.

//Returns a pointer to a framesource or NULL if invalid.
//The framesource will be either a framesequencer if a multi frame file
// or a static frame if only a single frame.

// Defaults to ILDA palette unless forced to use the pangolin one
// Obviously palletes override this logic if supplied.

#include <iostream>
#include <assert.h>
#include <vector>
#include <string.h>
#include <QtCore>
#include <boost/make_shared.hpp>

#include "point.h"
#include "framesource.h"
#include "framesequencer.h"
#include "staticframe.h"
#include "ildapallette.h"
#include "log.h"
#include "loadilda.h"

#include "colourrotator.h"


Ildaloader::Ildaloader ()
{
    trueColour = false;
}
Ildaloader::~Ildaloader ()
{
}

SourceImplPtr Ildaloader::load (QString filename, unsigned int &error, bool pangolin)
{
    slog()->infoStream() << "Loading ILDA file : " << filename.toStdString();
    slog()->infoStream() << "Using pangolin palette : " << ((pangolin) ? "True" : "False");
    error = 0;
    QFile infile (filename);
    if (!infile.open(QIODevice::ReadOnly)) {
        error = 1;
        slog()->infoStream() <<"Unable to open '" << filename.toStdString() <<"' for reading";
        return SourceImplPtr();
    }
    palette_.clear ();
    palette_.reserve (256);
    colour.clear();
    trueColour = false;
    // Load default colour palette
    unsigned int i = 0;
    const short * pal = (pangolin) ? pangolin_palette : ilda_palette;
    while (pal[3*i] != -1) {
        palette_.push_back (QColor (pal[3*i],pal[3*i+1],pal[3*i+2]));
        i++;
    }
    while (i < 256) {
        palette_.push_back (QColor(Qt::black));
        i++;
    }
    // Parse the file building either a single staticframe or a framesequencer full of static frames as we go
    StaticFramePtr frame;
    FrameSequencerPtr sequence;

    QDataStream stream (&infile);
    stream.setByteOrder (QDataStream::BigEndian);

    bool eof = false;
    while ((!stream.atEnd()) && (!eof)) {
        char buf[4];
        stream.readRawData (buf,4);
        if (memcmp ("ILDA",buf,4)) {
            // Not an ILDA section?
            error = 2;
            slog()->errorStream() << "Section name not ILDA";
            break;
        }
        quint32 format;
        stream >> format;
        StaticFramePtr fr;
        switch (format) {
        case 0:
            // 3D Frame
            slog()->infoStream() << "Type 0 (3D points) section";
            fr = readFrameSection(stream, true);
            eof = (fr == NULL);
            break;

        case 1:
            // 2D Frame
            slog()->infoStream() << "Type 1 (2D points) section";
            fr = readFrameSection(stream, false);
            eof = (fr == NULL);
            break;

        case 2:
            // Color Table
            slog()->infoStream() << "Type 2 (Indexed colour table) section";
            eof = readColorTable(stream);
            break;

        case 3:
            // True Color
            slog()->infoStream() << "Type 3 (True colour table) section";
            slog()->errorStream() <<"Type 3 is not a ratified ILDA standard and worse Laserboy uses something called type 3 that DOES NOT match the draft ILDA standard that briefly existed";
            eof = readTrueColorSection(stream);
            break;

        case 4:
            slog()->infoStream() << "Type 4 (True colour 3D) section";
            fr = readFormat5(stream, true);
            eof = (fr == NULL);
            break;

        case 5:
            slog()->infoStream() << "Type 5 (True colour 2D) section";
            fr = readFormat5(stream, false);
            eof = (fr == NULL);
            break;

        default:
            slog()->errorStream() << "Type " << format << " Unknown section skipping";
            qint32 dataLength;
            qint32 numberOfPoints;

            stream >> dataLength >> numberOfPoints;
            stream.skipRawData(dataLength);
            break;
        }
        if (fr) {
            // New frame
            if (sequence) {
                // Already got a frame sequencer
                sequence->addChild(fr);
            } else {
                if (!frame) {
                    // This is the first or only frame
                    frame = fr;
                } else {
                    // Second frame in file, we need a sequencer
                    sequence = boost::make_shared<FrameSequencer>();
                    sequence->setDescription(std::string("ILDA File import from : ") +
											filename.toStdString());
                    sequence->addChild (frame);
                    sequence->addChild (fr);
                }
            }
        }
    }
    infile.close ();
    palette_.clear();
    colour.clear();

    if (sequence){
      ColourRotatorPtr r = boost::make_shared<ColourRotator>();
      r->addChild(sequence);
      return r;
    }
    
    if (sequence) {
        return sequence;
    } else {
        return frame;
    }
}

// For old format 2 and 3d point data
void Ildaloader::readHeader(QDataStream & stream, std::string & name,
                            std::string & company, quint16 & pointcount)
{
    char name_[9], companyName_[9];

    name_[8] = '\0';
    companyName_[8] = '\0';

    stream.readRawData(name_, 8);
    stream.readRawData(companyName_, 8);
    stream >> pointcount;
    stream.skipRawData(6);
    name = name_;
    company = companyName_;
    slog()->infoStream() << "Name : " << name << " by : " << company;
    slog()->debugStream() << "Count : " << pointcount;
}

StaticFramePtr Ildaloader::readFrameSection(QDataStream & stream, bool is3DFrame)
{
    std::string name, companyName;
    quint16 points;
    StaticFramePtr frame;

    readHeader(stream, name, companyName, points);
    if (points == 0) {
        //End of data marker
        return frame;
    }
    frame = boost::make_shared<StaticFrame>();
    frame->setDescription(std::string("ILDA: ") + name + std::string(" By ") + companyName );
    frame->reserve(points);
    for (unsigned int i=0; i<points; ++i) {
        ILDAPoint p;
        qint16 x;
        qint16 y;
        stream >> x >> y;
        p.setX(x);
        p.setY(y);
        if (is3DFrame) {
            qint16 z;
            stream >> z;
            p.setZ(z);
        } else {
            p.setZ(0);
        }
        quint8 state = 0;
        quint8 col;
        stream >> state >> col;
        p.setBlanked((state & 64) == 64);
        if (!trueColour) {
            p.setR(palette_[col].red());
            p.setG(palette_[col].green());
            p.setB(palette_[col].blue());
        } else {
            p.setR(colour[i].red());
            p.setG(colour[i].green());
            p.setB(colour[i].blue());
        }
        frame->add_data (p);
    }
    colour.clear();
    trueColour = false;
    return frame;
}

bool Ildaloader::readTrueColorSection(QDataStream& stream)
{
    quint32 points;
    quint32 datalen;
    stream >> datalen;
    stream >> points;
    if (points >0xFFFF) {
        // Something odd, an ILDA frame can have no more then 65535 points!
        // Fall back on indexed colour mode
        stream.skipRawData(datalen-4);
        slog()->errorStream() << "ILDA Load - found truecolour section with invalid length!";
        return false;
    }
    if (points ==0) {
        return true;
    }
    colour.clear();
    colour.reserve(points);
    for (unsigned int  i=0; i<points; ++i) {
        quint8 r, g, b;
        stream >> r >> g >> b;
        colour.push_back(QColor((unsigned char)r,
                                (unsigned char)g,
                                (unsigned char)b));
    }
    trueColour = true;
    return false;
}

StaticFramePtr Ildaloader::readFormat5(QDataStream & stream, bool is3DFrame)
{
    std::string name, companyName;
    quint16 points;

    readHeader(stream, name, companyName, points);
    if (points == 0) {
        return StaticFramePtr();
    }
    StaticFramePtr frame = boost::make_shared<StaticFrame>();
    frame->setDescription(std::string("ILDA: ") + name + std::string(" By ") + companyName );
    frame->reserve(points);
    for (unsigned int i = 0; i < points; ++i) {
        ILDAPoint p;
        quint8 r, g, b, state;
        qint16 x,y;
        stream >> x >> y;
        p.setX(x);
        p.setY(y);
        if (is3DFrame) {
            qint16 z;
            stream >> z;
            p.setZ(z);
        } else {
            p.setZ(0);
        }
        stream >> state >> b >> g >> r;
        p.setBlanked((state & 64) == 64);
        p.setR(r);
        p.setG(g);
        p.setB(b);
        frame->add_data (p);
    }
    return frame;
}

bool Ildaloader::readColorTable(QDataStream & stream)
{
    char name_[9], companyName_[9];
    quint16 colours;

    name_[8] = '\0';
    companyName_[8] = '\0';

    stream.readRawData(name_, 8);
    stream.readRawData(companyName_, 8);
    slog()->infoStream() << "Palette name : " << std::string (name_);
    stream >> colours;
    stream.skipRawData(2);
    slog()->infoStream() << "Colours : " << colours;
    // skip 4 bytes
    stream.skipRawData(4);
    palette_.clear ();
    palette_.reserve(colours);
    for (unsigned int i = 0; i < colours; ++i) {
        quint8  r, g, b;
        stream >> r >> g >> b;
        palette_.push_back(QColor (r,g,b));
    }
    return false;
}

