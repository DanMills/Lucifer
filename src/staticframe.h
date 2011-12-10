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
#include "arcball.h"

///\brief A compact point representation for static frames

/// Points in general are floating point objects and somewhat large
/// For internal storage in staticframes. We use arrays of these because they are less then half
/// the size of the point structure (10 bytes packed, probably 12 in reality).
/// As most of the memory on a typical show goes on arrays of these, it is well
/// worth doing.

class ILDAPoint
{
public:
    ILDAPoint() : x_(0), y_(0), z_(0),blanked_(true),r_(0),g_(0),b_(0)
    {
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

/// \brief Static frame per playback data.
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
};

typedef boost::shared_ptr<StaticFramePlayback> StaticFramePlaybackPtr;

/// \brief A single static frame leaf node.

/// This holds an array of ILDAPoint 16 bit points and returns them as a FramePtr pointing to a standard floating point frame.
/// It is used to store single fixed frames either from ILDA files or imported from other image types.
class StaticFrame : public FrameSource_impl
{
public:
    StaticFrame ();
    ~StaticFrame();
    /// \brief Returns the next frame or NULL if playback exhausted.
    /// @param [in] pb is the StaticFramePlayback controlling this framesource.
    /// @return A pointer to a copy of the frame as a standard FramePtr shared pointer.
    FramePtr nextFrame(PlaybackImplPtr pb);
    /// \brief reserve a number of points in the frame storage.
    /// @param [in] points is the number of points to reserve.
    void reserve (size_t points);
    /// \brief Add an ILDAPoint to the point data.
    /// @param [in] p is the point to add to the end of the points data. 
    void add_data (const ILDAPoint &p);
    /// \brief return the number of frames this frame source will generate.
    /// @return the number of frames this source will return.
    size_t frames ();
    /// \brief returns the position within the frames that is current.
    /// \@return the number of frames into the total from frames() that has just been returned.
    /// @param [in] p is the playback for which the data is desired.
    size_t pos(PlaybackImplPtr p);
    /// \brief Resets the playback position and any other playback data.
    /// @param [out] p is the playback to reset.
    void reset(PlaybackImplPtr p);
    /// \brief Save the StaticFrame to xml.
    /// @param [out] w is the xml stream to write to.
    void save (QXmlStreamWriter* w);
    /// \brief load a StaticFrame from xml.
    /// @param [in] e is the xml stream to load from.
    void load (QXmlStreamReader *e);
    /// \brief A geometry matrix for affine transforms.
    /// This can be manipulated directly and will propagate up the tree until a 
    /// renderer eventually uses it to render the frame.
    QMatrix4x4 geometry;
    /// \brief The scale factor to apply to the matrix immediately before passing it up the tree.
    /// This is kept separate to simplify giving it its own control.
    float scale;
    /// Get a gui interface object for a StaticFrame.
    /// @param [in] parent is the parent QWidget in the usual QT way.
    /// @return A pointer to a FrameGui object.
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

/// \brief A GUI For a StaticFrame object. 
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
    void angleChangedData(QQuaternion q);
    void arcballDown();
    void arcballUp();
    void scaleChanged (int);
private:
    QLabel * pointsDisplay;
    QGridLayout * grid;
    QButtonGroup * group;
    QSpinBox * dewellEntry;
    QSpinBox * repeatEntry;
    QRadioButton * dewellSwitch;
    QRadioButton * repeatSwitch;
    StaticFrame * fp;
    ArcBall * arcball;
    QSlider * size;
};
#endif
