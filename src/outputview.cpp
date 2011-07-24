/* outputview.cpp is part of lucifer a laser show controller.

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


#include "outputview.h"
#include "log.h"

OutputView::OutputView(bool independent_window, QWidget* parent): DisplayFrame(parent), independent(independent_window)
{
    setIndicatorColour(QColor(Qt::black));
    setBorderColour(QColor(Qt::black));
    setBorderWidth(4);
    setIndicatorWidth(1);
    connect (this,SIGNAL(rightClicked()),this,SLOT(rightClickedData()));
    if (independent_window) { 
	// make this object auto delete on close
        setAttribute(Qt::WA_DeleteOnClose,true);
	// We dont want compositors to try to make this semi transparent
	//setAttribute(Qt::WA_PaintOnScreen,true);
      
    }
}

OutputView::~OutputView()
{
}

void OutputView::setTitle(QString title)
{
    name = title;
    setWindowTitle(name);
    setWindowIconText(name);
    // now we know the name, restore the geometry
    if (independent) {
        QSettings settings;
        settings.beginGroup(title);
        if (settings.contains("Geometry")) {
            restoreGeometry(settings.value("Geometry").toByteArray());
        }
        settings.endGroup();
    }
}



void OutputView::updateDisplay(FramePtr f)
{
    if (f != of) {
        of = f;
        setFrame(f);
    }
    emit displayUpdated (f);
}

void OutputView::leftClickedData()
{

}


void OutputView::rightClickedData()
{
    slog()->debugStream() <<"OutputView::clickedData()";
    if (!independent) {
        OutputView * w = new OutputView (true);
        slog() -> infoStream() <<"Creating new output view window : " << w;
        w->setTitle(name);
        connect (this,SIGNAL(displayUpdated(FramePtr)),w,SLOT(updateDisplay(FramePtr)));
        w->show();
    }
}

void OutputView::mouseReleaseEvent (QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        //Button clicked
        emit leftClicked();
    } else if (e->button() == Qt::RightButton) {
        emit rightClicked();
    }
}

void OutputView::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.beginGroup(name);
    settings.setValue("Geometry",saveGeometry());
    settings.endGroup();
    QWidget::closeEvent(event);
}


