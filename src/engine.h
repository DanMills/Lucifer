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
#include "frame.h"
#include "framesource.h"
#include "head.h"

/// Maximum number of laser heads supported on a single PC
#define MAX_HEADS (8)

/// A tiny little shim to put each laser head into its own thread.
/// With essentially all modern CPUs being multicore this does much to
/// optimise performance.
/// HeadThread->start must be called to actually start any head running
class HeadThread : public QThread
{
    Q_OBJECT
public:
    HeadThread();
    virtual ~HeadThread();
    void run();
    LaserHeadPtr head;
};

class Engine;

/// A Thread that saves the state of a show
class ShowSaver : public QThread
{
    Q_OBJECT
public:
    ShowSaver (Engine* engine_, QXmlStreamWriter* w_);
    ~ShowSaver();
    void run ();
signals:
    void saved();
private:
    QXmlStreamWriter * w;
    Engine *e;
};

/// A thread that loads a show
class ShowLoader : public QThread
{
	Q_OBJECT
	public:
		ShowLoader (Engine* engine_, QXmlStreamReader* r_);
		~ShowLoader();
		void run ();
	signals:
		void saved();
	private:
		QXmlStreamReader * r;
		Engine *e;
};


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

signals:
    void frameSourceChanged (unsigned long int pos, FrameSourcePtr newSource);
    void showLoaded ();
    void showSaved();

public slots:
    /// Kill all laser output, open the interlocks and scram the pile,
    /// Called by the emergency stop signal (Also WDT timeouts and such).
    void kill ();
private slots:
    void Saved();
    void Loaded();
private:
    /// Lock held while reading or writing the sources vector as this is prone to reallocation
    /// and I am not sure if it is atomic.
    QReadWriteLock source_lock;
    std::vector <FrameSourcePtr> sources;
    HeadThread heads[MAX_HEADS];
    ShowSaver *saver;
    QMutex save_mutex;
    QFile savef;
    QMutex load_mutex;
    QFile loadf;
		ShowLoader *loader;
};

typedef boost::shared_ptr<Engine> EnginePtr;
#endif
