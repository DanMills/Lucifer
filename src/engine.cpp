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
// MIDI
#include "midi.h"
#include "alsamidi.h"
#include "controlsurface.h"

// Unix specific threads stuff (hard RT, things of that nature)
#if __unix
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#endif
#include "motormix.h"
#include "mime.h"
#include <QtCore>

EngineStarter::EngineStarter(QObject *parent): QThread(parent)
{
}

EngineStarter::~EngineStarter()
{
    quit();
    while (!isFinished()) {
        usleep (10000);
    }
    e = EnginePtr();
}

void EngineStarter::run()
{
    slog()->debugStream() << "Starting laser engine thread " << std::hex << currentThreadId();
    /// TODO - Come up with a way to put things into windows soft RT scheduling class
#if __unix
    pthread_t tid = pthread_self();
    slog()->infoStream() << "Started show engine";
    struct sched_param sp;
    sp.sched_priority = 30;
    int err;
    err = pthread_setschedparam(tid,SCHED_FIFO,&sp);
    if (err) {
        slog()->errorStream() <<"Failed to set RT scheduling for laser engine control thread :" << strerror(err);
    } else {
        slog()->infoStream() <<"Set posix RT scheduling for engine thread";
    }
#endif
    e = boost::make_shared<Engine>();
    exec();
}

EnginePtr EngineStarter::engine()
{
    int i=0;
    do { // wait for the laser engine to come up
        if (e) {
            return e;
        }
        usleep (10000);
        i++;
    } while (i <500);
    slog()->critStream () << " Unable to start laser engine thread!";
    return e;
}


Engine::Engine(QObject* parent) : QObject(parent)
{
    saver = NULL;
    loader = NULL;
    importer = NULL;
    selected_head = 0;
    slog()->infoStream() << "New laser show engine created : " << this;
    emit message (tr("Starting show engine"),5000);
    QSettings settings;
    settings.beginGroup("Engine");
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        emit message(tr("Starting projector head"),5000);
        heads[i]=new HeadThread (this);
        startHead(i);// Bring up the laser head thread
    }
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        while (!heads[i]->head) { // Wait for the heads to come up
            usleep (10000);
        }
        connect (&(*heads[i]->head),SIGNAL(selectionChanged (uint, bool)),this,SLOT(selectionChangedData(uint,bool)));
        getHead(i)->setDriver("Dummy (ILDA)");
        getHead(i)->getDriver()->enumerateHardware();
        getHead(i)->getDriver()->connect(0);
    }
    // configure the midi interface
    settings.beginGroup("Midi");
    setMIDICard(settings.value("Device",QString("")).toString());
    for (unsigned int i=0; i < 15; i++) {
        surfaces[i] = NULL;
        setMIDIChannelDriver (i,settings.value(QString().sprintf("Controller %d",i+1),QString("")).toString());
        if (surfaces[i]) {
            surfaces[i]->connectEngine (this);
        }
    }
    settings.endGroup();
    settings.endGroup();
    emit message (tr("Laser projection system is ready"),5000);
}

void Engine::setMIDICard(QString name)
{
    //Set the midi card name
    // annoyingly ALSA does not react real well to pluggable USB midi devices (the device ID changes)
    // So we try using the device name reoprted by the driver.
    if (name.isNull()) {
        return;
    }
    // First get the system list of midi ports
    std::vector <std::pair<QString,QString> > ports;
    ports = AlsaMidi::enumeratePorts();
    slog()->debugStream() <<"MIDI PORTS: " << ports.size();
    for (unsigned int i=0; i < ports.size(); i++) {
        slog()->debugStream() << ports[i].first.toUtf8().constData() << " ::: " << ports[i].second.toUtf8().constData();
    }
    unsigned int i;
    for (i = 0; i < ports.size(); i++) {
        if (ports[i].second == name) {
            break;
        }
    }
    QString fn;
    if (i < ports.size()) {
        // Got a live one
        midiPortName = name;
        fn = ports[i].first;
    } else {
        midiPortName = "None";
    }
    slog()->infoStream() << "Setting midi device to id: " << fn.toUtf8().constData();
    midiPort.open(fn.toUtf8().constData());
    midiParser.setQIODevice(&midiPort);
    QSettings settings;
    settings.beginGroup("Engine");
    settings.beginGroup("Midi");
    settings.setValue("Device",name);
    settings.endGroup();
    settings.endGroup();
}

