/* engine.cpp is part of lucifer a laser show controller.

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


#include "engine.h"
#include "log.h"
#include <unistd.h>

HeadThread::HeadThread()
{
    head = boost::make_shared<LaserHead>();
}

HeadThread::~HeadThread()
{
}

void HeadThread::run()
{
    slog()->debugStream() << "Starting laserhead thread " << currentThreadId();
    exec();
}

Engine::Engine(QObject* parent) : QObject(parent)
{
    saver = NULL;
    loader = NULL;
    slog()->infoStream() << "New laser show engine created : " << this;
}

Engine::~Engine()
{
    kill();
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        heads[i].quit();
    }
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        ///UGLY Hack to get the threads stopped before we exit
        while (!heads[i].isFinished()) {
            usleep (20000);
        }
    }
    slog()->infoStream() << "Laser show engine deleted : " << this;
}

bool Engine::addFrameSource(FrameSourcePtr fs,long int pos)
{
    slog()->debugStream() << "Adding framesource " << fs << " to engine " << this <<" at index " << pos;
    QWriteLocker locker(&source_lock);
    if ((pos >= 0) && (sources.size() <= (unsigned long) pos)) {
        //extend the sources array to accomodate pos+1 objects
        sources.resize(pos+1);
    }
    if (pos < 0) {
        pos = sources.size();
    }
    sources[pos] = fs;
    locker.unlock();
    emit frameSourceChanged (pos, fs);
    return true;
}

FrameSourcePtr Engine::getFrameSource(const size_t pos)
{
    FrameSourcePtr ret;
    QReadLocker locker(&source_lock);
    if (pos < sources.size()) {
        ret = sources[pos];
    }
    return ret;
}

size_t Engine::getSourcesSize() const
{
    return sources.size();
}

LaserHeadPtr Engine::getHead(const size_t pos)
{
    LaserHeadPtr ret;
    if (pos < MAX_HEADS) {
        ret = heads[pos].head;
    } else {
        slog()->errorStream() << "Attempted to get invalid head " << pos;
    }
    return ret;
}

bool Engine::startHead(const size_t pos)
{
    if (pos < MAX_HEADS) {
        if (!heads[pos].isRunning()) {
            heads[pos].start(QThread::HighestPriority);
            /// TODO make the thread posix RT if on linux
        } else {
            slog()->errorStream() << "Attempted to start head that was already running";
            return false;
        }
    } else {
        slog()->errorStream() << "Attempted to start head "<< pos << ", but there are only " << int(MAX_HEADS);
        return false;
    }
    slog()->infoStream()<< "Head " << pos << " Started";
    return true;
}

void Engine::kill()
{
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        getHead(i)->loadFrameSource(FrameSourcePtr(), true);
        getHead(i)->getDriver()->ILDAShutter(false);
    }
}

void Engine::Saved()
{
    savef.close();
    save_mutex.unlock();
    slog()->debugStream() << "Show saved, thread terminated";
    emit showSaved();
}

void Engine::Loaded()
{
    loadf.close();
    load_mutex.unlock();
    slog()->debugStream() << "Show loaded, thread terminated";
    emit showLoaded();
}


bool Engine::saveShow(QString filename)
{
    slog()->infoStream() << "Saving file : " <<filename.toStdString();
    if (saver) {
        delete saver;
    }
    save_mutex.lock();
    savef.setFileName(filename);
    if (!savef.open(QFile::WriteOnly)) {
        save_mutex.unlock();
        slog()->errorStream() << "Couldn't open file for writing : " << loadf.errorString().toStdString();
        return false;
    }
    QXmlStreamWriter *w = new QXmlStreamWriter(&savef);
    saver = new ShowSaver(this,w);
    connect (saver,SIGNAL(finished()),this,SLOT(Saved()));
    slog()->debugStream() << "Starting file saver thread";
    saver->start(QThread::LowPriority);
    return true;
}


bool Engine::loadShow(QString filename, const bool clear)
{
    slog()->infoStream() << "Loading file : " << filename.toStdString();
    if (loader) {
        delete loader;
    }
    load_mutex.lock();
    loadf.setFileName(filename);
    if (!loadf.open(QFile::ReadOnly)) {
        load_mutex.unlock();
        slog()->errorStream() << "Couldn't open file for reading : " << loadf.errorString().toStdString();
        return false;
    }
    if (clear) {
        for (unsigned int i=0; i < sources.size(); i++) {
            FrameSourcePtr p;
            FrameSourcePtr op;
            op = getFrameSource(i);
            source_lock.lockForWrite();
            sources[i] = p;
            source_lock.unlock();
            if (op != p) {
                emit frameSourceChanged (i, p);
            }
        }
        // Clear the vector, the loader will add capacity as needed.
        source_lock.lockForWrite();
        sources.clear();
        source_lock.unlock();
    }
    QXmlStreamReader *r = new QXmlStreamReader(&loadf);
    loader = new ShowLoader(this,r);
    connect (loader,SIGNAL(finished()),this,SLOT(Loaded()));
    slog()->debugStream() << "Starting file loader thread";
    loader->start();
    return true;
}


ShowSaver::ShowSaver(Engine * engine_, QXmlStreamWriter *w_): QThread()
{
    e = engine_;
    w = w_;
}
ShowSaver::~ShowSaver()
{
}

void ShowSaver::run()
{
    w->setAutoFormatting(true);
    w->writeStartDocument();
    w->writeStartElement("Lucifer");
    w->writeAttribute("Version","1.1.0");
    for (unsigned int i=0; i < e->getSourcesSize(); i++) {
        FrameSourcePtr p = e->getFrameSource(i);
        if (p) {
            w->writeStartElement("Sequence");
            w->writeAttribute("Position",QString().number(i));
            p->saveFrames(w);
            w->writeEndElement();//Sequence
        }
    }
    w->writeEndElement();// Lucifer show
    w->writeEndDocument();
    delete w;
}


ShowLoader::ShowLoader(Engine* engine_, QXmlStreamReader* r_): QThread()
{
    e = engine_;
    r = r_;
}

ShowLoader::~ShowLoader()
{
}

void ShowLoader::run()
{
    while (!r->atEnd()) {
        // Find the document head and check that this is the correct file format
        if (r->name().toString() == "Lucifer") {
            slog()->debugStream() << "Found a Lucifer header";
            break;
        }
        r->readNext();
    }
    if ((!r->hasError ()) && (r->attributes().value("Version") == "1.1.0")) {
        // Ok, a V1.1.0 Lucifer file has been found
        // Next up sit in a loop reading sequence elements
        slog()->debugStream() << "File is version 1.1.0";
        while ((!r->atEnd()) && (!r->isEndDocument())) {
            if (r->isEndElement() && (r->name().toString() == "Lucifer")) {
                break;
            }
            while ((!r->atEnd()) && (!r->isStartElement()) && (r->name().toString() != "Sequence")) {
                slog()->infoStream() << "Loader looking for a sequence StartElement tag, got : "<<
                r->tokenType() << " : " << r->name().toString().toStdString();
                r->readNext();
            }
            if (r->isStartElement() && (r->name() == "Sequence")) {
                // something to load
                size_t index;
                index = r->attributes().value("Position").toString().toULong();
                slog()->infoStream()<<"Loading sequence at : " << index;
                r->readNextStartElement();
                assert (r->tokenType() == QXmlStreamReader::StartElement);
                FrameSourcePtr fs = FrameSource::loadFrames (r);
                e->addFrameSource(fs,index);
            } else {
                r->readNextStartElement();
            }
        }
    }
    if (r->hasError()) {
        slog()->errorStream() << "File read error :" << r->errorString().toStdString();
    }
    delete r;
}

