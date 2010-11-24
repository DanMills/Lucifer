#ifndef PLAYBACK_INCL
#define PLAYBACK_INCL

#include <qobject.h>
#include <qstringlist.h>
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
    enum SelectionModes {SINGLE,MULTIPLE,SHUFFLE};
    enum StepModes {ONCE,LOOP,MANUAL,BEAT};
		QStringList selectionModeList() const;
		QStringList stepModeList () const;
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
signals:
    /// Emitted whenever any selection in the list changes
    void selectionChanged (unsigned int pos, bool sel);
    /// Request to load the specified object
    void load (unsigned int pos);
    /// Emitted when we wish to replace the current selection
    void newCurrentSelection (unsigned int pos, bool active);
private:
    int next(bool running = true);
    std::vector <unsigned int> selections;
    enum SelectionModes selMode;
    enum StepModes stepMode;
    int currentIndex;
};
#endif





