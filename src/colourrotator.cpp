/*colourrotator.cpp is part of lucifer a laser show controller.

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

#include <vector>
#include <assert.h>
#include "framesource_impl.h"
#include <boost/make_shared.hpp>

#include "point.h"
#include "colourrotator.h"
#include "displayframe.h"
#include "log.h"

#define NAME "Colour_rotator"


static SourceImplPtr makeColourRotator()
{
    return boost::make_shared<ColourRotator>();
}

class ColourRotatorGen
{
public:
    ColourRotatorGen () {
        FrameSource_impl::registerFrameGen (NAME,makeColourRotator);
    }
};

static ColourRotatorGen crg;

ColourRotator::ColourRotator() : FrameSource_impl (FrameSource_impl::EFFECT,FrameSource_impl::ONE,NAME)
{
    setDescription("Colour modulation effect");
    slog()->debugStream() << "Created new colour modulator " << this;
    //Colour pulse parameters
    pulse_colour = Qt::yellow;
    pulse_dutycycle = 0.05;
    pulse_harmonic = 5;
    pulse_phase_advance = 0.1;
    // HSVRotator parameters
    rotate_h = rotate_s = rotate_v = false;
    rotate_harmonic = 0;
    rotate_phase_advance = 0.05;
    // ColourOverride
    colour_override = false;
    override_colour = Qt::cyan;

}

ColourRotator::~ColourRotator()
{
}

PlaybackImplPtr ColourRotator::newPlayback()
{
    return boost::make_shared<ColourRotatorPlayback>();
}

void ColourRotator::save(QXmlStreamWriter* w)
{
    assert (w);
    int r,g,b;
    pulse_colour.getRgb(&r,&g,&b);
    // colour pulser
    w->writeAttribute("Pulse_colour_r",QString().number(r));
    w->writeAttribute("Pulse_colour_g",QString().number(g));
    w->writeAttribute("Pulse_colour_b",QString().number(b));
    w->writeAttribute("Pulse_dutycycle",QString().number(pulse_dutycycle));
    w->writeAttribute("Pulse_harmonic", QString().number(pulse_harmonic));
    w->writeAttribute("Pulse_phase_advance", QString().number(pulse_phase_advance));
    // rotator parameters
    w->writeAttribute("Rotate_H", QString().number(rotate_h));
    w->writeAttribute("Rotate_S", QString().number(rotate_s));
    w->writeAttribute("Rotate_V", QString().number(rotate_v));
    w->writeAttribute("Rotate_harmonic", QString().number(rotate_harmonic));
    w->writeAttribute("Rotate_phase_advance", QString().number(rotate_phase_advance));
    // colour override
    w->writeAttribute("Colour_override", QString().number(colour_override));
    override_colour.getRgb(&r,&g,&b);
    w->writeAttribute("override_colour_r",QString().number(r));
    w->writeAttribute("override_colour_g",QString().number(g));
    w->writeAttribute("override_colour_b",QString().number(b));
}

void ColourRotator::load(QXmlStreamReader* e)
{
    assert (e);
    int r = e->attributes().value("Pulse_colour_r").toString().toInt();
    int g = e->attributes().value("Pulse_colour_g").toString().toInt();
    int b = e->attributes().value("Pulse_colour_b").toString().toInt();
    pulse_colour.setRgb(r,g,b);
    pulse_dutycycle = e->attributes().value("Pulse_dutycycle").toString().toFloat();
    pulse_harmonic =  e->attributes().value("Pulse_harmonic").toString().toInt();
    pulse_phase_advance = e->attributes().value("Pulse_phase_advance").toString().toFloat();
    rotate_h = e->attributes().value("Rotate_H").toString().toInt();
    rotate_s = e->attributes().value("Rotate_S").toString().toInt();
    rotate_v = e->attributes().value("Rotate_V").toString().toInt();
    rotate_harmonic = e->attributes().value("Rotate_harmonic").toString().toInt();
    rotate_phase_advance = e->attributes().value("Rotate_phase_advance").toString().toFloat();
    colour_override = e->attributes().value("Colour_override").toString().toInt();
    r = e->attributes().value("override_colour_r").toString().toInt();
    g = e->attributes().value("override_colour_g").toString().toInt();
    b = e->attributes().value("override_colour_b").toString().toInt();
    override_colour.setRgb(r,g,b);
}

void ColourRotator::copyDataTo(SourceImplPtr p) const
{
    ColourRotatorPtr pt = boost::dynamic_pointer_cast<ColourRotator>(p);
    assert (pt);

    pt->pulse_colour = pulse_colour;
    pt->pulse_dutycycle = pulse_dutycycle;
    pt->pulse_harmonic = pulse_harmonic;
    pt->pulse_phase_advance = pulse_phase_advance;

    pt->rotate_h = rotate_h;
    pt->rotate_s = rotate_s;
    pt->rotate_v = rotate_v;
    pt->rotate_harmonic = rotate_harmonic;
    pt->rotate_phase_advance = rotate_phase_advance;

    pt->colour_override = pt->colour_override;
    pt->override_colour = override_colour;
}

void ColourRotator::reset(PlaybackImplPtr p)
{
    assert (p);
    ColourRotatorPlaybackPtr pb = boost::dynamic_pointer_cast<ColourRotatorPlayback>(p);
    assert (pb);
    pb->pulser_start_phase = 0;
    pb->rotator_start_phase = 0;
    if (numChildren() == 1) {
        child(0)->reset (pb->child(0));
    }
};

FramePtr ColourRotator::nextFrame(PlaybackImplPtr p)
{
    assert (p);
    ColourRotatorPlaybackPtr pb = boost::dynamic_pointer_cast<ColourRotatorPlayback>(p);
    assert (pb);
    FramePtr ps;
    if (numChildren() == 0) {
        // Nothing to rotate
        reset (pb);
        return ps;
    }
    ps = child(0)->nextFrame(pb->child(0));
    // if the child is still returning data then we have work to do
    if (ps) {
        colourOverride(ps,pb);
        HSVRotator(ps,pb);
        colourPulse(ps,pb);
    } else { // Got a null source
        if (numChildren() == 1) {
            child(0)->reset (pb->child(0));
        }
    }
    return ps;
}


void ColourRotator::colourOverride(FramePtr frame, ColourRotatorPlaybackPtr)
{
    if (colour_override && frame) {
        int r,g,b;
        override_colour.getRgb(&r,&g,&b);
        for (unsigned int i=0; i < frame->getPointCount(); i++) {
            Point p = frame->getPoint(i);
            p.setRgb(r,g,b);
            frame->setPoint(i,p);
        }
    }
}

void ColourRotator::HSVRotator(FramePtr frame, ColourRotatorPlaybackPtr pb)
{
    float phase = pb->rotator_start_phase;
    if (frame && frame->getPointCount()) {
        float phase_incr = rotate_harmonic/(float)frame->getPointCount();
        for (unsigned int i=0; i < frame->getPointCount(); i++) {
            Point p = frame->getPoint(i);
            if (1/*!p.blanked*/) {
                float h = p.hueF();
                float s = p.saturationF();
                float v = p.valueF();
                if (rotate_h) {
                    h += phase;
                }
                if (rotate_v) {
                    v *= phase;
                }
                if (rotate_s) {
                    s += phase;
                }
                while (h > 1.0f) h -= 1.0f;
                while  (h < 0.0f) h += 1.0f;
                while (v > 1.0) v = 1.0;
                while (v < 0.0) v = 0.0;
                while (s > 1.0) s -=  1.0;
                while (s < 0.0) s += 1.0;
                phase += phase_incr;
                if (phase > 1.0) phase -= 1.0;
                if (phase  < 0.0) phase += 1.0;

                if ((h >= 0.0) && (h <= 1.0) && (v >= 0.0) && (v <= 1.0) && (s >= 0.0) && (s <= 1.0)) {
                    p.setHsvF(h,s,v);
                    p.toRgb();
                    frame->setPoint(i,p);
                } else {
                    slog()->errorStream() << "Out of range " << h <<" : " << s << " : " << v;
                }
            }
        }
        pb->rotator_start_phase += rotate_phase_advance;
        if (pb->rotator_start_phase > 1.0) pb->rotator_start_phase -= 1.0;
    }
}

