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
#include "engine_impl.h"
#include "log.h"
#include "loadilda.h"
#if __unix
#include <unistd.h>
#include <pthread.h>
#endif

HeadThread::HeadThread(Engine * e)
{
    engine = e;
}

HeadThread::~HeadThread()
{
}

void HeadThread::run()
{
    slog()->debugStream() << "Starting laserhead thread " << std::hex << currentThreadId();
    /// TODO - Come up with a way to put things into windows soft RT scheduling class
#if __unix
    pthread_t tid = pthread_self();
    struct sched_param sp;
    sp.sched_priority = 20;
    int err;
    err = pthread_setschedparam(tid,SCHED_FIFO,&sp);
    if (err) {
        slog()->errorStream() <<"Failed to set RT scheduling for laserhead control thread :" << strerror(err);
    } else {
        slog()->infoStream() <<"Set posix RT scheduling for projection thread";
    }
#endif
    head = boost::make_shared<LaserHead>(engine);
    exec();
}

Engine::Engine(QObject* parent) : QObject(parent)
{
    saver = NULL;
    loader = NULL;
    importer = NULL;
    slog()->infoStream() << "New laser show engine created : " << this;
    emit message (tr("Starting show engine"),5000);
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        emit message(tr("Starting projector head"),5000);
        heads[i]=new HeadThread (this);
        startHead(i);// Bring up the laser head thread
    }
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        while (!heads[i]->head) { // Wait for the head to come up
            usleep (10000);
        }
        connect (&(*heads[i]->head),SIGNAL(selectionChanged (uint, bool)),this,SLOT(selectionChangedData(uint,bool)));
        getHead(i)->setDriver("Dummy (ILDA)");
        getHead(i)->getDriver()->enumerateHardware();
        getHead(i)->getDriver()->connect(0);
    }
    emit message (tr("Laser projection engine is up and running"),5000);
}

Engine::~Engine()
{
    slog()->infoStream() << "Stopping show engine";
    kill();
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        if (heads[i]) {
            heads[i]->quit();
        }
    }
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        ///UGLY Hack to get the threads stopped before we exit
        while (heads[i] && (!heads[i]->isFinished())) {
            usleep (20000);
        }
    }
    // make sure any file save threads have finished.
    while (!save_mutex.tryLock()) {
        usleep (20000);
    }
    //make sure any file load threads have finished
    while (!load_mutex.tryLock()) {
        usleep (20000);
    }
    load_mutex.unlock();
    save_mutex.unlock();
    slog()->infoStream() << "Laser show engine deleted : " << this;
}

bool Engine::addFrameSource(FrameSourcePtr fs,long int pos)
{
    slog()->debugStream() << "Adding framesource " << fs << " to engine " << this <<" at index " << pos;
    bool resized= false;
    QWriteLocker locker(&source_lock);
    if (pos < 0) {
        for (unsigned int i = 0; i < sources.size(); i++) {
            if (!sources[i]) { // Find the first empty slot
                pos = i;
                break;
            }
        }
        if (pos < 0) {// Or create a new one
            pos = sources.size();
        }
    }
    if (sources.size() <= (unsigned long) pos) {
        //extend the sources array to accomodate pos+1 objects
        sources.resize(pos+1);
        resized = true;
    }
    sources[pos] = fs;
    locker.unlock();
    if (resized) {
        emit sourcesSizeChanged (pos+1);
    }
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
        ret = heads[pos]->head;
    } else {
        slog()->errorStream() << "Attempted to get invalid head " << pos;
    }
    return ret;
}

bool Engine::copy(unsigned int dest, unsigned int source)
{
    slog()->infoStream() << "Copying frameSource from index " << source << " to " << dest;
    FrameSourcePtr s = getFrameSource(source);
    if (s) {
        FrameSourcePtr d = s->clone();
        addFrameSource(d,dest);
        return true;
    }
    return false;
}

std::vector<std::string> Engine::mimeTypes(bool local) const
{
    std::vector <std::string> res;
    if (local) { // copy with this application instance so an engine location ID is sufficient
        res.push_back(std::string("Text/x-index"));
    }
    res.push_back(std::string("Text/x-FrameSource"));
    res.push_back(std::string("text/uri-list"));
    return res;
}



bool Engine::mimeHandler(const QMimeData* data, int pos)
{
    if (data->hasFormat("Text/x-index")) {
        QString id(data->data("Text/x-index").constData());
        int source_id = id.toInt();
        slog()->infoStream() <<"Dropped copy onto of " << source_id << " onto " << pos;
        // The copy can potentially take a while so run it in the background
        QtConcurrent::run(this,&Engine::copy,pos,source_id);
        return true;
    } else if (data->hasFormat("Text/x-FrameSource")) {
        std::string s((char *) data->data("x-Text/FrameSource").constData());
        FrameSourcePtr f = FrameSource::fromString(s);
        if (f) {
            slog()->infoStream() << "Dropped framesource into position  "<< pos;
            addFrameSource(f,pos);
            return true;
        }
        return false;
    } else if (data->hasFormat("text/uri-list")) {
        QList<QUrl> urls = data->urls();
        if (urls.size() >0) {
            QStringList l;
            for (int i=0; i < urls.size(); i++) {
                l.append(urls[i].toLocalFile());
            }
            importShow(l,pos);
            pos = -1;
            return true;
        }
        return false;
    }
    return false;
}

