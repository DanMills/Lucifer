/*screendisplay.cpp is part of lucifer a laser show controller.

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

/* $Id: screendisplay.cpp 14 2010-06-26 20:07:45Z dmills $ */

// Plays back a framesource on a DisplayFrame
// Also handles drag and drop

#include <QtGui>
#include <math.h>
#include <vector>
#include <iostream>

#include "screendisplay.h"
#include "loadilda.h"
#include "editparams.h"
#include "log.h"

ScreenDisplay::ScreenDisplay (EnginePtr engine_, int id_, QWidget * parent) : DisplayFrame(parent)
{
    id = id_;
    engine = engine_;
    playback_ = -1;
    on = false;
    timer = new QTimer (this);
    connect(timer, SIGNAL(timeout()),this, SLOT (advance()));
    popup.addAction (tr("Import"),this,SLOT(loadData()));
    popup.addAction (tr("Edit"),this,SLOT(editData()));
    setIndicatorColour(QColor(Qt::black));
    setBorderColour(QColor(Qt::black));
    setBorderWidth(4);
    setIndicatorWidth(1);
    dragging = false;
    setAcceptDrops(true);

};

void ScreenDisplay::animate (bool run, unsigned int fps)
{
    if (fs_) {
        if (run) {
            fs_->reset (playback_);
            timer->start (1000.0/fps);
        } else {
            timer->stop();
        }
    }
}

void ScreenDisplay::loadData()
{
    assert (engine);

    // Called to Import an ILDA graphics file
    // This remembers the last path used
    QSettings settings;
    settings.beginGroup ("ILDA Importer");
    QString home;
    if (getenv ("HOME")) {
        home = getenv ("HOME");
    } else {
        home = "/";
    }
    QString path = settings.value("path",home).toString();
    QString filename = QFileDialog::getOpenFileName(this,
                       tr("Open File"), path, tr("ILDA Files (*.ild *.ilda)"));
    if (!filename.isEmpty()) {
        QStringList names;
        names.append(filename);
        engine->importShow(names, id);
    } else {
        QMessageBox::warning(this, tr("Lucifer"),
                             tr("Unable to read ILDA file or file invalid"),
                             QMessageBox::Ok);

    }
    settings.endGroup();
}

bool ScreenDisplay::selected()
{
    return on;
}

void ScreenDisplay::editData()
{
    ParameterEditor *pe = new ParameterEditor(this);
    pe->load (fs_);
    pe->show();
}

void ScreenDisplay::dragEnterEvent(QDragEnterEvent *event)
{
    std::vector <std::string> valid_types;
    valid_types = engine->mimeTypes();
    for (unsigned int i=0; i < valid_types.size(); i++) {
        if (event->mimeData()->hasFormat(QString().fromStdString(valid_types[i]))) {
            event->acceptProposedAction();
            break;
        }
    }
}

void ScreenDisplay::dropEvent(QDropEvent *event)
{
    std::vector <std::string> valid_types;
    valid_types = engine->mimeTypes();
    for (unsigned int i=0; i < valid_types.size(); i++) {
        if (event->mimeData()->hasFormat(QString().fromStdString(valid_types[i]))) {
            if (engine->mimeHandler(event->mimeData(),id)) {
                event->acceptProposedAction();
                break;
            }
        }
    }
};


void ScreenDisplay::setSelected(bool sel)
{
    bool e = (sel != on);
    resetSelected(sel);
    if (e) {
        emit stateChanged (on);
    }
}

void ScreenDisplay::resetSelected(bool sel)
{
    bool e = (on != sel);
    on = sel;
    if (e) {
        setIndicatorColour(QColor( on ? Qt::red : Qt::black));
    }
}


void ScreenDisplay::mouseReleaseEvent (QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && !dragging) {
        // Start or stop this payback
        setSelected(!on);
        dragging = false;
    }
}

void ScreenDisplay::mousePressEvent (QMouseEvent * e)
{
    //Handle right click (popup menu) pass everything else to the underlying system
    if (e->button() == Qt::RightButton) {
        popup.popup(mapToGlobal (e->pos()));
        e->accept();
    } else if (e->button()==Qt::LeftButton) {
        dragStartPosition = e->pos();
        dragStartTime.start();
        e->accept();
    }
}

void ScreenDisplay::enterEvent (QEvent *e)
{
    // We run the preview animation when mouse is hovered over the picture
    if (e->type() == QEvent::Enter) {
        animate (true,15);
        e->accept();
        dragging =false;
    } else {
        QWidget::enterEvent (e);
    }
}

void ScreenDisplay::leaveEvent (QEvent *e)
{
    if (e->type()==QEvent::Leave) {
        animate (false,5);
        e->accept();
        dragging = false;
    } else {
        QWidget::leaveEvent (e);
    }
}

void ScreenDisplay::mouseMoveEvent(QMouseEvent *event)
{
    if (!fs_) {
        return;
    }
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if (((event->pos() - dragStartPosition).manhattanLength()
            < QApplication::startDragDistance()) ||
            (dragStartTime.elapsed() < QApplication::startDragTime())) {
        return;
    }
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = engine->mimeData(id);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap().grabWidget(this,
                                         QRect(4,4,this->size().width()-8, this->size().height()-8))
                    .scaled (48,48,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    // Execute the drag action
    dragging = true;
    /*Qt::DropAction dropAction = */drag->exec(Qt::CopyAction | Qt::MoveAction);
}


ScreenDisplay::~ScreenDisplay ()
{
    // Delete the old playback source handle if one existed
    // Note that we dont own the Framesource, just the playback
    // within that framesource
    if (fs_) {
        fs_->deletePlayback (playback_);
        fs_ = FrameSourcePtr();
    }
}

FrameSourcePtr ScreenDisplay::data()
{
    return fs_;
}

void ScreenDisplay::source (FrameSourcePtr fs)
{
    // Delete the old playback source handle if one existed
    if (fs_) {
        fs_->deletePlayback (playback_);
    }
    fs_ = fs;
    emit modified();
    if (!fs) {
        return;
    }
    playback_ = fs_->createPlayback ();
    setFrame(fs_->data (playback_));
}

void ScreenDisplay::advance ()
{
    FramePtr pts = fs_->data (playback_);
    if (!pts) {
        // End of sequence so reset
        fs_->reset (playback_);
        pts = fs_->data (playback_);
        // Todo - emit some kind of start of sequence signal
    }
    setFrame (pts);
}
