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

ColourRotator::ColourRotator() : FrameSource_impl (NOTHING,ONE,NAME)
{
    setDescription("Colour modulation effect");
    slog()->debugStream() << "Created new colour modulator " << this;
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
    //w->writeAttribute("Repeats",QString().number(1));
    //w->writeAttribute("Mode", "Sequential");
}

void ColourRotator::load(QXmlStreamReader* e)
{
//TODO Load the Repeats and mode data
}

void ColourRotator::copyDataTo(SourceImplPtr p) const
{
// TODO once we have some instance data it will need copying here
}

void ColourRotator::reset(PlaybackImplPtr p)
{
    assert (p);
    ColourRotatorPlaybackPtr pb = boost::dynamic_pointer_cast<ColourRotatorPlayback>(p);
    assert (pb);
    //slog()->debugStream() << this << " FrameSequencer reset";
    //pb->pos = 0;
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
      float phase = 0;
      float phase_increment = 2 * M_PI / ps->getPointCount();
      for (unsigned int i=0; i < ps->getPointCount(); i++){
	Point p = ps->getPoint(i);
	if (!p.blanked){
	  float h = p.hueF();
	  float s = p.saturationF();
	  float v = p.valueF();
	  h+= sin (phase += phase_increment);
	  while (h > 1.0f) h -= 1.0f;
	  while  (h < 0.0f) h += 1.0f;
	  if ((h >= 0.0) && (h <= 1.0) && (v >= 0.0) && (v <= 1.0) && (s >= 0.0) && (s <= 1.0)){
	    p.setHsvF(h,s,v);
	    p.toRgb();
	    ps->setPoint(i,p);
	  } else {
	    slog()->errorStream() << "Out of range " << h <<" : " << s << " : " << v; 
	  }
	}
      }
    }
    return ps;
}
 
size_t ColourRotator::pos(PlaybackImplPtr p)
{
  return 0;
}

size_t ColourRotator::frames()
{
  return numChildren() ? child(0)->frames() : 0;
}

ColourRotatorGui* ColourRotator::controls(QWidget* parent)
{

  return NULL;
}


ColourRotatorPlayback::ColourRotatorPlayback()
{

}



#if 0
// GUI controls from here on down

FrameSequencerGui * FrameSequencer::controls (QWidget *parent)
{
    FrameSequencerGui *g = new FrameSequencerGui (parent);
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
#endif