void ColourRotator::colourPulse(FramePtr frame, ColourRotatorPlaybackPtr pb)
{
    if (frame && frame->getPointCount() && pulse_harmonic) {
        float phase = pb->pulser_start_phase;
        float per_point_incr = (1.0/frame->getPointCount()) * pulse_harmonic;
        for (unsigned int i =0; i < frame->getPointCount(); i++) {
            if (phase < pulse_dutycycle) {
                Point pt = frame->getPoint(i);
                if (!pt.blanked) {
                    int r,g,b;
                    pulse_colour.getRgb(&r,&g,&b);
                    pt.setRgb(r,g,b);
                    frame->setPoint(i,pt);
                }
            }
            phase += per_point_incr;
            if (phase > 1.0) phase -= 1.0;
        }
        pb->pulser_start_phase += pulse_phase_advance;
        if (pb->pulser_start_phase > 1.0) pb->pulser_start_phase -= 1.0;
        if (pb->pulser_start_phase < 0.0) pb->pulser_start_phase += 1.0;
    }
}

size_t ColourRotator::pos(PlaybackImplPtr)
{
    return 0;
}

size_t ColourRotator::frames()
{
    return numChildren() ? child(0)->frames() : 0;
}

FrameGui* ColourRotator::controls(QWidget* parent)
{
    return new ColourRotatorGui (this, parent);
}

