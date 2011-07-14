#ifndef MOTORMIX_INCL
#define MOTORMIX_INCL

#include "midi.h"
#include "controlsurface.h"
#include <QtCore>

class MotorMix : public ControlSurface
{
  Q_OBJECT
public:
  MotorMix (QObject *parent = NULL);
  virtual ~MotorMix();
  bool connectController (const unsigned int midiChan);
  // Virtual methods inherited from ControlSurface
  virtual unsigned int displayCount () const {return 1;};
  virtual unsigned int displayWidth (const unsigned int) const {return 40;};
  virtual unsigned int displayHeight (const unsigned int) const {return 2;}; 
  virtual unsigned int displayWidthPerChannel (const unsigned int) const {return 5;};
  virtual unsigned int numberOfChannels () const {return 8;};
  virtual bool displayWrite (const unsigned int, const unsigned int position, const QString text);
  virtual bool setControl (unsigned int control, unsigned int val);
  virtual bool controlParameters (const unsigned int control, QString &name, int &min, int &max, int &steps, unsigned int &indicatorStates) const;
  // This handles connecting to the control surface and any required initialisation.
  virtual bool connectController (MIDIChannel *midi);
  
private slots:
  void midiControlValueChanged (int number, int value);
private:
  int controllers[2];
  int values[2];
  MIDIChannel * channel;
};
  
#endif