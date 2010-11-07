/// Laser projector head
#ifndef HEAD_INCL
#define HEAD_INCL

#include <QtCore>
#include <vector>
#include "driver.h"
#include "colour.h"
#include "framesource.h"
#include "resampler.h"
#include "colour.h"
#include "point.h"

class LaserHead : public QObject
{
    Q_OBJECT
public:
    LaserHead();
    ~LaserHead();
		DriverPtr getDriver() const;
signals:
    /// Emitted when the frame source runs out of frames.
    void endOfSource();
    /// Emitted when the scan hardware buffer empties.
    void bufferEmpty();
    /// emitted whenever the displayed frame changes.
    /// Useful for output window updates
    void newFrame (FramePtr frame);
public slots:
    /// Load a new framesource into the scan head
    bool loadFrameSource (FrameSourcePtr f, bool immediate = false);
		bool setDriver (std::string name);
		bool setPPS (unsigned int pps = 30000);
private:
		bool setDriver (DriverPtr d);
    unsigned int targetPPS;
    DriverPtr driver;
    FrameSourcePtr fs;
    Playback pb;
    std::vector<PointF> pointBuf;
    size_t frame_index;
    Resample resampler;
    ColourTrimmer colourTrim[3];
private slots:
    void dataRequested();
		void HWPpsChanged(unsigned int newPPS);
};
typedef boost::shared_ptr<LaserHead> LaserHeadPtr;
#endif