ColourRotatorPlayback::ColourRotatorPlayback()
{
    // colour pulser
    pulser_start_phase = 0.0;
    rotator_start_phase = 0.0;
}

// GUI from here down


ColourRotatorGui::ColourRotatorGui(ColourRotator * c, QWidget* parent): FrameGui(parent)
{
    setTitle(tr("Colour Modulator"));
    QGridLayout * grid = new QGridLayout (this);
    setLayout(grid);

    rotator = c;
    assert (rotator);

    QLabel *label = new QLabel(this);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label->setFrameShape(QFrame::Panel);
    label->setAlignment(Qt::AlignCenter);
    label->setText(tr("Colour Pulser"));
    grid->addWidget(label,0,0,1,2);

    label = new QLabel(this);
    label->setText(tr("Number"));
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label->setAlignment(Qt::AlignCenter);
    grid->addWidget(label, 1, 0);

    label = new QLabel(this);
    label->setText(tr("Speed"));
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label->setAlignment(Qt::AlignCenter);
    grid->addWidget(label, 1, 1);

    pulserHarmonic = new QDial(this);
    pulserHarmonic->setMaximum(50000);
    pulserHarmonic->setNotchesVisible(true);
    pulserHarmonic->setValue(1000 * rotator->pulse_harmonic);
    connect (pulserHarmonic,SIGNAL(valueChanged(int)),this,SLOT(pulserHarmonicData(int)));
    grid->addWidget(pulserHarmonic, 2, 0);

    pulserPhaseIncr = new QDial(this);
    pulserPhaseIncr->setMinimum(-9900);
    pulserPhaseIncr->setMaximum(9900);
    pulserPhaseIncr->setValue(1000 * rotator->pulse_phase_advance);
    pulserPhaseIncr->setNotchesVisible(this);
    connect (pulserPhaseIncr,SIGNAL(valueChanged(int)),this,SLOT(pulserPhaseIncData(int)));

    grid->addWidget(pulserPhaseIncr, 2, 1);

    pulseColourSelector = new QtColorTriangle (this);
    pulseColourSelector->setColor(rotator->pulse_colour);
    connect (pulseColourSelector,SIGNAL(colorChanged(QColor)),this,SLOT(pulserColourChangedData(QColor)));
    grid->addWidget(pulseColourSelector,3,0,1,2);

    label = new QLabel(this);
    label->setText(tr("Rotator"));
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label->setFrameShape(QFrame::Panel);
    label->setAlignment(Qt::AlignCenter);

    grid->addWidget(label,7,0,1,2);

    label = new QLabel(this);
    label->setText(tr("Cycles per frame"));
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    grid->addWidget(label,8,0);

    label = new QLabel(this);
    label->setText(tr("Speed"));
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    grid->addWidget(label, 8, 1);

    rotatorHarmonic = new QDial(this);
    rotatorHarmonic->setMaximum(20000);
    rotatorHarmonic->setNotchesVisible(true);
    rotatorHarmonic->setValue(1000 * rotator->rotate_harmonic);
    grid->addWidget(rotatorHarmonic, 9, 0);
    connect (rotatorHarmonic,SIGNAL(valueChanged(int)),this,SLOT(rotatorHarmonicData(int)));


    rotatorPhaseIncr = new QDial(this);
    rotatorPhaseIncr->setMinimum(-5000);
    rotatorPhaseIncr->setMaximum(5000);
    rotatorPhaseIncr->setNotchesVisible(true);
    rotatorPhaseIncr->setValue(1000 * rotator->rotate_phase_advance);
    connect (rotatorPhaseIncr,SIGNAL(valueChanged(int)),this,SLOT(rotatorPhaseIncrData(int)));
    grid->addWidget(rotatorPhaseIncr, 9, 1);

    hueMod = new QCheckBox(this);
    hueMod->setText(tr("Modulate hue"));
    hueMod->setChecked(rotator->rotate_h);
    connect(hueMod,SIGNAL(clicked(bool)),this,SLOT(rotatorHueModData(bool)));
    grid->addWidget(hueMod, 10, 0, 1, 2);

    brightMod = new QCheckBox(this);
    brightMod->setText(tr("Modulate brightness"));
    brightMod->setChecked(rotator->rotate_v);
    connect(brightMod,SIGNAL(clicked(bool)),this,SLOT(rotatorBrightModData(bool)));
    grid->addWidget(brightMod, 11, 0, 1, 2);

    satMod = new QCheckBox(this);
    satMod->setText(tr("Modulate saturation"));
    satMod->setChecked(rotator->rotate_s);
    connect(satMod,SIGNAL(clicked(bool)),this,SLOT(rotatorSatModData(bool)));
    grid->addWidget(satMod, 12, 0, 1, 2);

    label = new QLabel(this);
    label->setText(tr("Override"));
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label->setFrameShape(QFrame::Panel);
    label->setAlignment(Qt::AlignCenter);

    grid->addWidget(label,13,0,1,2);

    overrideSwitch = new QCheckBox(this);
    overrideSwitch->setText(tr("Enable"));
    overrideSwitch->setChecked(rotator->colour_override);
    grid->addWidget(overrideSwitch,14,0);
    connect (overrideSwitch,SIGNAL(clicked(bool)),this,SLOT(overrideSwitchData(bool)));

    overrideColourSelector = new QtColorTriangle (this);
    overrideColourSelector->setColor(rotator->override_colour);
    connect (overrideColourSelector,SIGNAL(colorChanged(QColor)),this,SLOT(overrideColourChangedData(QColor)));
    grid->addWidget(overrideColourSelector,15,0,1,2);

}

