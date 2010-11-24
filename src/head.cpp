/* head.cpp is part of lucifer a laser show controller.

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

#include "head.h"
#include "engine.h"

LaserHead::LaserHead(Engine* e)
{
		engine = e;
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
										assert (engine);
                    p = engine->getFrameSource(s);
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

