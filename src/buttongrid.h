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

/* $Id: buttongrid.h 22 2010-07-24 15:01:51Z dmills $ */

#ifndef BUTTON_GRID_INC
#define BUTTON_GRID_INC

#include <ostream>
#include <QtGui>
#include "screendisplay.h"

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
    /// @param *parent is the parent widget.
    ButtonGrid (unsigned int num_x = 8, unsigned int num_y = 8,unsigned int id=0 ,QWidget *parent = NULL);
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
    /// Save the entire contents of the grid and its layout.

		///
		/// @param *w is a pointer to the xmlwriter used to store the data.
    void save (QXmlStreamWriter *w);
    /// A Factory method that creates a button grid from the saved XML.
		/// @returns a new ButtonGrid loaded from file
		/// @param *e is the xmlreader containing the button grid.
    /// @param *parent is the parent widget.
    static ButtonGrid *load(QXmlStreamReader* e, unsigned int id, QWidget* parent);
signals:
    /// Emitted when one of the buttons is modified.
    void modified ();
		void clicked (unsigned int x, unsigned int y, unsigned int id, bool down);
public slots:
		/// Emits clicked if state changed
		void setState (unsigned int x, unsigned int y, bool selected);
		/// does not emit clicked
		void clear(int except_x = -1, int except_y=-1);
private:
    QGridLayout * layout;
		QSignalMapper * mapper;
    unsigned int num_x_;
    unsigned int num_y_;
		unsigned int id_;
private slots:
    void mod ();
		void stateChanged (int);
//public slots:
//    void stat(int);
};

#endif
