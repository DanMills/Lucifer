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

#include <iostream>
#include <assert.h>
#include <iostream>
#include <boost/make_shared.hpp>
#include <math.h>
#include <arpa/inet.h>

#include "log.h"
#include "staticframe.h"
#include "arcball.h"
#include <netinet/in.h>

#define NAME "Static_frame"

static SourceImplPtr makeStaticFrame()
{
    return boost::make_shared<StaticFrame>();
}

class StaticFrameGen
{
public:
    StaticFrameGen () {
        FrameSource_impl::registerFrameGen (NAME,makeStaticFrame);
    }
};
static StaticFrameGen sfg;

StaticFrame::StaticFrame () : FrameSource_impl(MAKES_FRAMES,NONE,NAME)
{
    setDescription("A single frame");
    slog()->debugStream() << "Created new static frame " << this;
    repeats = 1;
    dewell = 100;
    scale = 1.0f;
    useDewell = false;
}

StaticFrame::~StaticFrame ()
{
}

void StaticFrame::reserve (size_t points)
{
    data.reserve (points);
}

void StaticFrame::add_data (const ILDAPoint& p)
{
    data.push_back(p);
}

void StaticFrame::save (QXmlStreamWriter* w)
{
    assert (w);
    slog()->debugStream()<< "Saving static frame : " << this;
    w->writeAttribute("Points",QString().number(data.size()));
    w->writeAttribute("Use_Dewell", useDewell ? "True" : "False");
    w->writeAttribute("Dewell",QString().number(dewell));
    w->writeAttribute("Repeats",QString().number(repeats));
    w->writeAttribute("Scale",QString().number(scale));
    // Save the geometry matrix
    for (unsigned int i=0; i < 4; i++){
        for (unsigned j =0; j < 4; j++){
            w->writeAttribute(QString().sprintf("Geometry%d%d",j,i), QString().number(geometry(j,i)));
        }
    }
    // write out the point data
    QByteArray b;
    // 10 bytes per point
    b.reserve(10 * data.size());
    for (unsigned int i = 0; i < data.size(); i++) {
        const ILDAPoint p = data[i];
        unsigned short u;
        u=htons(p.x());
        b.append((char *)&u,2);
        u=htons(p.y());
        b.append((char *)&u,2);
        u=htons(p.z());
        b.append((char *)&u,2);
        b.append(p.r());
        b.append(p.g());
        b.append(p.b());
        b.append(p.blanked() ? 1 : 0);
    }
    // XML is icky for binary data so we convert to base 64
    // This is still better then saving each point as its own element.
    w->writeStartElement("PointList");
    w->writeCharacters(b.toBase64());
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
    scale = e->attributes().value("Scale").toString().toFloat();
    useDewell = (udw=="True") ? true : false;
    slog()->debugStream() << "Dewell : " << dewell;
    slog()->debugStream() << "Repeats : " << repeats;
    slog()->debugStream() << "Use Dewell : " << (useDewell ? "True" : "False");
    // Load the geometry matrix
    for (unsigned int i=0; i < 4; i++){
        for (unsigned j =0; j < 4; j++){
            geometry(j,i) = e->attributes().value(QString().sprintf("Geometry%d%d",j,i)).toString().toFloat();
        }
    }
    // Load the point list
    data.clear();
    data.reserve(pointcount);
    slog()->debugStream() << "Points : " << pointcount;
    while ((!e->atEnd()) && (e->name() != "PointList")){
        e->readNext();
    }
    if (e->name() == "PointList") {
        slog()->debugStream()<<"Found a PointList";
        e->readNext();
        QByteArray ba;
        ba.reserve(10 * pointcount);
        ba = QByteArray::fromBase64(e->text().toString().toUtf8());
        const char *b = ba.constData();
        // ba now contains the raw binary frame
        // 10 bytes per point
        slog()->debugStream()<<"ByteArray contains : " << ba.size() << " Bytes";
        for (int i=0; i < ba.size()/10; i++) {
            ILDAPoint p;
            const int a = 10 * i;
            p.setX(ntohs (*(unsigned short*)(b+a)));
            p.setY(ntohs (*(unsigned short*)(b+a+2)));
            p.setZ(ntohs (*(unsigned short*)(b+a+4)));
            p.setR(b[a+6]);
            p.setG(b[a+7]);
            p.setB(b[a+8]);
            p.setBlanked(b[a+9]);
            data.push_back(p);
        }
    } else {
        slog()->errorStream()<<"Failed to find valid point data: looking for 'PointList', found " << e->name().toString().toStdString();
    }
}

FramePtr StaticFrame::nextFrame(PlaybackImplPtr pb)
{
    assert (pb);
    StaticFramePlaybackPtr sp = boost::shared_dynamic_cast<StaticFramePlayback>(pb);
    assert (sp);
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
        return frame();
    } else {
        sp->active = true;
        sp->repeatsDone = 0;
    }
    return FramePtr();
}

FramePtr StaticFrame::frame() const
{
    FramePtr p = boost::make_shared<Frame>();
    p->geometry = geometry;
    p->geometry.scale(scale);
    for (unsigned int i=0; i < data.size(); ++i) {
        p->addPoint(data[i].point());
    }
    return p;
}

size_t StaticFrame::frames ()
{
    return 1;
}

