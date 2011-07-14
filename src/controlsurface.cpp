#include "controlsurface.h"
#include "log.h"

ControlSurface::ControlSurface(size_t num_controls, QObject* parent): QObject(parent)
{
    // set up the controls values mapping
   controlvalues.resize(num_controls);
  for (unsigned int i=0; i < num_controls; i++){
    // the vector contains the value of the appropriate control so it can be found by getControlValue
    controlvalues[i]=0;
  }
  slog()->infoStream() << "Control surface registered "<< num_controls << " controls";
}

ControlSurface::~ControlSurface()
{
}
unsigned int ControlSurface::numberOfControls() const
{
  return controlvalues.size();
}

void ControlSurface::controlValueChanged(const unsigned int control, const int value)
{
  if (control < controlvalues.size()){
    controlvalues[control] = value;
  } else {
    slog()->errorStream() << "Control out of range " << control;
    return;
  }
  emit controlChanged (control);
}

int ControlSurface::getControlValue(const unsigned int control)
{
  if (control < controlvalues.size()){
    return controlvalues[control];
  }
  slog()->errorStream() << "Control out of range " << control;
  return -1;
}

void ControlSurface::setControlState(const unsigned int control, const unsigned int value)
{
  // first check that the appropriate control has a valid state range.
  int min,max, steps;
  unsigned int states;
  QString name;
  if (!controlParameters(control,name,min,max,steps,states)){
    slog()->errorStream() << "Attempt to set invalid control :" << control;
    return;
  }
  if (value < states){
    setControl(control, value);
  } else {
    slog()->errorStream() << "Attempt to set out of range control "<< control << " value" << value;
  }
}

bool ControlSurface::connectToSurface (unsigned int)
{
  return true;
}

void ControlSurface::disconnectFromSurface(const unsigned int)
{
  return;
}

