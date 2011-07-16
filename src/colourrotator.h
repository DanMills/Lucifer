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

/// GUI for the frame sequencer controls in the editor.
class ColourRotatorGui : public FrameGui
{
public:
    ColourRotatorGui(QWidget* parent);
    virtual ~ColourRotatorGui();
    const QIcon * icon();
};


/// Frame sequencer playback specific data.
class ColourRotatorPlayback : public Playback_impl
{
public:
    ColourRotatorPlayback();
    float start_phase;
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

    ColourRotatorGui * controls (QWidget *parent);
};

typedef boost::shared_ptr<ColourRotator> ColourRotatorPtr;
typedef boost::shared_ptr<const ColourRotatorPtr> ConstColourRotatorPtr;

#endif
