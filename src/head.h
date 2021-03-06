/// Laser projector head
/* head.h is part of lucifer a laser show controller.

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


#ifndef HEAD_INC
#define HEAD_INC

#include <QtCore>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "driver.h"
#include "colour.h"
#include "framesource.h"
#include "resampler.h"
#include "colour.h"
#include "point.h"
#include "playbacklist.h"

// This needs to be forward declared to make LaserheadPtr available when engine.h
// includes this file
class LaserHead;
typedef boost::shared_ptr<LaserHead> LaserHeadPtr;
#include "engine.h"

/// \brief A tiny little shim to put each laser head into its own thread.
/// With essentially all modern CPUs being multicore this does much to
/// optimise performance.
/// HeadThread->start must be called to actually start any head running
class HeadThread : public QThread
{
    Q_OBJECT
public:
    HeadThread(Engine * e);
    virtual ~HeadThread();
    /// \pre this method MUST be called before attempting to use the head for anything.
    /// Further thre may be a delay afet calling run before the head actually becomes available.
    /// head becoming non NULL should be taken as indicating that this head is ready.
    void run();
    LaserHeadPtr head;
private:
    Engine * engine;
};


/// A Laser projection head. 
/// This object is responsible for actually projecting framesoure trees, 
/// requesting new ones from the engine as needed.
class LaserHead : public QObject
{
    Q_OBJECT
public:
    LaserHead(Engine * e);
    ~LaserHead();
    DriverPtr getDriver() const;
    /// Returns a list of the selection modes this head supports
    QStringList enumerateSelectionModes() const;
    /// returns a list of the step modes this head supports
    QStringList enumerateStepModes() const;
    bool isSelected (const int pos);
signals:
    /// Emitted when the frame source runs out of frames.
    void endOfSource();
    /// Emitted when the scan hardware buffer empties.
    void bufferEmpty();
    /// emitted whenever the displayed frame changes.
    /// Useful for output window updates
    void newFrame (FramePtr frame);
    /// Emitted whenever the currently projected source changes
    /// Emitted when any selection changes
    void selectionChanged (unsigned int sel, bool active);
    void selectionModeChanged (unsigned int mode);
    void stepModeChanged (unsigned int mode);
    /// Emitted whenever head activity changes (Blank or projecting).
    void headActive ();
    void headInactive ();
public slots:
    bool setDriver (std::string name);
    bool setPPS (unsigned int pps = 30000);
    void select (unsigned int pos, bool active);
    /// Set the selection mode
    void setSelectionMode (const PlaybackList::SelectionModes mode);
    /// Set the step modes
    void setStepMode (const PlaybackList::StepModes mode);
    /// Kill the output
    void kill();
    void restart();
private:
    Engine * engine;
    bool setDriver (DriverPtr d);
    int currentSelection;
    unsigned int targetPPS;
    DriverPtr driver;
    PlaybackPtr pb;
    std::vector<PointF> pointBuf;
    size_t frame_index;
    Resample resampler;
    ColourTrimmer colourTrim[3];
    PlaybackList sources;
    bool killed;

private slots:
    void dataRequested();
    void HWPpsChanged(unsigned int newPPS);
    void selectionChangedData (unsigned int sel, bool active);
    void dump();
    /// Load a new framesource into the scan head
    bool loadFrameSource (PlaybackPtr p, bool immediate = false);
    void manual();
    void beat();
};



#endif


