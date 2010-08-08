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


/* $Id: framesequencer.h 22 2010-07-24 15:01:51Z dmills $ */

#ifndef FRAMESEQUENCER_INC
#define FRAMESEQUENCER_INC

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "framesource.h"

/// GUI for the frame sequencer controls in the editor.
class FrameSequencerGui : public FrameGui
{
public:
    FrameSequencerGui (QWidget *parent);
    ~FrameSequencerGui ();
    const QIcon * icon();
};


/// Frame sequencer playback specific data.
class FrameSequencerPlayback : public PlaybackData
{
public:
    FrameSequencerPlayback () {
        pos = 0;
    }
    unsigned int pos;
};

typedef boost::shared_ptr<FrameSequencerPlayback> FrameSequencerPlaybackPtr;

/// A frame sequencer which is a single FrameSource having many children.
/// It can be set to various step sequence modes and basically hangs on the
/// selected child until it returns a null frame then moves on to the next.

class FrameSequencer : public FrameSource
{
public:
    FrameSequencer ();
    ~FrameSequencer();
    FramePtr data (Playback p);
    PlaybackDataPtr createPlaybackData ();

    size_t frames ();
    size_t pos(Playback p);
    void reset(Playback p);

		void save (QXmlStreamWriter *w);
    void load (QXmlStreamReader *e);

    FrameSequencerGui * controls (QWidget *parent);
private:
    FrameSequencerPlayback * playback(Playback p);
};

typedef boost::shared_ptr<FrameSequencer> FrameSequencerPtr;
typedef boost::shared_ptr<const FrameSequencer> ConstFrameSequencerPtr;




#endif
