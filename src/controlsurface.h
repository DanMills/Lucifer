/* controlsurface.h is part of lucifer a laser show controller.

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

#ifndef CONTROLSURFACE_INCL
#define CONTROLSURFACE_INCL

#include <QtCore>
#include "midi.h"

class Engine;

/// Virtual base class for control surfaces - Midi, USB, things of that ilk
class ControlSurface : public QObject
{
  Q_OBJECT
public:
  /// num_controls is the number of controls  the derived class provides
  ControlSurface (size_t num_controls, QObject* parent = 0);
  virtual ~ControlSurface();
  /// the number of LCD screens on the control surface
  virtual unsigned int displayCount () const = 0;
  /// For each screen this rreturns the width in characters
  virtual unsigned int displayWidth (const unsigned int display) const = 0;
  /// For each display this returns the height in characters
  virtual unsigned int displayHeight(const unsigned int display) const = 0;
  /// For each display this returns the number of characters related to each channel
  virtual unsigned int displayWidthPerChannel (const unsigned int display) const = 0;
  /// Write text to the specified display.
  virtual bool displayWrite (const unsigned int display, const unsigned int position, const QString text) = 0;
  /// Get the number of channels on the surface
  virtual unsigned int numberOfChannels () const = 0;
  unsigned int numberOfControls () const;
  virtual bool setControl (unsigned int control, unsigned int val) = 0;
  // Get the parameters associated with a control.
  virtual bool controlParameters (const unsigned int control, QString &name, int &min, int &max, int &steps, unsigned int &indicatorStates) const = 0;
  // This handles connecting to the control surface and any required initialisation.
  virtual bool connectMidi (MIDIChannel *midi);
  virtual void disconnectFromSurface (const unsigned int channel);
  // Connect the engine so signals can be hooked up.
  virtual void connectEngine (Engine * e) = 0;
  // get a control value 
  int getControlValue (const unsigned int control);
protected slots:
  void controlValueChanged (const unsigned int control, const int value);
public slots:
  void setControlState (const unsigned int control, const unsigned int value);
signals:
  void surfaceCconnected ();
  void surfaceDisconnected ();
  void controlChanged (int control);
protected:
  std::vector <int> controlvalues;
private:
  ControlSurface ();
};

#endif