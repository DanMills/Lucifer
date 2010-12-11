
#include <algorithm>
#include <functional>
#include "log.h"
#include "playbacklist.h"


PlaybackList::PlaybackList(QObject *parent) : QObject(parent)
{
    selMode = SINGLE;
    stepMode = LOOP;
    currentIndex = -1;
    lock = new QMutex (QMutex::Recursive);
}
PlaybackList::~PlaybackList()
{
    delete lock;
}

void PlaybackList::select(unsigned int pos, bool active)
{
    slog()->infoStream() << "Playback selected "<< pos << " : " << (active ? "on" : "off");
    slog()->debugStream() << "Selection mode : " << selectionModeList()[selMode].toStdString();
    slog()->debugStream() << "Step mode : " << stepModeList()[stepMode].toStdString();
    QMutexLocker l (lock);
    if (!active) {
        for (unsigned int i=0; i < selections.size(); i++) {
            if (selections[i] == pos) {
                selections.erase(selections.begin()+i);
                emit selectionChanged (pos,false);
                if (i==0) {
                    slog ()->debugStream() << "Dumping current playback";
                    currentIndex= -1;
                    emit dumpCurrentSelection();
                    next (false);
                }
                break;
            }
        }
    } else {
        // adding a selection
        for (unsigned int i=0; i < selections.size(); i++) {
            if (selections[i] == pos) {
                slog()->debugStream() << "Adding playback allready in list : deselecting";
                // Allready selected
                // so deselect it
                select (pos, false);
                break;
            }
        }
        if (selMode==SINGLE) {
            slog()->debugStream() << "Adding single selection : " << pos;
            for (unsigned int i=0; i < selections.size();i++) {
                emit selectionChanged(selections[i],false);
            }
            selections.clear();
            currentIndex= -1;
            emit dumpCurrentSelection();
            selections.push_back(pos);
            next(false);
            emit selectionChanged(pos,true);
        } else {
            slog()->debugStream() << "Adding in a multi select mode : " << pos;
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
    slog()->infoStream() << "Setting selection mode : " << selectionModeList()[mode].toStdString() << " : " << (int) mode;
    if (mode == SINGLE) {
        QMutexLocker l (lock);
        while (selections.size() > 1) {
            emit selectionChanged(selections[selections.size()-1],false);
            selections.erase(selections.end()-1);
        }
    }
    selMode = mode;
}

void PlaybackList::setStepMode(PlaybackList::StepModes mode)
{
    slog()->infoStream() << "Setting step mode : " << stepModeList()[mode].toStdString();
    stepMode = mode;
}

void PlaybackList::beatDetected()
{
    if (stepMode == BEAT) {
        currentIndex = next(true);
        emit dumpCurrentSelection();
    }
}

void PlaybackList::manualStepClicked()
{
    if (stepMode == MANUAL) {
        currentIndex = next(true);
        emit dumpCurrentSelection();
    }
}

void PlaybackList::clear()
{
    slog ()->debugStream() << "Clearing playback list";
    QMutexLocker l(lock);
    while (selections.size() > 1) {
        emit selectionChanged(selections[selections.size()-1],false);
        selections.erase(selections.end()-1);
    }
    currentIndex= -1;
    emit dumpCurrentSelection();
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
{		//selections.push_back(rand()%64);
    QMutexLocker l (lock);
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

bool PlaybackList::isSelected(const int sel)
{
    QMutexLocker l (lock);
    for (unsigned int i=0; i < selections.size(); i++) {
        if (selections[i]==(unsigned) sel) {
            return true;
        }
    }
    return false;
}


