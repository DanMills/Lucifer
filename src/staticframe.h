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

// Points in general are floating point objects and somewhat large
// For internal storage we use arrays of these because they are less then half
// the size of the point structure (10 bytes packed, probably 12 in reality).
// As most of the memory on a typical show goes on arrays of these, it is well
// worth doing.

class ILDAPoint
{
public:
    ILDAPoint()
    {
        x_=y_=z_= 0;
        blanked_ = true;
        r_ = g_ = b_ = 0;
    }
    short x() const {
        return x_;
    }
    short y() const {
        return y_;
    }
    short z() const {
        return z_;
    }
    bool blanked() const {
        return blanked_;
    }
    unsigned char r() const {
        return r_;
    }
    unsigned char g() const {
        return g_;
    }
    unsigned char b() const {
        return b_;
    }

    Point point() const {
				Point p(QVector3D((float)x_,(float)y_,(float)z_)*=1.0f/32768.f,
                QColor(r_,g_,b_));
        p.blanked = blanked_;
        return p;
    }
    void setX(short v) {
        x_=v;
    }
    void setY(short v) {
        y_=v;
    }
    void setZ(short v) {
        z_=v;
    }
    void setBlanked(bool b) {
        blanked_ = b;
    }
    void setR (unsigned char v) {
        r_=v;
    }
    void setG (unsigned char v) {
        g_=v;
    }
    void setB (unsigned char v) {
        b_=v;
    }
private:
    short x_;
    short y_;
    short z_;
    bool blanked_;
    unsigned char r_;
    unsigned char g_;
    unsigned char b_;
};


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
    void add_data (const ILDAPoint &p);
    size_t frames ();

    size_t pos(PlaybackImplPtr p);
    void reset(PlaybackImplPtr p);

    void save (QXmlStreamWriter* w);
    void load (QXmlStreamReader *e);

    FrameGui * controls (QWidget *parent);
private:
    PlaybackImplPtr newPlayback();
    void copyDataTo (SourceImplPtr p) const;
    FramePtr frame() const;
    std::vector <ILDAPoint> data;
    unsigned int repeats;
    unsigned int  dewell;
    bool useDewell;
    friend class StaticFrameGui;
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
