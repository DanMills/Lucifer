#include "head.h"

LaserHead::LaserHead()
{
    //engine = e;
    targetPPS = 30000;
    frame_index = 0;
    resampler.setInputPPS(targetPPS);
    resampler.setOutputPPS(30000);

}

LaserHead::~LaserHead()
{
    if (fs) {
        fs->deletePlayback(pb);
    }
}

void LaserHead::HWPpsChanged(unsigned int newPPS)
{
    resampler.setOutputPPS(newPPS);
}


bool LaserHead::setPPS(unsigned int pps)
{
    targetPPS = pps;
    resampler.setInputPPS(pps);
    return true;
}

bool LaserHead::setDriver(DriverPtr d)
{
    if (d && (d->flags() & Driver::OUTPUTS_ILDA)) {
        driver = d;
        resampler.setOutputPPS(driver->ILDAHwPointsPerSecond());
        connect (&(*driver),SIGNAL(ILDARequestMoreData()),this,SLOT(dataRequested()));
        connect (&(*driver),SIGNAL(ILDAHwPPSChanged(uint)),this, SLOT(HWPpsChanged(uint)));
        return true;
    } else {
        return false;
    }
}

bool LaserHead::setDriver(std::string name)
{
    DriverPtr d = Driver::newDriver(name);
    return setDriver(d);
}


void LaserHead::dataRequested()
{
    if (driver) {
        size_t t = driver->ILDANewPoints(pointBuf, frame_index);
        frame_index +=t;
        if (frame_index >= pointBuf.size()) {
            pointBuf.clear();
            frame_index = 0;
            FramePtr fp;
            if (fs) {
                fp = fs->data(pb);
            }
            emit newFrame(fp);
            if (!fp) {
                FrameSourcePtr p;
                int s;
                s = sources.getNextFramesource();
                if (s > -1) {
                    //p = engine->getFrameSource(s);
                }
                loadFrameSource(p,false);
                emit endOfSource();
            } else {
                std::vector<Point> p;
                p.reserve(fp->getPointCount());
                for (unsigned int i=0; i < fp->getPointCount(); i++) {
                    p.push_back(fp->getPoint(i));
                }
                pointBuf = resampler.run(p);
            }
        }
    }
}

bool LaserHead::loadFrameSource(FrameSourcePtr f, bool immediate)
{
    if (fs) {
        fs->deletePlayback(pb);
    }
    fs = f;
    if (fs) {
        pb = fs->createPlayback();
    }
    if (immediate) {
        frame_index = 0;
        pointBuf.clear();
    }
    return true;
}

DriverPtr LaserHead::getDriver() const
{
    return driver;
}

QStringList LaserHead::enumerateSelectionModes()const
{
	return sources.selectionModeList();
}

QStringList LaserHead::enumerateStepModes() const
{
	return sources.stepModeList();
}

void LaserHead::select(unsigned int pos, bool active)
{
	sources.select(pos,active);
}

void LaserHead::setSelectionMode(const PlaybackList::SelectionModes mode)
{
	sources.setSelectionMode(mode);
}

void LaserHead::setStepMode(const PlaybackList::StepModes mode)
{
	sources.setStepMode(mode);
}