size_t StaticFrame::pos (PlaybackImplPtr pb)
{
    assert (pb);
    StaticFramePlaybackPtr sp = boost::shared_dynamic_cast<StaticFramePlayback>(pb);
    assert (sp);
    return (sp->active) ? 1 : 0;
}

void StaticFrame::reset (PlaybackImplPtr pb)
{
    assert (pb);
    StaticFramePlaybackPtr sp = boost::shared_dynamic_cast<StaticFramePlayback>(pb);
    assert (sp);
    sp->active = true;
    sp->repeatsDone = 0;
    sp->dewellStart.start();
}

void StaticFrame::copyDataTo(SourceImplPtr p) const
{
    assert (p);
    StaticFramePtr sf = boost::dynamic_pointer_cast<StaticFrame>(p);
    assert (sf);
    sf->useDewell = useDewell;
    sf->dewell = dewell;
    sf->repeats = repeats;
    sf->scale = scale;
    sf->data.clear();
    sf->data.reserve(data.size());
    for (size_t i =0; i < data.size(); i++) {
        sf->data.push_back(data[i]);
    }
    sf->geometry = geometry;
    sf->setDescription(getDescription());
}

PlaybackImplPtr StaticFrame::newPlayback ()
{
    return boost::make_shared<StaticFramePlayback>();
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

    dewellSwitch->setChecked(fp->useDewell);
    repeatSwitch->setChecked(!fp->useDewell);
    dewellEntry->setDisabled(!fp->useDewell);
    repeatEntry->setDisabled(fp->useDewell);
    pointsDisplay->setNum((int)fp->data.size());
    dewellEntry->setValue(fp->dewell);
    repeatEntry->setValue (fp->repeats);
    size->setValue(100.0 * log10 (fp->scale));

    connect (group,SIGNAL(buttonClicked(int)),this,SLOT(buttonChangedData(int)));
    connect (dewellEntry,SIGNAL(valueChanged(int)),this,SLOT(dewellChangedData(int)));
    connect (repeatEntry,SIGNAL(valueChanged(int)),this,SLOT(repeatChangedData(int)));
    connect (size,SIGNAL(valueChanged(int)),this,SLOT(scaleChanged(int)));
    emit graphicsChanged();
}

StaticFrameGui::StaticFrameGui(QWidget* parent): FrameGui(parent)
{

    setTitle("Static Frame");

    group = new QButtonGroup (this);
    grid = new QGridLayout (this);

    dewellSwitch = new QRadioButton (this);
    dewellSwitch->setText("Dewell (ms)");
    dewellSwitch->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    repeatSwitch = new QRadioButton (this);
    repeatSwitch->setText("Repeat (frames)");
    repeatSwitch->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

    group->addButton(dewellSwitch,1);
    group->addButton(repeatSwitch,2);

    grid->addWidget(dewellSwitch,2,0,1,1);
    grid->addWidget(repeatSwitch,3,0,1,1);

    QLabel * pointsLabel = new QLabel ("Points",this);
    pointsLabel->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    pointsDisplay = new QLabel ("0",this);
    pointsDisplay->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
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
    QLabel *l = new QLabel (this);
    l->setText("ArcBall");
    l->setFrameShape(QFrame::Panel);
    l->setFrameShadow(QFrame::Raised);
    l->setAlignment(Qt::AlignHCenter);
    arcball = new ArcBall(this);
    connect (arcball,SIGNAL(angleChanged(QQuaternion)),this,SLOT(angleChangedData(QQuaternion)));
    connect (arcball,SIGNAL(mouseDown()),this,SLOT(arcballDown()));
    connect (arcball,SIGNAL(mouseUp()),this,SLOT(arcballUp()));
    grid->addWidget(l,4,0);
    grid->addWidget(arcball,5,0);
    l = new QLabel (this);
    l->setText ("Size");
    l->setFrameShape(QFrame::Panel);
    l->setFrameShadow(QFrame::Raised);
    l->setAlignment(Qt::AlignHCenter);
    grid->addWidget(l,4,1);
    size = new QSlider(Qt::Vertical,this);
    size->setRange(-200,100);
    size->setSliderPosition(0);
    grid->addWidget(size,5,1);
    setLayout(grid);
}

StaticFrameGui::~StaticFrameGui()
{
    fp = NULL;
    slog()->debugStream() << "Deleted Static frame GUI "<< this;
}

void StaticFrameGui::angleChangedData(QQuaternion q)
{
    fp->geometry = arcball->rotate();
    emit graphicsChanged(); //Update the thumbnail
}

void StaticFrameGui::arcballDown()
{
    arcball->setMatrix(fp->geometry);
}

void StaticFrameGui::arcballUp()
{
    fp->geometry = arcball->rotate();
}

void StaticFrameGui::scaleChanged(int v)
{
    fp->scale = pow (10.0,v/100.0);
    emit graphicsChanged(); // update the thumbnail
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
    QPixmap p(48,48);
    if (fp) {
        FramePtr f = fp->frame();
        if (f) {
            QPainter q(&p);
            q.setBackground (Qt::black);
            q.setRenderHint(QPainter::HighQualityAntialiasing);
            q.setBrush(QBrush(Qt::black));
            q.drawRect(0,0,48,48);
            f->render (q,0,0,48,48);
        }
    }
    QIcon * i = new QIcon(p);
    return i;
}

