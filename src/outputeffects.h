#ifndef OUTPUTFX_INCL
#define OUTPUTFX_INCL
#include <qtoolbox.h>

#include <framesource.h>


class OutputEffectEdit : public QFrame
{
  Q_OBJECT
    OutputEffectEdit(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~OutputEffectEdit();
    
};



class OutputEffectsStack : public QToolBox
{
 Q_OBJECT
public:
  OutputEffectsStack (QObject *parent = NULL);
  virtual ~OutputEffectsStack ();
  

private:
    OutputEffectEdit effects[4];
  
  
  
  
};





#endif