QMimeData* Engine::mimeData(int pos)
{
    QMimeData *mime = new QMimeData;
    mime->setData("Text/x-index",QString().number(pos).toAscii());
    return mime;
}

bool Engine::startHead(const size_t pos)
{
    assert (heads[pos]);
    if (pos < MAX_HEADS) {
        if (!heads[pos]->isRunning()) {
            heads[pos]->start(QThread::HighestPriority);
        } else {
            slog()->errorStream() << "Attempted to start head that was already running";
            return false;
        }
    } else {
        slog()->errorStream() << "Attempted to start head "<< pos << ", but there are only " << int(MAX_HEADS);
        return false;
    }
    slog()->infoStream()<< "Head " << pos << " Started";
    emit message (tr("Head started"),5000);
    return true;
}

void Engine::kill()
{
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        LaserHeadPtr h = getHead(i);
        if (h) {
            h->kill();
            DriverPtr d = h->getDriver();
            if (d) {
                d->ILDAShutter(false);
            }
        }
    }
}

void Engine::Saved()
{
    saveCompressor->close();
    delete saveCompressor;
    save_mutex.unlock();
    slog()->debugStream() << "Show saved, thread terminated";
    emit showSaved();
    emit message(tr("Show Saved"),5000);
}

void Engine::Loaded()
{
    loadCompressor->close();
    delete loadCompressor;
    load_mutex.unlock();
    slog()->debugStream() << "Show loaded, thread terminated";
    emit showLoaded();
    emit message(tr("Show Loaded"),5000);
}


bool Engine::saveShow(QString filename)
{
    slog()->infoStream() << "Saving file : " <<filename.toStdString();
    emit message(tr("Saving File : ") + filename,5000);
    if (save_mutex.tryLock()) {
        if (saver) {
            delete saver;
        }
        savef.setFileName (filename);
        saveCompressor = new QtIOCompressor (&savef,6,10 * 1024 * 1024);
        if (!saveCompressor->open(QIODevice::WriteOnly)) {
            delete saveCompressor;
            save_mutex.unlock();
            slog()->errorStream() << "Couldn't open file for writing : " << saveCompressor->errorString().toStdString();
            emit message (tr("Failed to save file : ") + filename,10000);
            return false;
        }
        QXmlStreamWriter *w = new QXmlStreamWriter(saveCompressor);
        saver = new ShowSaver(this,w);
        connect (saver,SIGNAL(finished()),this,SLOT(Saved()));
        slog()->debugStream() << "Starting file saver thread";
        saver->start(QThread::LowPriority);
        return true;
    } else {
        slog()->errorStream() << "Show save already in progress";
        emit message (tr("Show save already in progress"),5000);
        return false;
    }
}


bool Engine::loadShow(QString filename, const bool clear)
{
    slog()->infoStream() << "Loading file : " << filename.toStdString();
    if (load_mutex.tryLock()) {
        if (loader) {
            delete loader;
        }
        loadf.setFileName(filename);
        loadCompressor = new QtIOCompressor (&loadf,6,10 *1024 * 1024);
        if (!loadCompressor->open(QFile::ReadOnly)) {
            delete loadCompressor;
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
        QXmlStreamReader *r = new QXmlStreamReader(loadCompressor);
        loader = new ShowLoader(this,r);
        connect (loader,SIGNAL(finished()),this,SLOT(Loaded()));
        slog()->debugStream() << "Starting file loader thread";
        loader->start();
        return true;
    } else {
        slog()->errorStream() << " Another file loader is allready running";
        return false;
    }
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

bool Engine::importShow(QStringList filenames, int index)
{
    slog()->infoStream() << "Importing show from at index " << index;
    if (import_mutex.tryLock()) {
        // We got the lock!
        if (importer) {
            delete importer;
        }
        importer = new ShowImporter(this,filenames,index);
        emit message (tr("Importing frames"),5000);
        connect (importer,SIGNAL(finished()),this,SLOT(Imported()));
        slog()->debugStream() << "Starting file importer thread";
        importer->start();
        return true;
    } else {
        slog()->errorStream() <<"Another import is already running.";
        message (tr("Another import is running!"),20000);
        return false;
    }
}


void Engine::Imported()
{
    import_mutex.unlock();
    slog()->debugStream() << "file Imported, thread terminated";
    emit showImported();
    emit message (tr("Frames imported"),5000);
}



ShowImporter::ShowImporter (Engine* engine_, QStringList fileName, int index)
{
    e=engine_;
    name = fileName;
    idx = index;
}
ShowImporter::~ShowImporter()
{
}

void ShowImporter::run()
{
    for (int i=0; i < name.size(); i++) {
        Ildaloader loader;
        unsigned int err=0;
        FrameSourcePtr fs = loader.load(name[i],err, false);
        if (fs) {
            e->addFrameSource(fs,idx);
        } else {
            // ILDA Load error
        }
    }
}

void Engine::clicked(const int pos)
{
    LaserHeadPtr h = getHead(0);
    if (h) {
        h->select(pos,!(h->isSelected(pos)));
    }
}

void Engine::deselect(const int pos)
{
    for (int i=0; i < MAX_HEADS; i++) {
        LaserHeadPtr h = getHead(i);
        if (h) {
            h->select (pos,false);
        }
    }
}

void Engine::selectionChangedData(unsigned int pos, bool active)
{
    if (!active) {
        emit setIndicator (pos,Qt::black);
    } else {
        emit setIndicator (pos,Qt::red);
    }
}


