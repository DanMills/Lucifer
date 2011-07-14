#ifndef CONTROLSURFACE_INCL
#define CONTROLSURFACE_INCL

#include <QtCore>

// Virtual base class for control surfaces - Midi, USB, things of that ilk
class ControlSurface : public QObject
{
  Q_OBJECT
public:
  ControlSurface (size_t num_controls, QObject* parent = 0);
  virtual ~ControlSurface();
  // Scribble strip parameters
  virtual unsigned int displayCount () const = 0;
  virtual unsigned int displayWidth (const unsigned int display) const = 0;
  virtual unsigned int displayHeight(const unsigned int display) const = 0;
  virtual unsigned int displayWidthPerChannel (const unsigned int display) const = 0;
  virtual bool displayWrite (const unsigned int display, const unsigned int position, const QString text) = 0;
  // controls 
  virtual unsigned int numberOfChannels () const = 0;
  unsigned int numberOfControls () const;
  virtual bool setControl (unsigned int control, unsigned int val) = 0;
  // Get the parameters associated with a control.
  virtual bool controlParameters (const unsigned int control, QString &name, int &min, int &max, int &steps, unsigned int &indicatorStates) const = 0;
  // This handles connecting to the control surface and any required initialisation.
  virtual bool connectToSurface (const unsigned int channel);
  virtual void disconnectFromSurface (const unsigned int channel);
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