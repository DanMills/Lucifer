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

ScreenDisplay::ScreenDisplay(QWidget *parent) : DisplayFrame(parent)
{
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
        Ildaloader ilda;
        unsigned int err = 0;
        FrameSourcePtr fs = ilda.load(filename,err,false);
        if (fs) {
            settings.setValue ("path",QFileInfo(filename).absolutePath());
            source (fs);
        } else {
            QMessageBox::warning(this, tr("Lucifer"),
                                 tr("Unable to read ILDA file or file invalid"),
                                 QMessageBox::Ok);

        }
    }
    settings.endGroup();
}

void ScreenDisplay::editData()
{
        ParameterEditor *pe = new ParameterEditor(this);
        pe->load (fs_);
        pe->show();
}

void ScreenDisplay::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("Text/FrameSource") ||
			(event->mimeData()->hasFormat("text/uri-list")))
        event->acceptProposedAction();
}

void ScreenDisplay::dropEvent(QDropEvent *event)
{
		if (event->mimeData()->hasFormat("Text/FrameSource")){
			std::string data((char *) event->mimeData()->data("Text/FrameSource").constData());
			FrameSourcePtr f = FrameSource::fromString(data);
			if (f) {
				slog()->infoStream() << "Dropped framesource onto ScreenDisplay "<< this;
				source (f);
			}
			event->acceptProposedAction();
		} else if (event->mimeData()->hasFormat("text/uri-list")){
			QList<QUrl> urls = event->mimeData()->urls();
			if (urls.size() >0){
				QString s = urls[0].toLocalFile();
				slog()->infoStream() << "Dropped URI : " << s.toStdString() << " onto ScreenDisplay "<<this;
				Ildaloader ilda;
				unsigned int err = 0;
				FrameSourcePtr fs = ilda.load(s,err,false);
				if (fs){
					source (fs);
				}
			}
			event->acceptProposedAction();
		}
}

void ScreenDisplay::mouseReleaseEvent (QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton && !dragging) {
		// Start or stop this payback
		on = !on;
		setIndicatorColour(QColor( on ? Qt::red : Qt::black));
		emit stateChanged (on);
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
    QMimeData *mimeData = new QMimeData;
    // This is ugly, but does mean that the file save and load code is well tested
    std::string text = fs_->toString();
    // now setup the mime type to hold this data
    QByteArray arr (text.c_str(),text.size());
    mimeData->setData("Text/FrameSource", arr);
    drag->setMimeData(mimeData);
		// Execute the drag action
		dragging = true;
    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
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
