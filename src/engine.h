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

/// Maximum number of laser heads supported on a single PC
#define MAX_HEADS (8)

/// things from engine_impl.h
class ShowSaver;
class ShowLoader;
class ShowImporter;
class HeadThread;

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
    bool addFrameSource (FrameSourcePtr fs, long int pos);
    FrameSourcePtr getFrameSource (const size_t pos);
    LaserHeadPtr getHead(const size_t pos);
    bool startHead(const size_t pos);
    size_t getSourcesSize() const;

    bool loadShow (QString filename, const bool clear = true);
    bool saveShow (QString filename);
    bool importShow (QStringList filenames, int index = -1);

    /// Copies the frame souce in source to dest.
    /// source and dest are indicies into the sources table.
    bool copy (unsigned int dest, unsigned int source);
    /// Returns a list of mime types understood by the engine
    std::vector <std::string> mimeTypes(bool local = true) const;
    /// Returns a mime data set corresponding to a frame source
    QMimeData * mimeData(int pos);

signals:
    /// Emitted whenever a framesource is replaced in the main sources vector
    void frameSourceChanged (unsigned long int pos, FrameSourcePtr newSource);
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

public slots:
    /// Kill all laser output, open the interlocks and scram the pile,
    /// Called by the emergency stop signal (Also WDT timeouts and such).
    void kill ();
		/// Undo a kill
		void restart();
    /// Deal with drag and drop events.
    bool mimeHandler (const QMimeData * data, int pos);
    /// A frame select button has been clicked
    void clicked(const int pos);
    /// force a deselect
    void deselect (const int pos);
private slots:
    void Saved();
    void Loaded();
    void Imported();
    void selectionChangedData(unsigned int, bool);
private:
    /// Lock held while reading or writing the sources vector as this is prone to reallocation
    /// and I am not sure if that is atomic.
    QReadWriteLock source_lock;
    std::vector <FrameSourcePtr> sources;
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
};

#endif
