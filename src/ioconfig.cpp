#include "ioconfig.h"
#include "alsamidi.h"

IOConfiguration::IOConfiguration(EnginePtr e, QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    engine = e;
    setAttribute(Qt::WA_DeleteOnClose,true);
    // load the window position if available
    QSettings settings;
    settings.beginGroup("IO Configuration Window");
    if (settings.contains("Geometry")){
      restoreGeometry(settings.value("Geometry").toByteArray());
    }
    settings.endGroup();
    
    setWindowTitle(tr("IO Setup"));
    QGridLayout *grid = new QGridLayout (this);
    setLayout(grid);
    QLabel *label = new QLabel (this);
    label->setText(tr("Midi"));
    label->setAlignment(Qt::AlignHCenter);
    label->setFrameShape(QFrame::Panel);
    grid->addWidget(label,0,0,1,2);  
    
    QComboBox *midiPort = new QComboBox(this);
    label = new QLabel (this);
    label->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    label->setText(tr("Midi port"));
    grid->addWidget(label,1,0);
    std::vector <std::pair<QString, QString> > ports;
    ports = AlsaMidi::enumeratePorts();
    midiPort->addItem(tr("None"));
    for (unsigned int i=0; i < ports.size(); i++) {
        midiPort->addItem(ports[i].second);
    }
    grid->addWidget(midiPort,1,1);
    QComboBox *channel = new QComboBox(this);
    for (unsigned int i=0; i < 16; i++){
      channel->addItem(QString().sprintf("Channel %d",i+1));
    }
    grid->addWidget(channel,2,0);
    QComboBox *chanDriver = new QComboBox (this);
    chanDriver->addItem(tr("None"));
    chanDriver->addItem(tr("MotorMix"));
    chanDriver->addItem(tr("Keyboard"));
    grid->addWidget(chanDriver,2,1);
    
    label = new QLabel (this);
    label->setText(tr("Audio"));
    label->setFrameShape(QFrame::Panel);
    label->setAlignment(Qt::AlignHCenter);
    grid->addWidget(label,3,0,1,2);
    
    label = new QLabel (this);
    label->setText(tr("Main audio output"));
    grid->addWidget(label,4,0);
    QComboBox * audiocard = new QComboBox (this);
    audiocard->addItem(tr("None"));
    grid->addWidget(audiocard,4,1);
    label = new QLabel (this);
    label->setText(tr("Monitor audio output"));
    grid->addWidget(label,5,0);
    audiocard = new QComboBox (this);
    audiocard->addItem(tr("None"));
    grid->addWidget(audiocard,5,1);
    
    
    label = new QLabel (this);
    label->setText(tr("Audio input"));
    grid->addWidget(label,6,0);
    audiocard = new QComboBox (this);
    audiocard->addItem(tr("None"));
    grid->addWidget(audiocard,6,1);    
}

IOConfiguration::~IOConfiguration()
{
}

void IOConfiguration::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.beginGroup("IO Configuration Window");
    settings.setValue("Geometry",saveGeometry());
    settings.endGroup();
    QDialog::closeEvent(event);
}


