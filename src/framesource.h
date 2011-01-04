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

class Playback
{
public:
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
    void reset ()
    {
        if (frame) {
            frame->reset(pb);
        }
    }
    FramePtr nextFrame()
    {
        if (frame) {
            return frame->nextFrame(pb);
        } else {
            return FramePtr();
        }
    };
    SourceImplPtr getSource()
    {
        return frame;
    }
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
