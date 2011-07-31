/* engine.h is part of lucifer a laser show controller.

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

#ifndef ENGINE_INCL
#define ENGINE_INCL

#include <QtCore>
#include <boost/shared_ptr.hpp>
#include "frame.h"
#include "framesource.h"
#include "qtiocompressor.h"

class Engine;
typedef boost::shared_ptr<Engine> EnginePtr;

#include "head.h"
#include "alsamidi.h"
#include "midi.h"
#include "controlsurface.h"

/// Maximum number of laser heads supported on a single PC
#define MAX_HEADS (6)

/// things from engine_impl.h
class ShowSaver;
class ShowLoader;
class ShowImporter;
class HeadThread;

/// \brief  A little shim that starts the show engine in its own thread
///so the GUI event loop cannot block the engine from serving up new frame sources to the heads.
class EngineStarter : public QThread
{
    Q_OBJECT
public:
    EngineStarter(QObject *parent = NULL);
    virtual ~EngineStarter();
    /// Start the engine thread
    void run();
    ///  Obtain a reference counted pointer to the engine.
    EnginePtr engine();
private:
    EnginePtr e;
};

/// \brief The main laser show engine responsible for  serving framesources to the heads.
/// This object holds all the basic frame sources (master copies of the reference counted
/// ptrs) in sources, so deleting one will probably eventually result in that framesource
/// tree going away once nothing else is using it, it also holds the laser heads and deals
/// with their threads.
/// This is NOT a GUI object (and must never call a GUI method directly) so we can throw
/// threads for background processes around with impunity.

class Engine : public QObject
{
    Q_OBJECT
public:
    Engine(QObject *parent = NULL);
    ~Engine();
    /// \brief Adds a framesource reference counted pointer to the laster list of available sources.
    /// @param[in] fs is a reference counted pointer to an object derived from  class FrameSource.
    /// @param[in] pos is the index to store the reference in. (-1 causes the engine to use the next available slot).
    /// @return true on sucess, false on failure.
    /// Causes frameSourceChanged to be emitted.
    bool addFrameSource (SourceImplPtr fs, long int pos);
    /// \brief Returns a reference counted pointer to the FrameSource indexed at pos.
    /// @param[in] pos is the index of the FrameSource to return.
    /// @return a reference counted pointer to the FrameSource at position pos;
    SourceImplPtr getFrameSource (const size_t pos);
    /// \brief Create and return a reference counted  pointer to a new Playback appropriate to the FrameSource at pos.
    /// @param[in] pos is the index of the FrameSource that we need a playback for.
    /// @return a PlaybackPtr reference counted pointer.
    PlaybackPtr getPlayback(const size_t pos);
    /// \brief Looks up a specified laser projection head and returns a ref. counted pointer to it.
    /// @param[in] pos is the number of the head to return.
    /// @return a reference counted pointer to a projection head.
    LaserHeadPtr getHead(const size_t pos);
    /// \brief Starts a laser projection head in its own thread.
    /// @param[in] pos is the head number to start.
    /// @return true on success, false on error.
    bool startHead(const size_t pos);
    /// \brief Get the number of sources known to the engine.
    /// @return the number of framesources registered with the engine.
    size_t getSourcesSize() const;
    /// \brief Load a show (a set of frame sources), this starts the process but it really runs async in its own thread.
    /// @param[in] filename is the name of the .lsf file to load.
    /// @param[in] clear is true if the old show is to be dumped, false if this should be added to the set of loaded frames.
    /// @return false on error, true on success.
    bool loadShow (QString filename, const bool clear = true);
    /// \brief Start saving a show.
    /// @param[in] filename is the name of the file to save, it should probably end in .lsf.
    /// @return true on success, false on error.
    bool saveShow (QString filename);
    /// \brief Import one or more foregin files (ILDA or such).
    /// @param[in] filenames is a list of filenames to import.
    /// @param[in] index is the first location into which to store the resulting FrameSource (-1 means first enpty).
    /// @return true on sucess, false on failure.
    bool importShow (QStringList filenames, int index = -1);

    /// \brief Copies the frame souce in source to dest.
    /// source and dest are indicies into the sources table.
    bool copy (unsigned int dest, unsigned int source);
    /// Returns a list of mime types understood by the engine.
    QStringList mimeTypes () const;
    /// \brief Returns a mime data set corresponding to a frame source.
    /// @param[in] pos is the index of the frame to supply as a mime data set.
    /// @return mime data corresponding to the requested frame.
    /// \note the object returned is really a LaserMimeObject which is derived from the QMimeData class.
    QMimeData * mimeData(int pos);

signals:
    /// Emitted whenever a framesource is replaced in the main sources vector
    void frameSourceChanged (unsigned long int pos);
    /// Emitted when the sise of the sources vector changes
    void sourcesSizeChanged (size_t);
    /// Show file has completed loading
    void showLoaded ();
    /// show file has finished saving
    void showSaved();
    /// show imported
    void showImported();
    /// Status message
    void message (QString text, int time);
    void setIndicator (unsigned int pos, QColor col);
    void manualTrigger();
    /// head Selection changed
    void headSelectionChanged (int);

public slots:
    /// Kill all laser output, open the interlocks and scram the pile,
    /// Called by the emergency stop signal (Also WDT timeouts and such).
    void kill ();
    /// Restart projection shutdown by a kill.
    void restart();
    /// Deal with drag and drop events.
    bool mimeHandler (const QMimeData * data, int pos);
    /// A frame select button has been clicked
    void clicked(const int pos);
    /// force a deselect
    void deselect (const int pos);
    /// Called to manually go to next framesource in manual mode
    void manualNext();
    /// called to change the head selections will effect
    void selectHead (int head);
    /// MIDI driver parameters
    void setMIDICard (QString name);
    /// MIDI Channel drivers
    void setMIDIChannelDriver(unsigned int channel, QString driver);

private slots:
    void Saved();
    void Loaded();
    void Imported();
    void selectionChangedData(unsigned int, bool);
private:
    /// Lock held while reading or writing the sources vector as this is prone to reallocation
    /// and I am not sure if that is atomic.
    QReadWriteLock source_lock;
    std::vector <SourceImplPtr> sources;
    /// The laser projection heads
    HeadThread *heads[MAX_HEADS];
    /// File IO threads and associated locks
    ShowSaver *saver;
    QtIOCompressor * saveCompressor;
    QMutex save_mutex;
    QFile savef;
    QMutex load_mutex;
    QFile loadf;
    ShowLoader *loader;
    QtIOCompressor * loadCompressor;
    QMutex import_mutex;
    ShowImporter *importer;
    int selected_head;
    // MIDI interface
    QString midiPortName;
    QString midiDrivers[16];

    AlsaMidi midiPort;
    MIDIParser midiParser;
    ControlSurface * surfaces[16];
};

#endif
