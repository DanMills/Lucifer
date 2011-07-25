/*framesequencer.h is part of lucifer a laser show controller.

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

#ifndef COLOURROTATOR_INC
#define COLOURROTATOR_INC

#include "framesource_impl.h"
#include "qtcolortriangle.h"

class ColourRotator;

typedef boost::shared_ptr<ColourRotator> ColourRotatorPtr;
typedef boost::shared_ptr<const ColourRotatorPtr> ConstColourRotatorPtr;

/// GUI for the frame colour controls in the editor.
class ColourRotatorGui : public FrameGui
{
  Q_OBJECT
public:
    ColourRotatorGui(ColourRotator *rotator_, QWidget* parent);
    virtual ~ColourRotatorGui();
    const QIcon * icon();
private:
    QDial *pulserHarmonic;
    QDial *pulserPhaseIncr;
    QDial *rotatorHarmonic;
    QDial *rotatorPhaseIncr;
    QCheckBox *hueMod;
    QCheckBox *brightMod;
    QCheckBox *satMod;
    QCheckBox *overrideSwitch;
    QtColorTriangle *overrideColourSelector;
    QtColorTriangle *pulseColourSelector;
    ColourRotator * rotator;
    
private slots:
  void pulserHarmonicData (int);
  void pulserPhaseIncData(int);
  void rotatorHarmonicData(int);
  void rotatorPhaseIncrData (int);
  void rotatorHueModData(bool);  
  void rotatorSatModData(bool);  
  void rotatorBrightModData(bool);  
  
  void overrideSwitchData(bool);
  void overrideColourChangedData (QColor col);
  void pulserColourChangedData (QColor col);
  
};


/// Frame sequencer playback specific data.
class ColourRotatorPlayback : public Playback_impl
{
public:
    ColourRotatorPlayback();
    float pulser_start_phase;
    float rotator_start_phase;
};

typedef boost::shared_ptr<ColourRotatorPlayback> ColourRotatorPlaybackPtr;

/// A Colour rotator that modulates the colours of source frames.
class ColourRotator : public FrameSource_impl
{
public:
    ColourRotator();
    virtual ~ColourRotator();
    FramePtr nextFrame (PlaybackImplPtr p);
    PlaybackImplPtr newPlayback ();

    void copyDataTo (SourceImplPtr p) const;

    size_t frames ();
    size_t pos(PlaybackImplPtr p);
    void reset(PlaybackImplPtr p);

    void save (QXmlStreamWriter *w);
    void load (QXmlStreamReader *e);

    FrameGui* controls (QWidget* parent);
private:
    // colour pulser
    QColor pulse_colour;
    float pulse_dutycycle;
    int pulse_harmonic;
    float pulse_phase_advance;
    //HSV rotator 
    bool rotate_h;
    bool rotate_s;
    bool rotate_v;
    int rotate_harmonic;
    float rotate_phase_advance;
    // Colour override
    bool colour_override;
    QColor override_colour;
    
    void colourPulse (FramePtr frame, ColourRotatorPlaybackPtr pb);
    void HSVRotator (FramePtr frame, ColourRotatorPlaybackPtr pb);
    void colourOverride (FramePtr frame, ColourRotatorPlaybackPtr pb);
    
    float phase_cycle_increment;
    float harmonic;
    friend class ColourRotatorGui;
};

typedef boost::shared_ptr<ColourRotator> ColourRotatorPtr;
typedef boost::shared_ptr<const ColourRotatorPtr> ConstColourRotatorPtr;

#endif
