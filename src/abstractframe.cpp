/*Abstractframe.cpp is part of lucifer a laser show controller.

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

#include <iostream>
#include <assert.h>
#include <iostream>
#include <boost/make_shared.hpp>
#include <math.h>

#include "log.h"
#include "abstractframe.h"

static FrameSourcePtr makeAbstractFrame()
{
    return boost::make_shared<AbstractFrame>();
}

class AbstractFrameGen
{
public:
    AbstractFrameGen () {
        FrameSource_impl::registerFrameGen ("Abstract_frame",makeAbstractFrame);
    }
};
static AbstractFrameGen sfg;

AbstractFrame::AbstractFrame () : FrameSource(MAKES_FRAMES,NONE)
{
    name  = "Abstract_frame";
    description = "A source of abstract frames";
    slog()->debugStream() << "Created new abstract frame " << this;
    // Setup defaults
    formula = FUNCTION::ORDER1;
    zformula = FUNCTION::ORDER1;
    colourPtn = FUNCTION::ORDER1;
    envelope = ENVELOPE::SINE;

    ZChngNum = 0.0f;
    ZzChngNum = 0.0f;
    ZzzChngNum = 0.0f;
    GChngNum = 0.0f;

    colourSpd = 0.0f;
    blankSpd = 0.0f;
    blankSpot = 0.0f;

    initialZr = 0.0f;
    initialZb = 0.0f;
    initialZg = 0.0f;

    offsetR = offsetB = offsetG = 0.0f;
    offsetZ = offsetZz = offsetZzz =0.0f;
    gainZ = gainZz = gainZzz = 0.0f;
    zAxisGain = 0.0f;

    envelopeDepth = 0.0f;

    initialBlankerizer = 0.0f;
    initialColoriser = 0.0f;
    initialG = initialZ = initialZz = initialZzz = 0.0f;
};

AbstractFrame::~AbstractFrame ()
{
    slog()->debugStream() << "Deleted abstract frame " << this;
}

void AbstractFrame::save (QXmlStreamWriter* w)
{
    assert (w);
    slog()->debugStream()<< "Saving abstract frame : " << this;

    w->writeAttribute("Formula",QString().number(formula));
    w->writeAttribute("ZFormula",QString().number(zformula));
    w->writeAttribute("ColourPtn",QString().number(colourPtn));
    w->writeAttribute("Envelope",QString().number(envelope));

    w->writeAttribute("OffsetZ",QString().number(offsetZ));
    w->writeAttribute("OffsetZz",QString().number(offsetZz));
    w->writeAttribute("OffsetZzz",QString().number(offsetZzz));

    w->writeAttribute("GainZ",QString().number(gainZ));
    w->writeAttribute("GainZz",QString().number(gainZz));
    w->writeAttribute("GainZzz",QString().number(gainZzz));

    w->writeAttribute("OffsetR",QString().number(offsetR));
    w->writeAttribute("OffsetG",QString().number(offsetG));
    w->writeAttribute("OffsetB",QString().number(offsetB));

    w->writeAttribute("GChngNum",QString().number(GChngNum));
    w->writeAttribute("ZChngNum",QString().number(ZChngNum));
    w->writeAttribute("ZzChngNum",QString().number(ZzChngNum));
    w->writeAttribute("ZzzChngNum",QString().number(ZzzChngNum));

    w->writeAttribute("ColourSpd",QString().number(colourSpd));
    w->writeAttribute("BlankSpd",QString().number(blankSpd));
    w->writeAttribute("BlankSpot",QString().number(blankSpot));

    w->writeAttribute("ZAxisGain",QString().number(zAxisGain));
    w->writeAttribute("EnvelopeDepth",QString().number(envelopeDepth));

    w->writeAttribute("InitialZR",QString().number(initialZr));
    w->writeAttribute("InitialZG",QString().number(initialZg));
    w->writeAttribute("InitialZB",QString().number(initialZb));

    w->writeAttribute("InitialZ",QString().number(initialZ));
    w->writeAttribute("InitialZz",QString().number(initialZz));
    w->writeAttribute("InitialZzz",QString().number(initialZzz));
    w->writeAttribute("InitialG",QString().number(initialG));
    w->writeAttribute("InitialBlankeriser",QString().number(initialBlankerizer));
    w->writeAttribute("InitialColouriser",QString().number(initialColoriser));
}

void AbstractFrame::load(QXmlStreamReader* e)
{
#if 0
    assert (e);
    int pointcount = 1;
    std::string udw = "False";
    dewell = 0;
    repeats = 1;
    slog()->debugStream() << "Loading static frame : "<< this;
    pointcount= e->attributes().value("Points").toString().toInt();
    udw = e->attributes().value("Use_Dewell").toString().toStdString();
    dewell = e->attributes().value("Dewell").toString().toInt();
    repeats = e->attributes().value("Repeats").toString().toInt();
    useDewell = (udw=="True") ? true : false;
    slog()->debugStream() << "Dewell : " << dewell;
    slog()->debugStream() << "Repeats : " << repeats;
    slog()->debugStream() << "Use Dewell : " << (useDewell ? "True" : "False");
    data_->clear();
    data_->reserve(pointcount);
    slog()->debugStream() << "Points : " << pointcount;
    e->readNextStartElement();
    if (e->name() == "PointList") {
        slog()->debugStream()<<"Found a PointList";
        e->readNext();
        QByteArray ba;
        ba.reserve(16 * pointcount);
        ba = QByteArray::fromBase64(e->text().toString().toUtf8());
        const char *b = ba.constData();
        // ba now contains the raw binary frame
        // 16 bytes per point
        slog()->debugStream()<<"ByteArray contains : " << ba.size() << " Bytes";
        for (int i=0; i < ba.size()/16; i++) {
            Point p;
            const int a = 16 * i;
            p.setX(itof (*(unsigned int*)(b+a)));
            p.setY(itof (*(unsigned int*)(b+a+4)));
            p.setZ(itof (*(unsigned int*)(b+a+8)));
            p.r = b[a+12];
            p.g = b[a+13];
            p.b = b[a+14];
            p.blanked = b[a+15];
            data_->addPoint(p);
        }
    } else {
        slog()->errorStream()<<"Failed to find valid point data";
    }
#endif
}

FramePtr AbstractFrame::data(Playback p)
{
    AbstractFramePlayback *sp = dynamic_cast<AbstractFramePlayback*>(getPlayback(p));

    return FramePtr();
}

size_t AbstractFrame::frames ()
{
    return 1;
}

size_t AbstractFrame::pos (Playback p)
{
	#if 0
    StaticFramePlayback *sp = dynamic_cast<StaticFramePlayback*>(getPlayback(p));
    return (sp->active) ? 1 : 0;
	#endif
}

void AbstractFrame::reset (Playback p)
{
    StaticFramePlayback *sp = dynamic_cast<StaticFramePlayback*>(getPlayback(p));
    sp->active = true;
    sp->repeatsDone = 0;
    sp->dewellStart.start();
}

void AbstractFrame::copyDataTo(FrameSourcePtr p) const
{
    assert (p);
    StaticFrame * sf = dynamic_cast<StaticFrame*> (p.get());
    sf->useDewell = useDewell;
    sf->dewell = dewell;
    sf->repeats = repeats;
    sf->data_->clear();
    sf->data_->reserve(data_->getPointCount());
    for (size_t i =0; i < data_->getPointCount(); i++) {
        sf->data_->addPoint(data_->getPoint(i));
    }
}

PlaybackDataPtr AbstractFrame::createPlaybackData ()
{
    return PlaybackDataPtr(new AbstractFramePlayback);
}

// GUI controls from here on down
#if 0
void StaticFrameGui::dewellChangedData (int value)
{
    fp->dewell = value;
}

void StaticFrameGui::repeatChangedData(int value)
{
    fp->repeats = value;
}

void StaticFrameGui::buttonChangedData(int id)
{
    assert (fp);
    switch (id) {
    case 1:
        // dewell mode selected
        fp->useDewell = true;
        dewellEntry->setDisabled(false);
        repeatEntry->setDisabled(true);
        break;
    case 2:
        fp->useDewell = false;
        dewellEntry->setDisabled(true);
        repeatEntry->setDisabled(false);
        break;
    }
}

void StaticFrameGui::set (StaticFrame * p)
{
    fp = p;
    assert (fp);
    display->setFrame(fp->data_);
    dewellSwitch->setChecked(fp->useDewell);
    repeatSwitch->setChecked(!fp->useDewell);
    dewellEntry->setDisabled(!fp->useDewell);
    repeatEntry->setDisabled(fp->useDewell);
    pointsDisplay->setNum((int)fp->data_->getPointCount());
    dewellEntry->setValue(fp->dewell);
    repeatEntry->setValue (fp->repeats);

    connect (group,SIGNAL(buttonClicked(int)),this,SLOT(buttonChangedData(int)));
    connect (dewellEntry,SIGNAL(valueChanged(int)),this,SLOT(dewellChangedData(int)));
    connect (repeatEntry,SIGNAL(valueChanged(int)),this,SLOT(repeatChangedData(int)));
}

StaticFrameGui::StaticFrameGui(QWidget* parent): FrameGui(parent)
{

    setTitle("Static Frame");

    group = new QButtonGroup (this);
    grid = new QGridLayout (this);

    display = new DisplayFrame (this);
    grid->addWidget(display,0,0,1,2);

    dewellSwitch = new QRadioButton (this);
    dewellSwitch->setText("Dewell (ms)");
    repeatSwitch = new QRadioButton (this);
    repeatSwitch->setText("Repeat (frames)");
    group->addButton(dewellSwitch,1);
    group->addButton(repeatSwitch,2);

    grid->addWidget(dewellSwitch,2,0,1,1);
    grid->addWidget(repeatSwitch,3,0,1,1);

    QLabel * pointsLabel = new QLabel ("Points",this);
    pointsDisplay = new QLabel ("0",this);
    grid->addWidget(pointsLabel,1,0,1,1);
    grid->addWidget(pointsDisplay,1,1,1,1);

    dewellEntry = new QSpinBox (this);
    dewellEntry->setMinimum (40);
    dewellEntry->setMaximum (600000);

    repeatEntry = new QSpinBox (this);
    repeatEntry->setMinimum (1);
    repeatEntry->setMaximum (1000);

    grid->addWidget(dewellEntry,2,1,1,1);
    grid->addWidget(repeatEntry,3,1,1,1);
    setLayout(grid);
}

StaticFrameGui::~StaticFrameGui()
{
    fp = NULL;
    slog()->debugStream() << "Deleted Static frame GUI "<< this;
}

FrameGui * StaticFrame::controls (QWidget *parent)
{
    assert (this);
    StaticFrameGui *g = new StaticFrameGui (parent);
    assert (g);
    g->set(this);
    slog()->debugStream() << "Created Static frame GUI "<< this;
    return g;
}

const QIcon * StaticFrameGui::icon()
{
    return new QIcon();
}
#endif
