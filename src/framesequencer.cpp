/*framesequencer.cpp is part of lucifer a laser show controller.

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

/* $Id: framesequencer.cpp 22 2010-07-24 15:01:51Z dmills $ */

#include <vector>
#include <map>
#include <assert.h>
#include <iostream>
#include <stdio.h>

#include "point.h"
#include "framesource.h"
#include "framesequencer.h"
#include "displayframe.h"
#include "log.h"

static FrameSourcePtr makeFrameSequencer()
{
    return boost::make_shared<FrameSequencer>();
}

class FrameSequencerGen
{
public:
    FrameSequencerGen () {
        FrameSource::registerFrameGen ("Frame_sequence",makeFrameSequencer);
    }
};

static FrameSequencerGen fsr;

FrameSequencer::FrameSequencer () : FrameSource(NOTHING,MANY)
{
    name  = "Frame_sequence";
    description = "A step sequencer for other frame sources";
    slog()->debugStream() << "Created new frame sequencer " << this;
}

FrameSequencer::~FrameSequencer()
{
}

PlaybackDataPtr FrameSequencer::createPlaybackData ()
{
    return PlaybackDataPtr(new FrameSequencerPlayback);
}

void FrameSequencer::save (QXmlStreamWriter* w)
{
    assert (w);
    w->writeAttribute("Repeats",QString().number(1));
    w->writeAttribute("Mode", "Sequential");
}

void FrameSequencer::load(QXmlStreamReader* e)
{
//TODO Load the Repeats and mode data
}

FrameSequencerPlayback * FrameSequencer::playback(Playback p)
{
    FrameSequencerPlayback *pt = (FrameSequencerPlayback*) getPlayback(p);
    return pt;
}

void FrameSequencer::reset (Playback p)
{
    slog()->debugStream() << this << " FrameSequencer reset";
    // call the reset method of all the children downtree
    for (unsigned int i=0; i < numChildren(); i++) {
        child(i)->reset(p);
    }
    // we will need to reset our own state for this playback here
    playback(p)->pos = 0;
};

FramePtr FrameSequencer::data(Playback p)
{

    FramePtr ps;
    if (playback(p)->pos >= numChildren()) {
        // Run out of data
        reset (p);
        return ps;
    }
    ps = child(playback(p)->pos)->data(p);
    // if the child is still returning data then just pass the pointer up
    if (ps) {
        return ps;
    }
    // else move on to the next step in the sequence (or until we reach the end)
    while (playback(p)->pos <numChildren() -1) {
        // we send the reset signal just before trying to pull data so that dewell
        // times work right
        child(++playback(p)->pos)->reset (p);
        ps = child(playback(p)->pos)->data(p);
        if (ps) {
            return ps;
        }
    }
    return ps;
}

size_t FrameSequencer::pos (Playback p)
{
    return playback(p)->pos;
}

size_t FrameSequencer::frames ()
{
    size_t f = 0;
    for (unsigned int i=0; i < numChildren(); i++) {
        f +=child(i)->frames();
    }
    return f;
}


// GUI controls from here on down

FrameSequencerGui * FrameSequencer::controls (QWidget *parent)
{
    FrameSequencerGui *g = new FrameSequencerGui (parent);
    return g;
}

const QIcon * FrameSequencerGui::icon()
{
    static const QIcon * ic = new QIcon();
    return ic;
}


FrameSequencerGui::~FrameSequencerGui()
{
    slog()->debugStream() << "Deleting Framesequencer GUI "<< this;
}

FrameSequencerGui::FrameSequencerGui(QWidget* parent): FrameGui(parent)
{
    QGridLayout * grid;
    setTitle("Frame Sequencer");
    grid = new QGridLayout (this);
    DisplayFrame *display = new DisplayFrame (this);
    // Step mode switches
    grid->addWidget(display,0,0,1,2);

    //label the number of child sources
    QLabel *childtext = new QLabel(this);
    childtext->setText(tr("Child sources"));
    QLabel * childnum = new QLabel(this);
    childnum->setText(QString().number(0));
    grid->addWidget(childtext,1,0);
    grid->addWidget(childnum,1,1);

    QLabel *modetext = new QLabel(this);
    modetext->setText(tr("Step mode"));
    grid->addWidget(modetext,2,0,1,2);

    QButtonGroup *modegroup = new QButtonGroup (this);
    QRadioButton *sequential = new QRadioButton ("Sequential",this);
    sequential->setChecked(true);
    QRadioButton *random = new QRadioButton ("Random",this);
    QRadioButton *pingpong = new QRadioButton ("Pingpong",this);
    modegroup->addButton(sequential);
    modegroup->addButton(random);
    modegroup->addButton(pingpong);
    grid->addWidget (sequential,3,0,1,2);
    grid->addWidget (random,4,0,1,2);
    grid->addWidget (pingpong,5,0,1,2);

    // Number of repeats before returning a null to signal move to next
    QLabel *repeatstext = new QLabel(this);
    repeatstext->setText (tr("Repeats"));
    QSpinBox *repeatsspin = new QSpinBox(this);
    repeatsspin->setRange(1,1000);
    repeatsspin->setValue(1);
    grid->addWidget(repeatstext,6,0);
    grid->addWidget(repeatsspin,6,1);
    // Preview controls
    QPushButton * playbutton = new QPushButton(this);
    playbutton->setText(tr("Run"));
    QPushButton * stopbutton = new QPushButton(this);
    stopbutton->setText(tr("Reset"));
    grid->addWidget(playbutton,7,0);
    grid->addWidget(stopbutton,7,1);

    setLayout(grid);
    slog()->debugStream() << "Created Framesequencer GUI "<< this;
}
