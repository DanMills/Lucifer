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

#ifndef FRAMESEQUENCER_INC
#define FRAMESEQUENCER_INC

#include "framesource_impl.h"

class FrameSequencer;

/// GUI for the frame sequencer controls in the editor.
class FrameSequencerGui : public FrameGui
{
Q_OBJECT  
public:
    FrameSequencerGui (FrameSequencer *fs, QWidget *parent);
    virtual ~FrameSequencerGui ();
    const QIcon * icon();
    FrameSequencer *frameseq;
private slots:
    void modeChangedData (int);
    void repeatsChangedData (int);  
};


/// Frame sequencer playback specific data.
class FrameSequencerPlayback : public Playback_impl
{
public:
    FrameSequencerPlayback ();
    unsigned int pos;
    unsigned int repeats_done;
    
    std::vector <unsigned int> index_tbl;
    
};

typedef boost::shared_ptr<FrameSequencerPlayback> FrameSequencerPlaybackPtr;

/// A frame sequencer which is a single FrameSource having many children.
/// It can be set to various step sequence modes and basically hangs on the
/// selected child until it returns a null frame then moves on to the next.

class FrameSequencer : public FrameSource_impl
{
public:
    FrameSequencer ();
    virtual ~FrameSequencer();
    FramePtr nextFrame (PlaybackImplPtr p);
    PlaybackImplPtr newPlayback ();

    void copyDataTo (SourceImplPtr p) const;

    size_t frames ();
    size_t pos(PlaybackImplPtr p);
    void reset(PlaybackImplPtr p);

    void save (QXmlStreamWriter *w);
    void load (QXmlStreamReader *e);
    
    FrameSequencerGui * controls (QWidget *parent);
private:
    enum MODE {Sequential = 0, Random, Pingpong};
    enum MODE mode;
    enum MODE newMode;
    unsigned int repeats;
    
    unsigned int index (FrameSequencerPlaybackPtr pb);
    void reset_index (FrameSequencerPlaybackPtr); 
    
    
    friend class FrameSequencerGui;
};

typedef boost::shared_ptr<FrameSequencer> FrameSequencerPtr;
typedef boost::shared_ptr<const FrameSequencer> ConstFrameSequencerPtr;

#endif
