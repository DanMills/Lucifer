#include "motormix.h"
#include "engine.h"
#include "log.h"

struct control
{
    char ctrl1_number;
    char cntrl1_value;
    char cntrl2_number;
    char cntrl2_pressed;
    char cntrl2_released;
    char led_ctl1_number;
    char led_ctl1_value;
    char led_ctl2_number;
    char led_ctl2_off;
    char led_ctl2_on;
    char led_ctl2_blink;
    const char *name;
    int min;
    int max;
    int steps;
    int indicatorStates;
};

static const struct control switches[] = {
    // Key scan code details	Key LED switching codes		Key details
    {0x0f,0x00,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [0]",0,1,2,0},
    {0x0f,0x01,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [1]",0,1,2,0},
    {0x0f,0x02,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [2]",0,1,2,0},
    {0x0f,0x03,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [3]",0,1,2,0},
    {0x0f,0x04,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [4]",0,1,2,0},
    {0x0f,0x05,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [5]",0,1,2,0},
    {0x0f,0x06,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [6]",0,1,2,0},
    {0x0f,0x07,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Fader Touch [7]",0,1,2,0},
    // Select switches next
    {0x0f,0x00,0x2f,0x41,0x01, 	0x0c,0x00,0x2c,0x01,0x41,0x51, 	"Select [0]",0,1,2,3},
    {0x0f,0x01,0x2f,0x41,0x01, 	0x0c,0x01,0x2c,0x01,0x41,0x51, 	"Select [1]",0,1,2,3},
    {0x0f,0x02,0x2f,0x41,0x01, 	0x0c,0x02,0x2c,0x01,0x41,0x51, 	"Select [2]",0,1,2,3},
    {0x0f,0x03,0x2f,0x41,0x01, 	0x0c,0x03,0x2c,0x01,0x41,0x51, 	"Select [3]",0,1,2,3},
    {0x0f,0x04,0x2f,0x41,0x01, 	0x0c,0x04,0x2c,0x01,0x41,0x51, 	"Select [4]",0,1,2,3},
    {0x0f,0x05,0x2f,0x41,0x01, 	0x0c,0x05,0x2c,0x01,0x41,0x51, 	"Select [5]",0,1,2,3},
    {0x0f,0x06,0x2f,0x41,0x01, 	0x0c,0x06,0x2c,0x01,0x41,0x51, 	"Select [6]",0,1,2,3},
    {0x0f,0x07,0x2f,0x41,0x01, 	0x0c,0x07,0x2c,0x01,0x41,0x51, 	"Select [7]",0,1,2,3},
    // Mute switches
    {0x0f,0x00,0x2f,0x42,0x02, 	0x0c,0x00,0x2c,0x02,0x42,0x52, 	"Mute [0]",0,1,2,3},
    {0x0f,0x01,0x2f,0x42,0x02, 	0x0c,0x01,0x2c,0x02,0x42,0x52, 	"Mute [1]",0,1,2,3},
    {0x0f,0x02,0x2f,0x42,0x02, 	0x0c,0x02,0x2c,0x02,0x42,0x52, 	"Mute [2]",0,1,2,3},
    {0x0f,0x03,0x2f,0x42,0x02, 	0x0c,0x03,0x2c,0x02,0x42,0x52, 	"Mute [3]",0,1,2,3},
    {0x0f,0x04,0x2f,0x42,0x02, 	0x0c,0x04,0x2c,0x02,0x42,0x52, 	"Mute [4]",0,1,2,3},
    {0x0f,0x05,0x2f,0x42,0x02, 	0x0c,0x05,0x2c,0x02,0x42,0x52, 	"Mute [5]",0,1,2,3},
    {0x0f,0x06,0x2f,0x42,0x02, 	0x0c,0x06,0x2c,0x02,0x42,0x52, 	"Mute [6]",0,1,2,3},
    {0x0f,0x07,0x2f,0x42,0x02, 	0x0c,0x07,0x2c,0x02,0x42,0x52, 	"Mute [7]",0,1,2,3},
    // Solo switches
    {0x0f,0x00,0x2f,0x43,0x03, 	0x0c,0x00,0x2c,0x03,0x43,0x53, 	"Solo [0]",0,1,2,3},
    {0x0f,0x01,0x2f,0x43,0x03, 	0x0c,0x01,0x2c,0x03,0x43,0x53, 	"Solo [1]",0,1,2,3},
    {0x0f,0x02,0x2f,0x43,0x03, 	0x0c,0x02,0x2c,0x03,0x43,0x53, 	"Solo [2]",0,1,2,3},
    {0x0f,0x03,0x2f,0x43,0x03, 	0x0c,0x03,0x2c,0x03,0x43,0x53, 	"Solo [3]",0,1,2,3},
    {0x0f,0x04,0x2f,0x43,0x03, 	0x0c,0x04,0x2c,0x03,0x43,0x53, 	"Solo [4]",0,1,2,3},
    {0x0f,0x05,0x2f,0x43,0x03, 	0x0c,0x05,0x2c,0x03,0x43,0x53, 	"Solo [5]",0,1,2,3},
    {0x0f,0x06,0x2f,0x43,0x03, 	0x0c,0x06,0x2c,0x03,0x43,0x53, 	"Solo [6]",0,1,2,3},
    {0x0f,0x07,0x2f,0x43,0x03, 	0x0c,0x07,0x2c,0x03,0x43,0x53, 	"Solo [7]",0,1,2,3},
    // Multi switches
    {0x0f,0x00,0x2f,0x44,0x04, 	0x0c,0x00,0x2c,0x04,0x44,0x54, 	"Multi [0]",0,1,2,3},
    {0x0f,0x01,0x2f,0x44,0x04, 	0x0c,0x01,0x2c,0x04,0x44,0x54, 	"Multi [1]",0,1,2,3},
    {0x0f,0x02,0x2f,0x44,0x04, 	0x0c,0x02,0x2c,0x04,0x44,0x54, 	"Multi [2]",0,1,2,3},
    {0x0f,0x03,0x2f,0x44,0x04, 	0x0c,0x03,0x2c,0x04,0x44,0x54, 	"Multi [3]",0,1,2,3},
    {0x0f,0x04,0x2f,0x44,0x04, 	0x0c,0x04,0x2c,0x04,0x44,0x54, 	"Multi [4]",0,1,2,3},
    {0x0f,0x05,0x2f,0x44,0x04, 	0x0c,0x05,0x2c,0x04,0x44,0x54, 	"Multi [5]",0,1,2,3},
    {0x0f,0x06,0x2f,0x44,0x04, 	0x0c,0x06,0x2c,0x04,0x44,0x54, 	"Multi [6]",0,1,2,3},
    {0x0f,0x07,0x2f,0x44,0x04, 	0x0c,0x07,0x2c,0x04,0x44,0x54, 	"Multi [7]",0,1,2,3},
    // Rdy switches
    {0x0f,0x00,0x2f,0x45,0x05, 	0x0c,0x00,0x2c,0x05,0x45,0x55, 	"Rdy [0]",0,1,2,3},
    {0x0f,0x01,0x2f,0x45,0x05, 	0x0c,0x01,0x2c,0x05,0x45,0x55, 	"Rdy [1]",0,1,2,3},
    {0x0f,0x02,0x2f,0x45,0x05, 	0x0c,0x02,0x2c,0x05,0x45,0x55, 	"Rdy [2]",0,1,2,3},
    {0x0f,0x03,0x2f,0x45,0x05, 	0x0c,0x03,0x2c,0x05,0x45,0x55, 	"Rdy [3]",0,1,2,3},
    {0x0f,0x04,0x2f,0x45,0x05, 	0x0c,0x04,0x2c,0x05,0x45,0x55, 	"Rdy [4]",0,1,2,3},
    {0x0f,0x05,0x2f,0x45,0x05, 	0x0c,0x05,0x2c,0x05,0x45,0x55, 	"Rdy [5]",0,1,2,3},
    {0x0f,0x06,0x2f,0x45,0x05, 	0x0c,0x06,0x2c,0x05,0x45,0x55, 	"Rdy [6]",0,1,2,3},
    {0x0f,0x07,0x2f,0x45,0x05, 	0x0c,0x07,0x2c,0x05,0x45,0x55, 	"Rdy [7]",0,1,2,3},
    // Left side switches
    {0x0f,0x08,0x2f,0x40,0x00, 	0x0c,0x08,0x2c,0x00,0x40,0x50, 	"Shift",0,1,2,3},
    {0x0f,0x08,0x2f,0x41,0x01, 	0x0c,0x08,0x2c,0x01,0x41,0x51, 	"Undo",0,1,2,3},
    {0x0f,0x08,0x2f,0x42,0x02, 	0x0c,0x08,0x2c,0x02,0x42,0x52, 	"Default",0,1,2,3},
    {0x0f,0x08,0x2f,0x43,0x03, 	0x0c,0x08,0x2c,0x03,0x43,0x53, 	"All",0,1,2,3},
    {0x0f,0x08,0x2f,0x44,0x04, 	0x0c,0x08,0x2c,0x04,0x44,0x54, 	"Window",0,1,2,3},
    {0x0f,0x08,0x2f,0x45,0x05, 	0x0c,0x08,0x2c,0x05,0x45,0x55, 	"Plugin",0,1,2,3},
    {0x0f,0x08,0x2f,0x46,0x06, 	0x0c,0x08,0x2c,0x06,0x46,0x56, 	"Suspend",0,1,2,3},
    {0x0f,0x08,0x2f,0x47,0x07, 	0x0c,0x08,0x2c,0x07,0x47,0x57, 	"Auto Enable",0,1,2,3},

    // Right side switches
    {0x0f,0x09,0x2f,0x40,0x00, 	0x0c,0x09,0x2c,0x00,0x40,0x50, 	"Escape",0,1,2,3},
    {0x0f,0x09,0x2f,0x41,0x01, 	0x0c,0x09,0x2c,0x01,0x41,0x51, 	"Enter",0,1,2,3},
    {0x0f,0x09,0x2f,0x42,0x02, 	0x0c,0x09,0x2c,0x02,0x42,0x52, 	"Last",0,1,2,3},
    {0x0f,0x09,0x2f,0x43,0x03, 	0x0c,0x09,0x2c,0x03,0x43,0x53, 	"Next",0,1,2,3},
    {0x0f,0x09,0x2f,0x44,0x04, 	0x0c,0x09,0x2c,0x04,0x44,0x54, 	"Rewind",0,1,2,3},
    {0x0f,0x09,0x2f,0x45,0x05, 	0x0c,0x09,0x2c,0x05,0x45,0x55, 	"FFwd",0,1,2,3},
    {0x0f,0x09,0x2f,0x46,0x06, 	0x0c,0x09,0x2c,0x06,0x46,0x56, 	"Stop",0,1,2,3},
    {0x0f,0x09,0x2f,0x47,0x07, 	0x0c,0x09,0x2c,0x07,0x47,0x57, 	"Play",0,1,2,3},

    // Arrows and misc
    {0x0f,0x0A,0x2f,0x40,0x00, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Left Arrow",0,1,2,0},
    {0x0f,0x0A,0x2f,0x41,0x01, 	0x00,0x00,0x00,0x00,0x00,0x00, 	"Right Arrow",0,1,2,0},
    {0x0f,0x0A,0x2f,0x42,0x02, 	0x0c,0x0A,0x2c,0x00,0x40,0x50, 	"Bank",0,1,2,3},
    {0x0f,0x0A,0x2f,0x43,0x03, 	0x0c,0x0A,0x2c,0x01,0x41,0x51, 	"Group",0,1,2,3},
    {0x0f,0x0A,0x2f,0x44,0x04, 	0x0c,0x0A,0x2c,0x02,0x42,0x52, 	"Record Rdy",0,1,2,3},
    {0x0f,0x0A,0x2f,0x45,0x05, 	0x0c,0x0A,0x2c,0x04,0x44,0x54, 	"Write",0,1,2,3},
    {0x0f,0x0A,0x2f,0x46,0x06, 	0x0c,0x0A,0x2c,0x06,0x46,0x56, 	"Burn",0,1,2,3},

    // A few LEDs out on their lonesome
    {0x00,0x00,0x00,0x00,0x00, 	0x0c,0x0A,0x2c,0x03,0x43,0x53, 	"Funct A",0,0,0,3},
    {0x00,0x00,0x00,0x00,0x00, 	0x0c,0x0A,0x2c,0x05,0x45,0x55, 	"Funct B",0,0,0,3},
    {0x00,0x00,0x00,0x00,0x00, 	0x0c,0x0A,0x2c,0x07,0x47,0x57, 	"Funct C",0,0,0,3},
    {0x00,0x00,0x00,0x00,0x00, 	0x0c,0x0B,0x2c,0x01,0x41,0x51, 	"Effect 1",0,0,0,3},
    {0x00,0x00,0x00,0x00,0x00, 	0x0c,0x0B,0x2c,0x03,0x43,0x53, 	"Effect 2",0,0,0,3},
    {0x00,0x00,0x00,0x00,0x00, 	0x0c,0x0B,0x2c,0x05,0x45,0x55, 	"Effect 3",0,0,0,3},
    {0x00,0x00,0x00,0x00,0x00, 	0x0c,0x0B,0x2c,0x07,0x47,0x57, 	"Effect 4",0,0,0,3},

    // The last of the right hand controls
    {0x0f,0x0B,0x2f,0x40,0x00, 	0x0c,0x0B,0x2c,0x00,0x40,0x50, 	"Fx-Bypass",0,1,2,3},
    {0x0f,0x0B,0x2f,0x41,0x01, 	0x0c,0x0B,0x2c,0x02,0x42,0x52, 	"Send Mute",0,1,2,3},
    {0x0f,0x0B,0x2f,0x42,0x02, 	0x0c,0x0B,0x2c,0x04,0x44,0x54, 	"PrePost",0,1,2,3},
    {0x0f,0x0B,0x2f,0x43,0x03, 	0x0c,0x0B,0x2c,0x06,0x46,0x56, 	"Select",0,1,2,3},

};
MotorMix::MotorMix(QObject* parent): ControlSurface(sizeof(switches)/sizeof(switches[0]),parent)
{
    controllers [0] = controllers [1] = 0;
    controlvalues [0] = controlvalues[1] = 0;
    engine = NULL;
#if 1
    headControls[0] = controlByName ("Fx-Bypass");
    headControls[1] = controlByName ("Send Mute");
    headControls[2] = controlByName ("PrePost");
    headControls[3] = controlByName ("Select");
#else
    headControls[0] = controlByName ("Select [0]");
    headControls[1] = controlByName ("Select [1]");
    headControls[2] = controlByName ("Select [2]");
    headControls[3] = controlByName ("Select [3]");
    headControls[4] = controlByName ("Select [4]");
    headControls[5] = controlByName ("Select [5]");
    headControls[6] = controlByName ("Select [6]");
    headControls[7] = controlByName ("Select [7]");
#endif



    messageTimer = new QTimer (this);
    connect(messageTimer,SIGNAL(timeout()),this,SLOT(messageTimeout()));
}

MotorMix::~MotorMix()
{
}

int MotorMix::controlByName(QString name)
{
    for (unsigned int i=0; i <(sizeof(switches)/sizeof(switches[0])); i++) {
        QString n;
        int min,max,steps;
        unsigned int indicator;
        controlParameters(i,n,min,max,steps,indicator);
        if (name == n) {
            return i;
        }
    }
    return -1;
}

bool MotorMix::controlParameters(const unsigned int control, QString& name, int& min, int& max, int& steps, unsigned int& indicatorStates) const
{
    if (control < numberOfControls()) {
        name = QString(switches[control].name);
        min = switches[control].min;
        max = switches[control].max;
        steps = switches[control].steps;
        indicatorStates = switches[control].indicatorStates;
        return true;
    } else {
        slog()->errorStream() << "Attempt to access invalid control : " << control;
    }
    return false;
}

void MotorMix::connectEngine(Engine* e)
{
    engine = e;
    // The engine emits this to signal a change to the head selection and thus a change to the status LEDs.
    connect(engine,SIGNAL(headSelectionChanged(int)),this,SLOT(headSelected(int)));
    // We emit this to signal a head selection button being pressed.
    connect (this,SIGNAL(headSelectionChanged(int)),engine,SLOT(selectHead(int)));
    // Messages from the engine go on the display
    connect (engine,SIGNAL(message(QString,int)),this,SLOT(message(QString,int)));
    // Hook the midi control changed signal to our handler
    connect (this,SIGNAL(controlChanged(int)),this,SLOT(controlChangedData(int)));

    QSignalMapper *activeMap = new QSignalMapper (this);
    connect (activeMap, SIGNAL(mapped(int)),this,SLOT(headActive(int)));
    QSignalMapper *inactiveMap = new QSignalMapper (this);
    connect (inactiveMap,SIGNAL(mapped(int)),this, SLOT(headInactive(int)));
    for (int i=0; i < MAX_HEADS; i++) {
        // head activity indicators
        connect (&(*engine->getHead(i)),SIGNAL(headActive()), activeMap, SLOT(map()));
        activeMap->setMapping (&(*engine->getHead(i)),i);
        connect (&(*engine->getHead(i)),SIGNAL(headInactive()), inactiveMap, SLOT(map()));
        inactiveMap->setMapping (&(*engine->getHead(i)),i);
        head_active[i] = false;
        head_selected[i] = false;
        head_state[i] = -1;
    }
    // Clear the displays and led status
    displayWrite(0,0,"                                         ");
    displayWrite(0,40,"                                         ");
    for (unsigned int i=0; i < numberOfControls(); i++) {
        setControl(i,0);
    }
    shiftControl = controlByName("Shift");
    escControl = controlByName("Escape");
}

void MotorMix::controlChangedData(int control)
{
    bool shift = (getControlValue(shiftControl) != 0);
    bool escape = (getControlValue(escControl) != 0);
    setControl(shiftControl,shift ? 1 : 0);
    setControl(escControl,escape ? 1 : 0);
       
    if (!shift && !escape){
      // No Modifier keys 
      for (int i=0; i < MAX_HEADS;i++) {
	  if (control == headControls[i]) {
	      emit headSelectionChanged (i);
	      break;
	  }
      }
    } else if (shift && !escape){
      // Shift pressed
      
      
    } else if (!shift && escape){
      // escape pressed
      
    }
      
}

void MotorMix::outputEffectSelected(int effect)
{
    
  
  
}



void MotorMix::updateHeadStatus()
{
    for (unsigned int i = 0; i < MAX_HEADS; i++)
    {
        int hs = 0;
        if (head_selected[i]) {
            hs = 2;
        } else {
            if (head_active[i]) {
                hs = 1;
            }
        }
        if (hs != head_state[i]) {
            if ((hs == 1) && (head_state[i] == 2)) {
                // There is a bug in the motormix 1.08 firmware when switching from blink to on */
                setControl(headControls[i],0);
            }
            setControl(headControls[i],hs);
            head_state[i] = hs;
        }
    }
}


void MotorMix::headActive(int head)
{
    bool old = head_active[head];
    head_active[head] = true;
    if (!old) {
        updateHeadStatus();
    }
}

void MotorMix::headInactive(int head)
{
    bool old = head_active[head];
    head_active[head] = false;
    if (old) {
        updateHeadStatus();
    }
}

void MotorMix::headSelected(int head)
{
    for (int i = 0 ; i < MAX_HEADS; i++) {
        head_selected[i] = (head == i);
    }
    updateHeadStatus();
}

void MotorMix::message(QString text, int duration)
{
    messageTimer->stop();
    messageTimeout();
    text.truncate(40);
    displayWrite(0,0,text);
    messageTimer->start(duration);
}

void MotorMix::messageTimeout()
{
    QString s("                                        ");
    displayWrite(0,0,s);
    messageTimer->stop();
}

bool MotorMix::displayWrite(const unsigned int, const unsigned int position, const QString text)
{
    static const unsigned char MotorMixheader[9]={0xb0,0xf0,0x00,0x01,0x0f,0x00,0x11,0x00,0x10};
    std::vector<unsigned char>data;
    for (int i=0; i <9; i++) {
        data.push_back (MotorMixheader[i]);
    }
    data.push_back(position);
    for (int i=0; i < text.length(); i++) {
        data.push_back (text.toAscii()[i]);
    }
    data.push_back(0xf7);
    if (channel) {
        channel->send(data);
        return true;
    }
    return false;
}

bool MotorMix::connectMidi(MIDIChannel * midi)
{
    slog()->infoStream() << "Motormix driver on midi channel driver : "<< midi;
    channel = midi;
    if (midi) {
        connect(midi,SIGNAL(controller(int,int)),this,SLOT(midiControlValueChanged(int,int)));
        return true;
    }
    return false;
}

void MotorMix::midiControlValueChanged(int number, int value)
{
    // This little dance is because the motormix uses TWO control messages per event
    controllers[0] = controllers[1];
    controlvalues[0] = controlvalues[1];
    controllers[1] = number;
    controlvalues[1] = value;

    slog()->debugStream() << "Motormix:: control changed : " << number <<"::" << value;
    slog ()->debugStream() << "History : " <<controllers[0]<<" " <<controlvalues[0] <<" "<<controllers[1]<<" "<<controlvalues[1];


    // next iterate the switches to find the appropriate one
    for (unsigned int i = 0; i < numberOfControls(); i++) {
        if ((switches[i].ctrl1_number == controllers[0]) && (switches[i].cntrl1_value == controlvalues[0])
                && (switches[i].cntrl2_number == controllers[1])) {
            if (switches[i].cntrl2_pressed == controlvalues[1]) { // key pressed
                slog()->infoStream() << "Motormix key " << switches[i].name << " Pressed";
                controlValueChanged(i,1);
            } else {
                if (switches[i].cntrl2_released == controlvalues[1]) {//key released
                    slog()->infoStream() << "Motormix key " << switches[i].name << " Released";
                    controlValueChanged(i,0);
                } else {
                    continue;
                }
            }
            break;
        }
    }
}

bool MotorMix::setControl(unsigned int control, unsigned int val)
{
    if (control < numberOfControls()) {
        std::vector <unsigned char> data;
        data.push_back(0xb0);
        data.push_back(switches[control].led_ctl1_number);
        data.push_back(switches[control].led_ctl1_value);
        data.push_back(switches[control].led_ctl2_number);
        printf ("#%d \n",val);
        switch (val) {
        case 0:
            data.push_back(switches[control].led_ctl2_off);
            break;
        case 1:
            data.push_back(switches[control].led_ctl2_on);
            break;
        case 2:
            data.push_back(switches[control].led_ctl2_blink);
            break;
        default:
            return false;
        }
        // 5 byte switch LED control message ready to be queued
        if (channel) {
            channel->send(data);
        }
        return true;
    }
    return false;
}
