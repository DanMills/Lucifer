/*buttonwindow.h is part of lucifer a laser show controller.

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

/* $Id: buttonwindow.h 19 2010-07-03 16:53:57Z dmills $ */

#ifndef BUTTON_WINDOW_INC
#define BUTTON_WINDOW_INC

#include <QtGui>
#include "buttongrid.h"

// The source selector window
#include "driver.h"
#include "head.h"

class Selection
{
	public:
		Selection (const unsigned int g, const unsigned int x, const unsigned int y)
		{
			g_= g;
			x_ = x;
			y_ = y;
		}
		~Selection(){};
		unsigned int grid() {return g_;};
		unsigned int getX() {return x_;};
		unsigned int getY() {return y_;};
	private:
		unsigned int g_;
		unsigned int x_;
		unsigned int y_;
};


class ButtonWindow : public QMainWindow
{
    Q_OBJECT

public:
    ButtonWindow ();
public slots:
		void loadFile(const QString &fn);
		bool saveFile(const QString &fn);

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void openFile();
    bool saveFile ();
    bool saveAsFile();
    void modified(); // A child has been modified, need to re-save?
    void setFullScreen();
    void clearFullScreen();
		void importFiles ();
		void selectionChanged (unsigned int x, unsigned int y, unsigned int id, bool down);
		void selectionModeChanged (int sel);
		void nextFrameSource();
private:
    std::vector<ButtonGrid *> grids;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *setupMenu;
    QMenu *helpMenu;

    QAction * openAct;
    QAction * saveAct;
    QAction * saveAsAct;
    QAction * fullScreenAct;
    QAction * windowScreenAct;
    QAction * blankLasersAct;
    QAction * exitAct;
		QAction * importAct;
    QTabWidget * tabs;
		QComboBox * selectionMode;
		QComboBox * stepMode;

    bool unsaved;
    QString fileName;
    QString pathName;

    bool saveAs();
    void setCurrentFile(const QString &fn);
    bool maybeSave();
    void loadSettings();
    void storeSettings();
    void makeActions();
    void loadFrame();

		DriverPtr driver;
		LaserHead head;
		// The list of currently selected frame sources
		std::vector<Selection> selections;
		enum SELECTIONMODE {SINGLE,MULTIPLE,SHUFFLE};
		SELECTIONMODE mode;
};
#endif
