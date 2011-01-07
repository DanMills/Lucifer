/*buttongrid.h is part of lucifer a laser show controller.

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

#ifndef BUTTON_GRID_INC
#define BUTTON_GRID_INC

#include <ostream>
#include <QtGui>
#include "screendisplay.h"
#include "engine.h"

/// A grid of ScreenDiplays displayed on the screen.

/// This diaplays a grid of ScreenDisplay objects inside whatever parent QWidget is used.
/// Various slection modes are supported

class ButtonGrid : public QWidget
{
    Q_OBJECT
public:
		/// Construct an empty ButtonGrid

		///
    /// @param num_x is the number of buttons to create across the screen.
    /// @param num_y is the number of rows of buttons to create.
		/// @param offset is the engine framesource index of the upper left corner
    /// @param *parent is the parent widget.
    ButtonGrid (EnginePtr engine_, unsigned int num_x = 8, unsigned int num_y = 8,unsigned int offset=0 ,QWidget *parent = NULL);
    ~ButtonGrid();
    /// @returns Returns a pointer to the ScreenDisplay corresponding to the given coordinates in the grid.

		///
		/// @param x the (zero based) X coordinate of the ScreenDisplay to be returned.
    /// @param y the (zero based) y coordinate of the ScreenDisplay to be returned.
    ScreenDisplay * at (unsigned int x, unsigned int y);
    /// Get the dimensions of the grid.

		///
		/// @param &x reference to variable to contain the x value.
    /// @param &y reference to variable to contain the y value.
    void dimensions(int &x, int &y);

signals:
    /// Emitted when one of the buttons is modified.
    void modified ();
public slots:
		/// This gets called by the engine whenever a framesource has been replaced by a new one
		/// Used to update the displayed grid
		void frameSourceChanged(unsigned long int pos);
		/// called when the screen display menu item import is selected
		void message (QString text, int time);

private:
    QGridLayout * layout;
		QSignalMapper * mapper;
		QStatusBar *statusbar;
		QLabel * ppsLabel;
		QLabel * framesLabel;
    unsigned int num_x_;
    unsigned int num_y_;
		unsigned int offset_;
		EnginePtr engine;
private slots:
    //void mod ();
		void selectionChangedData(unsigned int pos, QColor col);
		//void stateChanged (int);
};

#endif
