/*screendisplay.h is part of lucifer a laser show controller.

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

/* $Id: screendisplay.h 14 2010-06-26 20:07:45Z dmills $ */

#ifndef SCREEN_DISP_INC
#define SCREEN_DISP_INC

#include <QtGui>
#include <QObject>
#include <QPainter>
#include <vector>

#include "framesource.h"
#include "displayframe.h"
#include "point.h"
#include "engine.h"

class ScreenDisplay : public DisplayFrame
{
    Q_OBJECT
public:
    ScreenDisplay (EnginePtr engine_, int id_, QWidget * parent = 0);
    ~ScreenDisplay ();
    FrameSourcePtr data();
		bool selected();
public slots:
    void advance ();
    void animate (bool run, unsigned int fps);
    void loadData();
		void editData();
		void source (FrameSourcePtr fs);
		void setSelected (bool sel);
		void resetSelected (bool sel);
	signals:
    void modified(); //emitted when the underlying data is replaced
		/// Emitted when a frame source is drag and dropped onto this displayframe
		/// Should be hooked to eventually call engine.addFrameSource with
		///apropriate parameters.
		//void addFrameSource (FrameSourcePtr p);
    void stateChanged (bool); // emitted when playback is activated or deactivated by pressing the button
		//void importClicked();
		//void dropped(const QMimeData * data);

protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);
    void enterEvent (QEvent *);
    void leaveEvent (QEvent *);
    void mousePressEvent (QMouseEvent * e);
		void mouseMoveEvent(QMouseEvent *e);
		void mouseReleaseEvent (QMouseEvent *e);
private:
    Playback playback_;
    FrameSourcePtr fs_;
    QTimer *timer;
    QMenu popup;
    bool on;
		QPoint dragStartPosition;
		QTime dragStartTime;
		bool dragging;
		int id;
		EnginePtr engine;
};

#endif
