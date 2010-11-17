/*frameroot.h is part of lucifer a laser show controller.

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

#ifndef FRAMEROOT_INC
#define FRAMEROOT_INC

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "framesource.h"

/// GUI for the FrameRoot controls in the editor.
class FrameRootGui : public FrameGui
{
	public:
		FrameRootGui (QWidget *parent);
		~FrameRootGui ();
		const QIcon * icon();
};
/// This just passed data through unmodified, it has a single child
/// and only exists to ensure that the tree in the editor does not
/// ever have a NULL root. This makes quite a few special cases go away.

class FrameRoot : public FrameSource
{
	public:
		FrameRoot ();
		~FrameRoot();
		FramePtr data (Playback p);
		PlaybackDataPtr createPlaybackData ();

		size_t frames ();
		size_t pos(Playback p);
		void reset(Playback p);
		void copyDataTo(FrameSourcePtr p) const{};

		void save (QXmlStreamWriter *w);
		void load (QXmlStreamReader *e);

		FrameRootGui * controls (QWidget *parent);
};

typedef boost::shared_ptr<FrameRoot> FrameRootPtr;
typedef boost::shared_ptr<const FrameRoot> ConstFrameRootPtr;


#endif
