/* outputview.h is part of lucifer a laser show controller.

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

#ifndef OUTPUT_VIEW_INCL
#define OUTPUT_VIEW_INCL

#include "displayframe.h"
#include "frame.h"

class OutputView : public DisplayFrame
{
    Q_OBJECT
public:
    OutputView (bool independent_window, QWidget *parent = NULL);
    ~OutputView ();
    void setTitle (QString title);
public slots:
    void updateDisplay (FramePtr f);
signals:
    void displayUpdated (FramePtr f);
    void leftClicked();
    void rightClicked();
private slots:
    //    void setup ();
    void leftClickedData ();
    void rightClickedData ();

protected:
    void mouseReleaseEvent (QMouseEvent *e);
private:
    FramePtr of;
    bool independent;
    QString name;
};

#endif
