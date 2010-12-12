/* engine_impl.h is part of lucifer a laser show controller.

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


/// Engine implementation details, mostly thread creation classes for various sub threads.

#ifndef ENGINE_IMPL
#define ENGINE_IMPL

#include <qthread.h>
#include "engine.h"
#include "head.h"

/// A tiny little shim to put each laser head into its own thread.
/// With essentially all modern CPUs being multicore this does much to
/// optimise performance.
/// HeadThread->start must be called to actually start any head running
class HeadThread : public QThread
{
    Q_OBJECT
public:
    HeadThread(Engine * e);
    virtual ~HeadThread();
    void run();
    LaserHeadPtr head;
private:
    Engine * engine;
};

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

/// A Thread that imports a foregin show file
class ShowImporter : public QThread
{
    Q_OBJECT
public:
    ShowImporter (Engine* engine_, QStringList fileName, int index = -1);
    ~ShowImporter();
    void run ();
signals:
    void imported();
private:
    QStringList name;
    Engine *e;
    int idx;
};
#endif
