/*buttonwindow.cpp is part of lucifer a laser show controller.

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

/* $Id: buttonwindow.cpp 22 2010-07-24 15:01:51Z dmills $ */

#include <iostream>
#include <fstream>

#include "buttonwindow.h"
#include "loadilda.h"
#include <fstream>
#include "log.h"
#include "outputview.h"

ButtonWindow::ButtonWindow(EnginePtr e, QWidget* parent): QMainWindow(parent)
{
    engine = e;
    loadSettings();
    makeActions();

    QToolBar * toolbar = new QToolBar (this);
    toolbar->setMovable (false);
    toolbar->setFloatable(false);
    toolbar->addAction(openAct);
    toolbar->addAction(saveAct);
    toolbar->addAction(saveAsAct);
    toolbar->addSeparator();
    toolbar->addAction (fullScreenAct);
    toolbar->addAction (windowScreenAct);
    toolbar->addSeparator();

    connect (&(*engine),SIGNAL(message(QString,int)),this,SLOT(status(QString,int)));
    // This toolbar gets the output heads displayed as thumb nails on it
    //for (unsigned int i=0; i <8; i++) {
    //    DisplayFrame *f = new DisplayFrame(this);
    //    f->setGeometry(0,0,96,96);
    //    f->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    //    toolbar->addWidget (f);
    //}
    // Multiselect mode
    // Random
    // GPIO indicators
    // Midi indicator
    // Audio meters

    // And the SCRAM button
    mode = ButtonWindow::SINGLE;
    selectionMode = new QComboBox (toolbar);
    selectionMode->addItem(tr("Single"));
    selectionMode->addItem(tr("Multiple"));
    selectionMode->addItem(tr("Shuffle"));
    selectionMode->setToolTip(tr("Frame selection mode"));
    toolbar->addWidget(selectionMode);
    connect (selectionMode,SIGNAL(currentIndexChanged(int)),this,SLOT(selectionModeChanged(int)));


    stepMode = new QComboBox (toolbar);
    stepMode->addItem(tr("Manual"));
    stepMode->addItem(tr("Once"));
    stepMode->addItem(tr("Beat"));
    stepMode->setToolTip(tr("Frame loop end condition"));
    toolbar->addWidget(stepMode);
    toolbar->addAction(blankLasersAct);
    addToolBar(toolbar);
    QFrame *frame = new QFrame(this);
		setCentralWidget(frame);
    QVBoxLayout *vlayout = new QVBoxLayout(frame);
    frame->setLayout(vlayout);
    tabs = new QTabWidget();
    tabs->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QFrame *headframe = new QFrame();
    vlayout->addWidget(headframe);
    vlayout->addWidget(tabs);
    headframe->setFixedHeight(128);
    QHBoxLayout *hlayout = new QHBoxLayout (this);
    headframe->setLayout(hlayout);
    // Heads
    for (unsigned int i=0; i < MAX_HEADS; i++) {
				OutputView *ov = new OutputView (this);
        hlayout->addWidget(ov);
				connect ((&(*engine->getHead(i))),SIGNAL(newFrame(FramePtr)),ov,SLOT(updateDisplay(FramePtr)));
    }


    //setCentralWidget(tabs);
    ButtonGrid *g = new ButtonGrid(engine, 8,8,0,this);
    grids.push_back (g);
    // This connection ensures that the grids have a consistent view of what is loaded into the engine
    connect (&(*engine),SIGNAL(frameSourceChanged(unsigned long int, FrameSourcePtr)),
             g,SLOT(frameSourceChanged(unsigned long int,FrameSourcePtr)));
    // This ensures that the number of grids is sufficient for the number of frames loaded into the engine
    connect (&(*engine),SIGNAL(sourcesSizeChanged(size_t)),this,SLOT(sourcesSizeChanged(size_t)));
    tabs->addTab(grids[0],QString("&")+QString().number(1));
    connect (grids[0],SIGNAL(modified()), this, SLOT(modified()));
    connect (grids[0],SIGNAL(clicked(uint,uint,uint,bool)),
             this,SLOT(selectionChanged(uint,uint,uint,bool)));


    tabs->setCurrentIndex(0);
    fileMenu = menuBar()->addMenu(tr("&File"));
    editMenu= menuBar()->addMenu(tr("&Edit"));
    viewMenu = menuBar()->addMenu(tr("&View"));
    setupMenu= menuBar()->addMenu(tr("&Setup"));
    helpMenu= menuBar()->addMenu(tr("&Help"));

    fileMenu->addAction (openAct);
    fileMenu->addAction (saveAct);
    fileMenu->addAction (saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction (importAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    statusBar();
    show();
    unsaved = false;
    setCurrentFile(QString());
    //connect (&head,SIGNAL(endOfSource()),this,SLOT(nextFrameSource()));
    //OutputView *view = new OutputView (NULL);
    //connect ((&(*engine->getHead(0))),SIGNAL(newFrame(FramePtr)),view,SLOT(updateDisplay(FramePtr)));
    //view->show();

}

// Load window position and size preferences
void ButtonWindow::loadSettings()
{// Note that main has already set the application name and domain
    QSettings settings;
    settings.beginGroup("Button Window");
    resize(settings.value("size", QSize(600, 600)).toSize());
    move(settings.value("pos", QPoint(300, 300)).toPoint());
    bool fullscreen = settings.value("fullscreen",bool(false)).toBool();
    if (fullscreen) {
        setFullScreen();
    } else {
        clearFullScreen();
    }
    QString home;
    if (getenv ("HOME")) {
        home = getenv ("HOME");
    } else {
        home = "/";
    }
    pathName = settings.value("path",home).toString();
    slog()->debugStream()<< "default path set to : "<<pathName.toStdString();
    mode = (ButtonWindow::SELECTIONMODE) settings.value("selectionMode",ButtonWindow::SINGLE).toInt();
    //selectionMode->setCurrentIndex((int) mode);
    selectionModeChanged((int) mode);
    settings.endGroup();
}

// This saves the window position and size preferences
void ButtonWindow::storeSettings()
{
    QSettings settings;
    settings.beginGroup("Button Window");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("fullscreen",bool(windowState() & Qt::WindowFullScreen));
    settings.setValue ("path",pathName);
    settings.setValue("selectionMode", mode);
    settings.endGroup();
}

void ButtonWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        storeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void ButtonWindow::sourcesSizeChanged(size_t s)
{
    while (s > 64 * grids.size()) {
        // We dont have enough tabs to represent all the frameSources we have loaded.
        ButtonGrid *g = new ButtonGrid(engine, 8,8,64 * grids.size(),this);
        grids.push_back (g);
        // This connection ensures that the grids have a consistent view of what is loaded into the engine
        connect (&(*engine),SIGNAL(frameSourceChanged(unsigned long int, FrameSourcePtr)),
                 g,SLOT(frameSourceChanged(unsigned long int,FrameSourcePtr)));
        tabs->addTab(g,QString("&")+QString().number(grids.size()));
    }
}


bool ButtonWindow::maybeSave()
{
    if (unsaved) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Lucifer"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveFile();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

bool ButtonWindow::saveFile()
{
    if (fileName.isEmpty()) {
        return saveAsFile();
    } else {
        return saveFile(fileName);
    }
}

bool ButtonWindow::saveAsFile()
{
    QString fn = QFileDialog::getSaveFileName(this,
                 tr("Save File"), pathName, tr("Lucifer show (*.lsf)"));
    if (fn.isEmpty()) {
        return false;
    }
    return saveFile(fn);
}

void ButtonWindow::importFiles ()
{
    QSettings settings;
    settings.beginGroup ("ILDA Importer");
    QString home;
    if (getenv ("HOME")) {
        home = getenv ("HOME");
    } else {
        home = "/";
    }
    QString path = settings.value("path",home).toString();


    QStringList l = QFileDialog::getOpenFileNames (this,tr("Import Files"),
                    path,
                    tr("ILDA Files (*.ilda *.ild)"));
    if (l.size()) {
        settings.setValue ("path",QFileInfo(l[0]).absolutePath());
    }
    settings.endGroup();
    engine->importShow(l,-1);
    modified();
}

bool ButtonWindow::saveFile(const QString &fn)
{

    engine->saveShow(fn);
    statusBar()->showMessage(tr("File saving"), 20000);
    return true;
}

void ButtonWindow::status(QString text, int time)
{
    statusBar()->showMessage(text,time);
}


void ButtonWindow::openFile()
{
    if (maybeSave()) {
        QString fn = QFileDialog::getOpenFileName(this,
                     tr("Load File"), pathName, tr("Lucifer show (*.lsf)"));
        if (!fn.isEmpty())
        {
            loadFile(fn);
        }
    }
}

void ButtonWindow::loadFile(const QString &fn)
{ // Todo - this is painfully slow at the moment
    // QApplication::setOverrideCursor(Qt::WaitCursor);
    slog()->infoStream()<<"Loading show file : " << fn.toStdString();
    engine->loadShow(fn);
    statusBar()->showMessage(tr("File Loading"), 20000);
}

void ButtonWindow::setCurrentFile(const QString &fn)
{
    fileName = fn;
    unsaved = false;
    setWindowModified(false);
    slog()->debugStream()<<"Set current file name to : " << fn.toStdString();
    QString shownName;
    if (fileName.isEmpty()) {
        shownName = "untitled.lsf";
    } else {
        pathName = QFileInfo(fileName).absolutePath();
        slog()->debugStream() << "default path set to :" << pathName.toStdString();
        shownName = QFileInfo(fileName).fileName();
    }
    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Lucifer - frame selector")));
}

void ButtonWindow::modified()
{
    unsaved = true;
    setWindowModified (true);
}

void ButtonWindow::setFullScreen()
{
    setWindowState(windowState() | Qt::WindowFullScreen);
    slog()->infoStream()<<"Window mode set to full screen";
}

void ButtonWindow::clearFullScreen()
{
    setWindowState(windowState() & ~Qt::WindowFullScreen);
    slog()->infoStream()<<"Window mode set to windowed";
}

// I hate writing GUI code, so much endless boilerplate
void ButtonWindow::makeActions()
{
    // File menu actions
    openAct = new QAction (tr("&Open"),this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open a file"));
    openAct->setIcon(QIcon(":/icons/document-open.svg"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
    saveAct = new QAction (tr("&Save"),this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save a file"));
    saveAct->setIcon(QIcon(":/icons/document-save.svg"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));
    saveAsAct = new QAction (tr("Save As"),this);
    saveAsAct->setStatusTip(tr("Save a file"));
    saveAsAct->setIcon(QIcon(":/icons/document-save-as.svg"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsFile()));
    fullScreenAct = new QAction (tr("Fullscreen mode"),this);
    fullScreenAct->setIcon(QIcon(":/icons/view-fullscreen.svg"));
    // Window mode actions
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(setFullScreen()));
    windowScreenAct = new QAction (tr("Windowed mode"),this);
    windowScreenAct->setIcon(QIcon(":/icons/view-restore.svg"));
    connect(windowScreenAct, SIGNAL(triggered()), this, SLOT(clearFullScreen()));
    // Laser SCRAM switch
    blankLasersAct = new QAction (tr("&Kill Output"),this);
    blankLasersAct->setShortcut(tr("Ctrl+K"));
    blankLasersAct->setStatusTip(tr("Shutdown all laser output"));
    blankLasersAct->setIcon(QIcon(":icons/process-stop.svg"));
    // Import multiple
    importAct = new QAction (tr("Import files"),this);
    importAct->setStatusTip(tr("Import multiple files"));
    importAct->setIcon(QIcon(":/icons/document-open.svg"));
    connect(importAct, SIGNAL(triggered()),this, SLOT(importFiles()));

    exitAct = new QAction (tr("Quit"),this);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()),qApp, SLOT(quit()));
}

void ButtonWindow::selectionChanged(unsigned int x, unsigned int y, unsigned int id, bool down)
{
    if (down) {
        // Button pressed
        if (mode == ButtonWindow::SINGLE) {
            // Single select mode
            selections.clear();
            selections.push_back(Selection(id,x,y));
            for (unsigned int g = 0; g <grids.size(); g++) {
                if (g != id) {
                    grids[g]->clear();
                } else {
                    grids[g]->clear(x,y);// This excludes x,y from being cleared
                    // Push the frame out to the scan head
                    loadFrame();
                }
            }
        } else {// one of the multiselect modes
            selections.push_back(Selection(id,x,y));
            if (selections.size() == 1) {
                // First frame in a multi select, push it to the scanners
                loadFrame();
            }
        }
    } else {
        // release the selection
        if (selections.size()) {
            unsigned int i;
            for (i=0; i < selections.size(); i++) {
                if ((selections[i].grid() == id) &&
                        (selections[i].getX() == x) &&
                        (selections[i].getY() == y)) {
                    selections.erase(selections.begin()+i);
                    break;
                }
            }
            if (i == 0) {
                FrameSourcePtr p;
                // dump whatever is currently playing
                head.loadFrameSource(p, true);
            }
        }
    }
    slog()->debugStream() << "Selections:";
    for (unsigned int i=0; i < selections.size(); i++) {
        slog()->debugStream() << "\t" << selections[i].grid() << ":" <<
        selections[i].getX() << ":" << selections[i].getY();
    }
}

void ButtonWindow::selectionModeChanged(int sel)
{
    enum ButtonWindow::SELECTIONMODE newmode = (ButtonWindow::SELECTIONMODE) sel;
    if (newmode != mode) {
        mode = newmode;
        if (selections.size()) {
            // may need to do some cleanup
            if (mode == ButtonWindow::SINGLE) {
                Selection s = selections[0];
                selectionChanged(s.getX(),s.getY(),s.grid(),true);
            }
            if (mode == ButtonWindow::SHUFFLE) {
                ///TODO Randomise the list order
            }
        }
    }
}

void ButtonWindow::loadFrame()
{
    if (selections.size()) {
        Selection s = selections[0];
        FrameSourcePtr fp = grids[s.grid()]->at(s.getX(),s.getY())->data();
        if (fp) {
            head.loadFrameSource(fp);
        }
    }
}

#if 0
void ButtonWindow::nextFrameSource()
{
    // drop the currently selected frame source
    if (selections.size()) {
        Selection s = selections[0];
        // remove from the head of the list
        grids[s.grid()]->at(s.getX(),s.getY())->setSelected(false);
        // Put it back at the back of the queue
        grids[s.grid()]->at(s.getX(),s.getY())->setSelected(true);
    }
    loadFrame();

}

#endif

