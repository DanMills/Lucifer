
#include <algorithm>
#include <functional>
#include "playbacklist.h"


PlaybackList::PlaybackList(QObject *parent) : QObject(parent)
{
    selMode = SINGLE;
    stepMode = LOOP;
    currentIndex = -1;
}
PlaybackList::~PlaybackList()
{
}
void PlaybackList::select(unsigned int pos, bool active)
{
    if (!active) {
        for (unsigned int i=0; i < selections.size(); i++) {
            if (selections[i] == pos) {
                selections.erase(selections.begin()+i);
                emit selectionChanged (pos,false);
                if (i==0) {
                    // Current selection changed
                    next (false);
                }
                break;
            }
        }
    } else {
        // adding a selection
        for (unsigned int i=0; i < selections.size(); i++) {
            if (selections[i] == pos) {
                // Allready selected
                // so deselect it
                select (pos, false);
                break;
            }
        }
        if (selMode==SINGLE) {
            for (unsigned int i=0; i < selections.size();i++) {
                emit selectionChanged(selections[i],false);
            }
            selections.clear();
            selections.push_back(pos);
            emit selectionChanged(pos,true);
            next(false);
        } else {
            selections.push_back(pos);
            emit selectionChanged(pos,true);
        }
    }
}

PlaybackList::SelectionModes PlaybackList::getSelectionMode() const
{
    return selMode;
}
PlaybackList::StepModes PlaybackList::getStepMode() const
{
    return stepMode;
}

void PlaybackList::setSelectionMode(PlaybackList::SelectionModes mode)
{
    if (mode == SINGLE) {
        while (selections.size() > 1) {
            emit selectionChanged(selections[selections.size()-1],false);
            selections.erase(selections.end()-1);
        }
        selMode = mode;
    }
}

void PlaybackList::setStepMode(PlaybackList::StepModes mode)
{
    stepMode = mode;
}

void PlaybackList::beatDetected()
{
    if (stepMode == BEAT) {
        currentIndex = next();
    }
}

void PlaybackList::manualStepClicked()
{
    if (stepMode == MANUAL) {
        currentIndex = next();
    }
}

int PlaybackList::getNextFramesource ()
{
    if ((stepMode == ONCE) || (stepMode == LOOP) || (currentIndex == -1)) {
        currentIndex = next(currentIndex != -1);
    }
    return currentIndex;
}

int PlaybackList::getFrameSource()
{
    return currentIndex;
}


int PlaybackList::next(bool running)
{
    if (selections.size()) {
        int oldsel = selections[0];
        switch (stepMode) {
        case ONCE:
            if (running) {
                select (oldsel,false);
            }
            if (selections.size()) {
                emit load (selections[0]);
            }
            emit selectionChanged (oldsel,false);
            break;
        case LOOP:
        case MANUAL:
        case BEAT:
            selections.push_back(oldsel);
            selections.erase(selections.begin());
            if (selMode == SHUFFLE) {
                std::random_shuffle	(selections.begin(),selections.end());
            }
            emit load (selections[0]);
            break;
        }
    }
    if (selections.size()) {
        return selections[0];
    }
    return -1;
}

QStringList PlaybackList::selectionModeList() const
{
    QStringList l;
    l.append(QString("Single"));
    l.append(QString("Multiple"));
    l.append(QString("Shuffle"));
    return l;
}

QStringList PlaybackList::stepModeList() const
{
    QStringList l;
    l.append(QString("Once"));
    l.append(QString("Loop"));
    l.append(QString("Manual"));
    l.append(QString("Beat"));
    return l;
}



