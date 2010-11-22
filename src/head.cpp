#include "head.h"

LaserHead::LaserHead()
{
    targetPPS = 30000;
    frame_index = 0;
    resampler.setInputPPS(targetPPS);
    resampler.setOutputPPS(30000);
    //selection_mode = 0;
    //step_mode = 0;
		//currentSelection = -1;
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
#if 0
std::vector< std::string> LaserHead::enumerateSelectionModes()
{
    std::vector<std::string> res;
    res.push_back(std::string("Single"));
    res.push_back(std::string("Multiple"));
    res.push_back(std::string("Shuffle"));
    return res;
}

std::vector< std::string > LaserHead::enumerateStepModes()
{
    std::vector<std::string> res;
		res.push_back(std::string("Loop"));
    res.push_back(std::string("Manual"));
    res.push_back(std::string("Once"));
    res.push_back(std::string("Beat"));
    return res;
}

int LaserHead::getNextSelection()
{
	selection_lock.lock();
	// deal with empty selection first
	if (selections.size() == 0){
		if (currentSelection != -1){
			emit currentSelectionChanged (currentSelection,-1);
			currentSelection = -1;
		}
		return -1;
	}
	int oldsel = currentSelection;
	switch (step_mode){
		case 0: //loop
			if (selections.size()==1){
					currentSelection = selections[0];
			} else {
				selections.push_back(selections[0]);
				selections.erase(selections.begin());
				currentSelection = selections[0];
			}




	}
}


void LaserHead::select(unsigned int pos, bool active)
{
    selection_lock.lock();
    if (active) {
        if (selection_mode == 0) {
            // Single selection mode
            while (selections.size()>1) {
                int s = selections[selections.size()-1];
                selections.erase(selections.back());
                emit selectionsChanged(s,false);
            }
            if (selections.size()>0) {
                if (selections[0] != pos) {
                    emit selectionsChanged(selections[0],false);
                }
                selections[0] = pos;
            } else {
                selections.push_back(pos);
            }
            emit selectionsChanged(selections[0],true);
        } else {
            // one of the multi select modes
            for (unsigned int i=0; i < selections.size(); i++) {
                if (selections[i] == pos) {
                    selection_lock.unlock();
                    return;
                }
            }
            selections.push_back(pos);
            emit selectionsChanged(pos,true);
        }
    } else {
        // deselecting
        for (unsigned int i=0; i < selections.size(); i++) {
            if (selections[i] == pos) {
                selections.erase(selections.begin()+i);
                emit selectionsChanged(i,false);
                break;
            }
        }
    }
    selection_lock.unlock();
}
void LaserHead::setSelectionMode(const unsigned int mode)
{
    if (selection_mode != mode) {
        selection_mode = mode;
        if (mode == 0) {
            // Single select mode
            if (selections.size() >0) {
                // Dump all the non current selections
                select (selections[0],true);
            }
        }
        emit selectionModeChanged (selection_mode);
    }
}

void LaserHead::setStepMode(const unsigned int mode)
{
    if (step_mode != mode) {
        mode = step_mode;
        emit stepModeChanged(mode);
    }
}

#endif
