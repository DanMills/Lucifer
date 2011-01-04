/*staticframe.h is part of lucifer a laser show controller.

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

#ifndef STATICFRAME_INC
#define STATICFRAME_INC


#include <vector>
#include <map>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <QtGui>

#include "point.h"
#include "framesource.h"
#include "displayframe.h"

class StaticFramePlayback : public Playback_impl
{
public:
    StaticFramePlayback () {
        active = true;
        repeatsDone = 0;
    };
    bool active;
    QTime dewellStart;
    unsigned int repeatsDone;
		float angleX;
		float angleY;
		float angleZ;
};

typedef boost::shared_ptr<StaticFramePlayback> StaticFramePlaybackPtr;

/// A single static frame leaf node.
class StaticFrame : public FrameSource_impl
{
public:
    StaticFrame ();
    ~StaticFrame();
    FramePtr nextFrame(PlaybackImplPtr pb);
    void reserve (size_t points);
    void add_data (const Point &p);
    size_t frames ();

    size_t pos(PlaybackImplPtr p);
    void reset(PlaybackImplPtr p);

		void save (QXmlStreamWriter* w);
		void load (QXmlStreamReader *e);

    FrameGui * controls (QWidget *parent);
private:
		PlaybackImplPtr newPlayback();
		void copyDataTo (SourceImplPtr p) const;

    FramePtr data_;
    unsigned int repeats;
    unsigned int  dewell;
    bool useDewell;
    friend class StaticFrameGui;
		// used for load and save of pointsets
		unsigned int ftoi (float f);
		float itof(unsigned int i);

};


typedef boost::shared_ptr<StaticFrame> StaticFramePtr;
typedef boost::shared_ptr<const StaticFrame> ConstStaticFramePtr;

class StaticFrameGui : public FrameGui
{
    Q_OBJECT
public:
    StaticFrameGui (QWidget *parent);
    ~StaticFrameGui ();
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

    StaticFrame * fp;
};
#endif
