/*framesource.h is part of lucifer a laser show controller.

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

#ifndef FRAMESOURCE_INC
#define FRAMESOURCE_INC

#include "framesource_impl.h"
#include "frame.h"

class FrameGui;
class FrameSource;
class Playback;

typedef boost::shared_ptr<Playback> PlaybackPtr;

/// \brief An object which holds the playback and the associated framesource.
class Playback
{
public:
    /// \brief Constructs a Playback appropriate to a given SourceImplPtr
    /// @param [in] fs_ is the shared pointer to the source that is serving as the framesource.
    Playback (SourceImplPtr fs_)
    {
        frame = fs_;
        if (frame) {
            pb = frame->createPlayback();
        } else {
            pb = PlaybackImplPtr();
        }
    }
    ~Playback() {};
    /// \brief Resets the playback by calling the frames reset(pb) method.
    void reset ()
    {
        if (frame) {
            frame->reset(pb);
        }
    }
    /// \brief Returns the next frame.
    /// @return a FramePtr to the next frame.
    FramePtr nextFrame()
    {
        if (frame) {
            return frame->nextFrame(pb);
        } else {
            return FramePtr();
        }
    };
    /// \brief returns the framesource that this playback references.
    /// @return A SourceImplPtr shared pointer to the source this references.
    SourceImplPtr getSource() 
    {
        return frame;
    }
    /// \brief returns the PlaybackImplPtr shared pointer this references.
    /// @return A PlaybackImplPtr shared pointer to the source this references.
    PlaybackImplPtr getPlayback()
    {
        return pb;
    }
private:
    Playback();
    SourceImplPtr frame;
    PlaybackImplPtr pb;
};

#endif
