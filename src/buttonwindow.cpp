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

ButtonWindow::ButtonWindow ()
{
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
    // This toolbar gets the output heads displayed as thumb nails on it
    for (unsigned int i=0; i <8; i++) {
        DisplayFrame *f = new DisplayFrame(this);
        f->setGeometry(0,0,96,96);
        f->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        toolbar->addWidget (f);
    }
    // Multiselect mode
    // Random
    // GPIO indicators
    // Midi indicator
    // Audio meters

    // And the SCRAM button
    toolbar->addAction(blankLasersAct);
    addToolBar(toolbar);

    tabs = new QTabWidget(this);
    tabs->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setCentralWidget(tabs);
    for (unsigned int i=0; i < 24; i++) {
        grids.push_back (new ButtonGrid(8,8,this));
        tabs->addTab(grids[i],QString("&")+QString().number(i+1));
        connect (grids[i],SIGNAL(modified()), this, SLOT(modified()));
    }
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
		QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int i=0; i < l.size(); i++) {
        QString s = l[i];
        Ildaloader ilda;
        unsigned int error = 0;
        FrameSourcePtr p = ilda.load (s,error);
        // Scan the grid arrays looking for somewhere to put this thing.
        for (unsigned int g=0; g < grids.size(); g++) {
            int gx, gy;
            grids[g]->dimensions (gx,gy);
            for (int x =0; x < gx; x++) {
                for (int y = 0; y < gy; y++) {
                    if (!grids[g]->at(x,y)->data()) {
                        // Got an empty one
                        grids[g]->at(x,y)->source (p);
                        p = FrameSourcePtr();
                        break;
                    }
                }
            }
        }
    }
    QApplication::restoreOverrideCursor();
		modified();
}

bool ButtonWindow::saveFile(const QString &fn)
{
    QFile f(fn);
    if (!f.open(QFile::WriteOnly)) {
        slog()->errorStream()<<"Unable to open file for writing";
        QMessageBox::warning(this, tr("Lucifer"),
                             tr("The file could not be opened."),
                             QMessageBox::Ok);
        QApplication::restoreOverrideCursor();
        statusBar()->showMessage(tr("File save error"), 5000);
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    slog()->infoStream()<<"Saving file : " << fn.toStdString();
    QXmlStreamWriter *w = new QXmlStreamWriter(&f);
    w->setAutoFormatting(true);
    w->writeStartDocument();
    w->writeStartElement("Lucifer");
    w->writeAttribute("Version","1.0.0");
    for (unsigned int i=0; i < grids.size(); i++) {
        grids[i]->save(w);
    }
    w->writeEndElement();// Lucifer show
    w->writeEndDocument();
    delete w;
    f.close();
    QApplication::restoreOverrideCursor();
    slog()->infoStream()<<"File saved";
    setCurrentFile(fn);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
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
    QApplication::setOverrideCursor(Qt::WaitCursor);
    slog()->infoStream()<<"Loading show file : " << fn.toStdString();
    QFile f(fn);
    if (!f.open(QFile::ReadOnly)) {
        slog()->errorStream()<<"Error opening file";
        QMessageBox::warning(this, tr("Lucifer"),
                             tr("The file could not be opened."),
                             QMessageBox::Ok);
        QApplication::restoreOverrideCursor();
        statusBar()->showMessage(tr("File Not Loaded"), 5000);
        return;
    }
    QXmlStreamReader *r = new QXmlStreamReader(&f);
    while (!r->atEnd()) {
        // Find the document head and check that this is the correct file format
        if (r->name().toString() == "Lucifer") {
            slog()->debugStream() << "Found a Lucifer header";
            break;
        }
        slog()->debugStream() << "Element name : " << r->name().toString().toStdString();
        r->readNext();
    }
    if ((!r->hasError ()) && (r->attributes().value("Version") == "1.0.0")) {
        // Ok, a V1.00 Lucifer file has been found
        // Next up sit in a loop looking for "Grid" elements and creating the grids
        slog()->debugStream() << "File is version 1.0.0";
        int count = 0;
        grids.clear();
        tabs->clear();
        while (!r->atEnd()) {
            if (r->isEndElement() && (r->name().toString() == "Lucifer")) {
                break;
            }
            if (r->isStartElement() && (r->name() == "Grid")) {
                slog()->debugStream() << "Loading grid : " << count +1;
                ButtonGrid *g = ButtonGrid::load(r,this);
                grids.push_back(g);
                tabs->addTab (g,QString("&")+QString().number (++count));
            }
            r->readNextStartElement();
        }
        tabs->setCurrentIndex(0);
    }
    if (r->hasError()) {
        slog()->errorStream() << "File read error :" << r->errorString().toStdString();
        QMessageBox::warning(this, tr("Lucifer"),
                             r->errorString(),
                             QMessageBox::Ok);
    }
    delete r;
    f.close();
    QApplication::restoreOverrideCursor();
    setCurrentFile(fn);
    statusBar()->showMessage(tr("File Loaded"), 2000);
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

