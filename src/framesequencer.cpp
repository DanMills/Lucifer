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

#include <vector>
#include <assert.h>
#include "framesource_impl.h"
#include <boost/make_shared.hpp>

#include "point.h"
#include "framesequencer.h"
#include "displayframe.h"
#include "log.h"

#define NAME "Frame_sequence"


static SourceImplPtr makeFrameSequencer()
{
    return boost::make_shared<FrameSequencer>();
}

class FrameSequencerGen
{
public:
    FrameSequencerGen () {
        FrameSource_impl::registerFrameGen (NAME,makeFrameSequencer);
    }
};

static FrameSequencerGen fsr;

FrameSequencer::FrameSequencer () : FrameSource_impl (NOTHING,MANY,NAME)
{
    setDescription("A step sequencer for other frame sources");
    slog()->debugStream() << "Created new frame sequencer " << this;
    mode = Sequential;
    repeats = 0;
}

FrameSequencer::~FrameSequencer()
{
}

FrameSequencerPlayback::FrameSequencerPlayback()
{
    pos = 0;
    repeats_done = 0;
}


PlaybackImplPtr FrameSequencer::newPlayback()
{
    return boost::make_shared<FrameSequencerPlayback>();
}

void FrameSequencer::save (QXmlStreamWriter* w)
{
    assert (w);
    w->writeAttribute("Repeats",QString().number(repeats));
    w->writeAttribute("Mode", QString().number((int)mode));
}

void FrameSequencer::load(QXmlStreamReader* r)
{
    assert (r);
    repeats=r->attributes().value("Repeats").toString().toUInt();
    mode = (enum MODE) r->attributes().value("Mode").toString().toUInt();

}

void FrameSequencer::copyDataTo(SourceImplPtr p) const
{
    FrameSequencerPtr fp = boost::dynamic_pointer_cast<FrameSequencer>(p);
    assert (fp);
    fp->mode = mode;
    fp->repeats = repeats;
}

void FrameSequencer::reset (PlaybackImplPtr p)
{
    assert (p);
    FrameSequencerPlaybackPtr pb = boost::dynamic_pointer_cast<FrameSequencerPlayback>(p);
    assert (pb);
    slog()->debugStream() << this << " FrameSequencer reset";
    pb->pos = 0;
    pb->repeats_done = 0;
    reset_index(pb);
};

FramePtr FrameSequencer::nextFrame(PlaybackImplPtr p)
{
    assert (p);
    FrameSequencerPlaybackPtr pb = boost::dynamic_pointer_cast<FrameSequencerPlayback>(p);
    assert (pb);
    FramePtr ps;
    do {
        if (pb->pos >= numChildren()-1) {
            // Run out of data
            if (pb->repeats_done >= repeats) { // finished repeating
                reset (pb);
                return ps;
            } else {
                pb->repeats_done++;
                pb->pos = 0;
                // just reset child nodes
                for (unsigned int i=0; i < numChildren(); i++) {
                    child(i)->reset(pb->child(i));
                }
            }
        }
        ps = child(pb->pos)->nextFrame(pb->child(pb->pos));
        // if the child is still returning data then just pass the pointer up
        if (ps) {
            return ps;
        }
        // else move on to the next step in the sequence (or until we reach the end)
        while (pb->pos <numChildren() -1) {
            // we send the reset signal just before trying to pull data so that dewell
            // times work right
            ++pb->pos;
            child(pb->pos)->reset (pb->child(pb->pos));
            ps = child(pb->pos)->nextFrame(pb->child(pb->pos));
            if (ps) {
                return ps;
            }
        }
    } while (1);
}

void FrameSequencer::reset_index(FrameSequencerPlaybackPtr p)
{
    // fill in the table of indexes for the appropriate mode
    p->index_tbl.resize(numChildren() * (mode==Pingpong) ? 2 :1);
    for (unsigned int i = 0; i < numChildren(); i++){
	p->index_tbl[i] = i;
	if (mode == Pingpong){
	    p->index_tbl[p->index_tbl.size() -i] = i;
	}
    }
    if (mode == Random){
	std::random_shuffle(p->index_tbl.begin(),p->index_tbl.end());
    }
}

