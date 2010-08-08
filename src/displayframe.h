/*displayframe.h is part of lucifer a laser show controller.

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

/* $Id: displayframe.h 22 2010-07-24 15:01:51Z dmills $ */

/// Display a vector frame on a QWidget

/// This draws a vector frame on a QWidget for display.

#ifndef DISPLAYFRAME_INC
#define DISPLAYFRAME_INC

#include <QtGui>
#include "frame.h"

class DisplayFrame : public QWidget
{
    Q_OBJECT
public:
    DisplayFrame (QWidget *parent = 0);
    virtual ~DisplayFrame ();
		/// Set the frame to display.
		/// @param frame a shared pointer to the frame to be displayed.
    void setFrame (ConstFramePtr frame);
		/// Set the width of the border around the displayed frame.
		/// @param width the border width in pixels.
    void setBorderWidth (unsigned int width);
		/// Set the width of the indicator band around the displayed frame.
		/// @param width the indicator width in pixels.
    void setIndicatorWidth (unsigned int width);
		/// Set the indicator colour.
		/// @param colour the colour to be used for the indicator.
    void setIndicatorColour (QColor colour);
		/// Set the border colour.
		/// @param colour the colour to be used for the border.
    void setBorderColour (QColor colour);
		/// Clear the display.
    void clear ();
protected:
    void paintEvent (QPaintEvent *e);
private:
    unsigned int bw;
    unsigned int iw;
    QColor bc;
    QColor ic;
    ConstFramePtr points;
};

#endif
