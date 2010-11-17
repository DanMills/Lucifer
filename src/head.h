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
		/// Returns a list of the selection modes this head supports
		//std::vector<std::string> enumerateSelectionModes();
		/// returns a list of the step modes this head supports
		//std::vector<std::string> enumerateStepModes();
		/// Set the selection mode
		//void setSelectionMode (const unsigned int mode);
		/// Set the step modes
		//void setStepMode (const unsigned int mode);
signals:
		//FrameSourcePtr requestNewSource(unsigned int index);
    /// Emitted when the frame source runs out of frames.
    void endOfSource();
    /// Emitted when the scan hardware buffer empties.
    void bufferEmpty();
    /// emitted whenever the displayed frame changes.
    /// Useful for output window updates
    void newFrame (FramePtr frame);
		/// Emitted whenever the currently projected source changes
		//void currentSelectionChanged (unsigned int oldsel, unsigned int newsel);
		/// Emitted when any selection changes
		//void selectionsChanged (unsigned int sel, bool active);
		//void selectionModeChanged (unsigned int mode);
		//void stepModeChanged (unsigned int mode);
public slots:
    /// Load a new framesource into the scan head
    bool loadFrameSource (FrameSourcePtr f, bool immediate = false);
		bool setDriver (std::string name);
		bool setPPS (unsigned int pps = 30000);
		//void select (unsigned int pos, bool active);
private:
		bool setDriver (DriverPtr d);
		int currentSelection;
    unsigned int targetPPS;
    DriverPtr driver;
    FrameSourcePtr fs;
    Playback pb;
    std::vector<PointF> pointBuf;
    size_t frame_index;
    Resample resampler;
    ColourTrimmer colourTrim[3];
		//unsigned int selection_mode;
		//unsigned int step_mode;
		//std::vector <unsigned int> selections;
		//QMutex selection_lock;

private slots:
    void dataRequested();
		void HWPpsChanged(unsigned int newPPS);
};
typedef boost::shared_ptr<LaserHead> LaserHeadPtr;
#endif


