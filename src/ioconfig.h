/* IO port configuration dialogue */

#ifndef IO_CONF_INCL
#define IO_CONF_INCL

#include <QtGui>
#include "engine.h"


class IOConfiguration : public QDialog
{
    Q_OBJECT
public:
    IOConfiguration(EnginePtr e, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~IOConfiguration();
signals:
    // MIDI Control surfaces
    void midiPortChanged (QString name);
    void midiChannelDriverChanged (unsigned int chan, QString driverName);
    // Audio IO devices

    // Laser heads

    // Joystick

    //GPIO

    //DMX

    //Timecode
protected:
  virtual void closeEvent (QCloseEvent *event);
  
    
private:
  EnginePtr engine;
    
    
    
};



#endif