void Engine::setMIDIChannelDriver(unsigned int channel, QString driver)
{
    if (driver.isNull()) {
        return;
    }
    if (channel > 15) {
        slog()->errorStream()<<"MIDI, invalid channel";
        return;
    }
    if (surfaces[channel]) {
        delete surfaces[channel];
        surfaces[channel] = NULL;
    }
    if (driver == QString ("MotorMix")) {
        surfaces[channel] = new MotorMix ();
    }
    if (surfaces[channel]) {
        surfaces[channel]->connectMidi (&midiParser.channels[channel]);
    }
    QSettings settings;
    settings.beginGroup("Engine");
    settings.beginGroup("Midi");
    settings.setValue(QString().sprintf("Controller %d",channel+1),driver);
    settings.endGroup();
    settings.endGroup();
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

bool Engine::addFrameSource(SourceImplPtr fs,long int pos)
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
        emit sourcesSizeChanged (sources.size());
    }
    emit frameSourceChanged (pos);
    return true;
}

SourceImplPtr Engine::getFrameSource(const size_t pos)
{
    SourceImplPtr ret;
    QReadLocker locker(&source_lock);
    if (pos < sources.size()) {
        ret = sources[pos];
    }
    return ret;
}

PlaybackPtr Engine::getPlayback(const size_t pos)
{
    PlaybackPtr pb = boost::make_shared<Playback>(getFrameSource(pos));
    return pb;
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
    SourceImplPtr s = getFrameSource(source);
    if (s) {
        SourceImplPtr d = s->clone();
        addFrameSource(d,dest);
        return true;
    }
    return false;
}

QStringList Engine::mimeTypes() const
{
    QStringList l =  LaserMimeObject::mimeTypes();
    l.push_back (QString("text/uri-list"));
    return l;
}

bool Engine::mimeHandler(const QMimeData* data, int pos)
{

    // There is an additional type handled here....
    if (data->hasFormat("text/uri-list")) {
        QList<QUrl> urls = data->urls();
        if (urls.size() >0) {
            QStringList l;
            for (int i=0; i < urls.size(); i++) {
                l.append(urls[i].toLocalFile());
            }
            importShow(l,pos);
        }
        return true;
    } else {
        SourceImplPtr p = LaserMimeObject::getSource(data);
        if (p) {
            addFrameSource(p,pos);
            return true;
        }
    }
    return false;
}

QMimeData* Engine::mimeData(int pos)
{
    LaserMimeObject *mime = new LaserMimeObject;
    mime->setFrame(getFrameSource(pos));
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
    slog()->critStream() << "Laser output killed!";
}

void Engine::restart()
{
    slog()->critStream() << "Laser output restarted!";
    for (unsigned int i=0; i < MAX_HEADS; i++) {
        LaserHeadPtr h = getHead(i);
        if (h) {
            h->restart();
            DriverPtr d = h->getDriver();
            if (d) {
                d->ILDAShutter(true);
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
        saveCompressor->setStreamFormat(QtIOCompressor::GzipFormat);
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
        loadCompressor->setStreamFormat(QtIOCompressor::GzipFormat);
        if (!loadCompressor->open(QFile::ReadOnly)) {
            delete loadCompressor;
            load_mutex.unlock();
            slog()->errorStream() << "Couldn't open file for reading : " << loadf.errorString().toStdString();
            return false;
        }
        if (clear) {
            for (unsigned int i=0; i < sources.size(); i++) {
                SourceImplPtr p;
                SourceImplPtr op;
                op = getFrameSource(i);
                source_lock.lockForWrite();
                sources[i] = p;
                source_lock.unlock();
                if (op != p) {
                    emit frameSourceChanged (i);
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
    w->writeAttribute("Date",QDateTime::currentDateTime().toString());
    for (unsigned int i=0; i < e->getSourcesSize(); i++) {
        SourceImplPtr p = e->getFrameSource(i);
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
                if (r->tokenType()==QXmlStreamReader::Invalid){
                    slog()->errorStream() <<"Xml error : " << r->errorString().toStdString();
                    break;
                }
                assert (r->tokenType() == QXmlStreamReader::StartElement);
                SourceImplPtr fs = FrameSource_impl::loadFrames (r);
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
        SourceImplPtr fs = loader.load(name[i],err, false);
        if (fs) {
            e->addFrameSource(fs,idx);
            idx = -1;
        } else {
            // ILDA Load error
        }
    }
}

void Engine::clicked(const int pos)
{
    LaserHeadPtr h = getHead(selected_head);
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

void Engine::selectOutputEffect (int effect)
{
    emit outputEffectSelected(effect);
}

void Engine::selectionChangedData(unsigned int pos, bool active)
{
    if (!active) {
        emit setIndicator (pos,Qt::black);
    } else {
        emit setIndicator (pos,Qt::red);
    }
}

void Engine::manualNext()
{
    emit manualTrigger();
}

void Engine::selectHead(int head)
{
    if (head < MAX_HEADS) {
        selected_head = head;
        emit headSelectionChanged (head);
    }
    LaserHeadPtr h = getHead(selected_head);
    for (unsigned int i=0; i <getSourcesSize(); i++) {
        selectionChangedData(i,h->isSelected(i));
    }
}


