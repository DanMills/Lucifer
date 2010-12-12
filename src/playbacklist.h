/* playbacklist.h is part of lucifer a laser show controller.

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


#ifndef PLAYBACK_INCL
#define PLAYBACK_INCL

#include <qobject.h>
#include <qstringlist.h>
#include <qmutex.h>
#include <vector>

/// This is the list of things to play back on any given head
/// It gets updated by clicking to select or deselect frames in the
/// main window, and by using up frames as they get played out
class PlaybackList : public QObject
{
    Q_OBJECT
public:
    PlaybackList(QObject *parent = NULL);
    ~PlaybackList();
    enum SelectionModes {SINGLE=0,MULTIPLE,SHUFFLE};
    enum StepModes {ONCE=0,LOOP,MANUAL,BEAT};
		QStringList selectionModeList() const;
		QStringList stepModeList () const;
		bool isSelected (const int i);
public slots:
    void select (unsigned int pos, bool active);
    void setSelectionMode (enum SelectionModes mode);
    void setStepMode(enum StepModes mode);
    enum SelectionModes getSelectionMode() const;
    enum StepModes getStepMode () const;
    void manualStepClicked();
    void beatDetected();
    int getNextFramesource ();
		int getFrameSource ();
		void clear();
signals:
    /// Emitted whenever any selection in the list changes
    void selectionChanged (unsigned int pos, bool sel);
    /// Request to load the specified object
    void load (unsigned int pos);
    /// Emitted when we wish to replace the current selection
    void dumpCurrentSelection();
private:
		// Recursive mutex
		QMutex *lock;
    int next(bool running = true);
    std::vector <unsigned int> selections;
    enum SelectionModes selMode;
    enum StepModes stepMode;
    int currentIndex;
};
#endif





