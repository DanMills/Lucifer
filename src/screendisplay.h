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
    //SourceImplPtr data();
public slots:
    void advance ();
    void animate (bool run, unsigned int fps);
    void loadData();
    void editData();
    void source (PlaybackPtr pb_);
signals:
    void modified(); //emitted when the underlying data is replaced
    void clicked (int pos); // emitted when playback is activated or deactivated by pressing the button
    void message (QString text, int time);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void enterEvent (QEvent *);
    void leaveEvent (QEvent *);
    void mousePressEvent (QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent (QMouseEvent *e);
private:
    PlaybackPtr pb;
    QTimer *timer;
    QMenu popup;
    //bool on;
    QPoint dragStartPosition;
    QTime dragStartTime;
    bool dragging;
    int id;
    EnginePtr engine;
};

#endif