void ColourRotatorGui::overrideColourChangedData(QColor col)
{
    if (rotator) {
        rotator->override_colour =  col;
    }
}

void ColourRotatorGui::pulserColourChangedData(QColor col)
{
    if (rotator) {
        rotator->pulse_colour =  col;
    }
}

void ColourRotatorGui::pulserHarmonicData(int harmonic)
{
    if (rotator) {
        rotator->pulse_harmonic = harmonic/1000.0;
    }
}

void ColourRotatorGui::pulserPhaseIncData(int adv)
{
    if (rotator) {
        rotator->pulse_phase_advance = adv/1000.0;
    }
}


void ColourRotatorGui::rotatorHarmonicData(int har)
{
    if (rotator) {
        rotator->rotate_harmonic = har/1000.0;

    }
}

void ColourRotatorGui::rotatorPhaseIncrData(int pha)
{
    if (rotator) {
        rotator->rotate_phase_advance = pha/1000.0;
    }
}

void ColourRotatorGui::overrideSwitchData(bool f)
{
    if (rotator) {
        rotator->colour_override = f;
    }
}

void ColourRotatorGui::rotatorBrightModData(bool f)
{
    if (rotator){
	rotator->rotate_v = f;
    }
}


void ColourRotatorGui::rotatorHueModData(bool f)
{
    if (rotator){
	rotator->rotate_h = f;
    }
}


void ColourRotatorGui::rotatorSatModData(bool f)
{
    if (rotator){
	rotator->rotate_s = f;
    }
}


const QIcon* ColourRotatorGui::icon()
{
    return new QIcon(":/icons/yves_guillou_star.svg");
}

ColourRotatorGui::~ColourRotatorGui()
{
}