unsigned int FrameSequencer::index(FrameSequencerPlaybackPtr pb)
{
    // check that the index_tbl is the right size 
    // a new node could have been added under us
    unsigned int ts = numChildren() * (mode == Pingpong) ? 2 : 1;
    if (ts != pb->index_tbl.size()){
      // something has changed
      reset_index (pb);
    }
    assert (pb->index_tbl.size() == ts);
    return pb->index_tbl[pb->pos];
}





size_t FrameSequencer::pos (PlaybackImplPtr p)
{
    assert (p);
    FrameSequencerPlaybackPtr pb = boost::dynamic_pointer_cast<FrameSequencerPlayback>(p);
    assert (pb);
    return pb->pos;
}

size_t FrameSequencer::frames ()
{
    size_t f = 0;
    for (unsigned int i=0; i < numChildren(); i++) {
        f +=child(i)->frames();
    }
    return f;
}


#if 1
// GUI controls from here on down

FrameSequencerGui * FrameSequencer::controls (QWidget *parent)
{
    FrameSequencerGui *g = new FrameSequencerGui (this, parent);
    return g;
}

const QIcon * FrameSequencerGui::icon()
{
    static const QIcon * ic = new QIcon(":/icons/rotate.svg");
    return ic;
}


FrameSequencerGui::~FrameSequencerGui()
{
    slog()->debugStream() << "Deleting Framesequencer GUI "<< this;
}

FrameSequencerGui::FrameSequencerGui(FrameSequencer *fs ,QWidget* parent): FrameGui(parent)
{
    frameseq = fs;
    assert (frameseq);

    QGridLayout * grid;
    setTitle("Frame Sequencer");
    grid = new QGridLayout (this);
    DisplayFrame *display = new DisplayFrame (this);
    // Step mode switches
    grid->addWidget(display,0,0,1,2);
    //label the number of child sources
    QLabel *childtext = new QLabel(this);
    childtext->setText(tr("Child sources"));
    childtext->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    QLabel * childnum = new QLabel(this);
    childnum->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    childnum->setText(QString().number(frameseq->numChildren()));

    grid->addWidget(childtext,1,0);
    grid->addWidget(childnum,1,1);

    QLabel *modetext = new QLabel(this);
    modetext->setText(tr("Step mode"));
    modetext->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    grid->addWidget(modetext,2,0,1,2);

    QButtonGroup *modegroup = new QButtonGroup (this);
    QRadioButton *sequential = new QRadioButton ("Sequential",this);
    sequential->setChecked(true);
    QRadioButton *random = new QRadioButton ("Random",this);
    QRadioButton *pingpong = new QRadioButton ("Pingpong",this);
    modegroup->addButton(sequential);
    modegroup->setId(sequential,0);
    modegroup->addButton(random);
    modegroup->setId(random,1);
    modegroup->addButton(pingpong);
    modegroup->setId(pingpong,2);
    connect(modegroup,SIGNAL(buttonClicked(int)),this,SLOT(modeChangedData(int)));
    grid->addWidget (sequential,3,0,1,2);
    grid->addWidget (random,4,0,1,2);
    grid->addWidget (pingpong,5,0,1,2);
    switch (frameseq->mode) {
    case FrameSequencer::Sequential :
        sequential->setChecked(true);
        break;
    case FrameSequencer::Random :
        random->setChecked(true);
        break;
    case FrameSequencer::Pingpong :
        pingpong->setChecked(true);
        break;
    }
    // Number of repeats before returning a null to signal move to next
    QLabel *repeatstext = new QLabel(this);
    repeatstext->setText (tr("Repeats"));
    repeatstext->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    QSpinBox *repeatsspin = new QSpinBox(this);
    repeatsspin->setRange(0,1000);
    repeatsspin->setValue(frameseq->repeats);
    connect (repeatsspin,SIGNAL(valueChanged(int)),this,SLOT(repeatsChangedData(int)));
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

void FrameSequencerGui::modeChangedData(int mode)
{
    switch (mode) {
    case 0:
        frameseq->mode = FrameSequencer::Sequential;
        break;
    case 1:
        frameseq->mode = FrameSequencer::Random;
        break;
    case 2:
        frameseq->mode = FrameSequencer::Pingpong;
        break;
    }
}

void FrameSequencerGui::repeatsChangedData(int reps)
{
    frameseq->repeats = reps;
}





#endif
