/*staticframe.cpp is part of lucifer a laser show controller.

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

/* $Id: staticframe.cpp 22 2010-07-24 15:01:51Z dmills $ */

#include <iostream>
#include <assert.h>
#include <iostream>
#include <boost/make_shared.hpp>
#include <math.h>
#include <arpa/inet.h>

#include "log.h"
#include "staticframe.h"
#include <netinet/in.h>

static FrameSourcePtr makeStaticFrame()
{
    return boost::make_shared<StaticFrame>();
}

class StaticFrameGen
{
public:
    StaticFrameGen () {
        FrameSource::registerFrameGen ("Static_frame",makeStaticFrame);
    }
};
static StaticFrameGen sfg;

StaticFrame::StaticFrame () : FrameSource(MAKES_FRAMES,NONE)
{
    name  = "Static_frame";
    description = "A single frame";
    slog()->debugStream() << "Created new static frame " << this;
    data_ = boost::make_shared<Frame>();
    repeats = 1;
    dewell = 100;
    useDewell = false;
		// default geometry = no rotation, no spinning, scale 1:1;
		//startAngleX = startAngleY = startAngleZ = 0.0;
		//incrementX = incrementY = incrememtZ = 0;
		//centerX = centerY = centreZ = 0;
		//scaleX = scaleY = scaleZ = 1.0;
}

StaticFrame::~StaticFrame ()
{
    slog()->debugStream() << "Deleted static frame " << this;
}

void StaticFrame::reserve (size_t points)
{
    data_->reserve (points);
}

void StaticFrame::add_data (const Point &p)
{
    data_->addPoint(p);
}

// This technically breaks strict aliasing.
// but almost every compiler lets you do it.
unsigned int StaticFrame::ftoi (float f)
{
    union {
        float f;
        unsigned int i;
    } u;
    assert (sizeof(u.f) == sizeof (u.i));
    u.f = f;
    unsigned int i = u.i;
    i = htonl (i);
    return i;
}

float StaticFrame::itof(unsigned int i)
{
    union {
        float f;
        unsigned int i;
    } u;
    assert (sizeof(u.f) == sizeof (u.i));
    u.i = ntohl(i);
    return u.f;
}


void StaticFrame::save (QXmlStreamWriter* w)
{
    assert (w);
    slog()->debugStream()<< "Saving static frame : " << this;
    w->writeAttribute("Points",QString().number(data_->getPointCount()));
    w->writeAttribute("Use_Dewell", useDewell ? "True" : "False");
    w->writeAttribute("Dewell",QString().number(dewell));
    w->writeAttribute("Repeats",QString().number(repeats));
    QByteArray b;
    // 16 bytes per point
    b.reserve(16 * data_->getPointCount());
    assert (sizeof (unsigned int) == 4);
    assert (sizeof (float) == 4);
    for (unsigned int i = 0; i < data_->getPointCount(); i++) {
        Point p = data_->getPoint(i);
        unsigned int v;
        v = ftoi (p.v[Point::X]);
        b.append((char*)&v,4);
        v = ftoi(p.v[Point::Y]);
        b.append((char*)&v,4);
        v = ftoi(p.v[Point::Z]);
        b.append((char*)&v,4);
        b.append(p.r);
        b.append(p.g);
        b.append(p.b);
        b.append(p.blanked);
    }
    // XML is icky for binary data so we convert to base 64
    // This is still better then saving each point as its own element.
    w->writeStartElement("PointList");
    w->writeCharacters(qCompress(b).toBase64());
    w->writeEndElement();
}

void StaticFrame::load(QXmlStreamReader* e)
{
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
        ba = qUncompress(QByteArray::fromBase64(e->text().toString().toUtf8()));
        const char *b = ba.constData();
        // ba now contains the raw binary frame
        // 16 bytes per point
        slog()->debugStream()<<"ByteArray contains : " << ba.size() << " Bytes";
        for (int i=0; i < ba.size()/16; i++) {
            Point p;
            const int a = 16 * i;
            p.v[Point::X] = itof (*(unsigned int*)(b+a));
            p.v[Point::Y] = itof (*(unsigned int*)(b+a+4));
            p.v[Point::Z] = itof (*(unsigned int*)(b+a+8));
            p.r = b[a+12];
            p.g = b[a+13];
            p.b = b[a+14];
            p.blanked = b[a+15];
            data_->addPoint(p);
        }
    } else {
        slog()->errorStream()<<"Failed to find valid point data";
    }
}

FramePtr StaticFrame::data(Playback p)
{
    StaticFramePlayback *sp = dynamic_cast<StaticFramePlayback*>(getPlayback(p));
    if (useDewell) {
        // Dewell timer active
        if (sp->dewellStart.elapsed() > (int)dewell) {
            sp->dewellStart.restart();
            sp->active = false;
        }
    } else {
        if (sp->repeatsDone++ >= repeats) {
            sp->active = false;
        }
    }
    if (sp->active) {
			FramePtr p = boost::make_shared<Frame>();
			*p = *data_;
			return p;
    } else {
        sp->active = true;
        sp->repeatsDone = 0;
    }
    return FramePtr();
}

size_t StaticFrame::frames ()
{
    return 1;
}

size_t StaticFrame::pos (Playback p)
{
    StaticFramePlayback *sp = dynamic_cast<StaticFramePlayback*>(getPlayback(p));
    return (sp->active) ? 1 : 0;
}

void StaticFrame::reset (Playback p)
{
    StaticFramePlayback *sp = dynamic_cast<StaticFramePlayback*>(getPlayback(p));
    sp->active = true;
    sp->repeatsDone = 0;
    sp->dewellStart.start();
}

PlaybackDataPtr StaticFrame::createPlaybackData ()
{
    return PlaybackDataPtr(new StaticFramePlayback);
}

// GUI controls from here on down

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

