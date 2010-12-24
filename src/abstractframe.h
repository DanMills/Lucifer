/*abstractframe.h is part of lucifer a laser show controller.

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

#ifndef ABSTRACTFRAME_INC
#define ABSTRACTFRAME_INC

#include <vector>
#include <map>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <QtGui>

#include "point.h"
#include "framesource.h"
#include "displayframe.h"

class AbstractFramePlayback : public PlaybackData
{
public:
    AbstractFramePlayback () {
        Z = Zz = Zzz = 0.0f;
        G = 0.0f;
        Blankerizer = 0.0f;
        Colouriser = 0.0f;
        last = Point();
        next = Point();
        saved = Point();
    };
    float Z;
    float Zz;
    float Zzz;
    float G;
    float Blankerizer;
    float Colouriser;
    Point last;
    Point next;
    Point saved;
};

/// Abstract frame source
class AbstractFrame : public FrameSource
{
public:
    AbstractFrame ();
    ~AbstractFrame();
    PlaybackDataPtr createPlaybackData ();
    FramePtr data(Playback p);
    size_t frames ();
    void copyDataTo (FrameSourcePtr p) const;

    size_t pos(Playback p);
    void reset(Playback p);

    void save (QXmlStreamWriter* w);
    void load (QXmlStreamReader *e);

//    AbstractFrameGui * controls (QWidget *parent);
private:
//    friend class AbstractFrameGui;
    /// These are constant across all playbacks
    enum ENVELOPE {SINE = 0,SQUARE};
    enum FUNCTION {ORDER1 = 1,ORDER2,ORDER3,SUMO1O2,DISCONTO1O2,DISCONTO1O3,SCAN1,SCAN2};

    enum FUNCTION formula;
    enum FUNCTION zformula;
    enum FUNCTION colourPtn;
    enum ENVELOPE envelope;

    float gainZ;
    float gainZz;
    float gainZzz;

    float offsetZ;
    float offsetZz;
    float offsetZzz;

    float offsetR;
    float offsetG;
    float offsetB;

    float zAxisGain;

    float envelopeDepth;

    unsigned short colourSpd;
    unsigned short blankSpd;
    unsigned short blankSpot;

    float ZChngNum;
    float ZzChngNum;
    float ZzzChngNum;
    float GChngNum;

		float initialZr;
		float initialZg;
		float initialZb;
    float initialBlankerizer;
    float initialColoriser;
    float initialZ;
    float initialZz;
    float initialZzz;
    float initialG;
};


typedef boost::shared_ptr<AbstractFrame> AbstractFramePtr;
typedef boost::shared_ptr<const AbstractFrame> ConstAbstractFramePtr;

#if 0
class AbstractFrameGui : public FrameGui
{
    Q_OBJECT
public:
    AbstractFrameGui (QWidget *parent);
    ~AbstractFrameGui ();
    void set (StaticFrame * p);
    const QIcon * icon();
private slots:
    void buttonChangedData(int id);
    void dewellChangedData (int value);
    void repeatChangedData(int value);
private:
    QLabel * pointsDisplay;
    DisplayFrame * display;
    QGridLayout * grid;
    QButtonGroup * group;
    QSpinBox * dewellEntry;
    QSpinBox * repeatEntry;
    QRadioButton * dewellSwitch;
    QRadioButton * repeatSwitch;
};
#endif
#endif
