/*frameroot.cpp is part of lucifer a laser show controller.

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

#include <assert.h>
#include <iostream>

#include "framesource.h"
#include "frameroot.h"
#include "log.h"

FrameRoot::FrameRoot () : FrameSource(NOTHING,ONE)
{
	name  = "Frame_Root";
	description = "An invisable node to root the editor tree";
	slog()->debugStream() << "Created new frame root " << this;
}

FrameRoot::~FrameRoot()
{
}

PlaybackDataPtr FrameRoot::createPlaybackData ()
{ // We dont have any state so just return the bass class to keep
	// the management logic happy
	return PlaybackDataPtr(new PlaybackData);
}

void FrameRoot::save (QXmlStreamWriter*)
{ // Should never happen.
	assert (0);
}

void FrameRoot::load(QXmlStreamReader*)
{ // Should never happen.
	assert (0);
}

void FrameRoot::reset (Playback p)
{
	slog()->debugStream() << this << " FrameRoot reset";
	// call the reset method of all the children downtree
	for (unsigned int i=0; i < numChildren(); i++) {
		child(i)->reset(p);
	}
};

FramePtr FrameRoot::data(Playback p)
{
	if (numChildren()){
		return child(0)->data(p);
	}
	return FramePtr();
}


size_t FrameRoot::pos (Playback)
{
	return 0;
}

size_t FrameRoot::frames ()
{
	if (numChildren()){
		return child(0)->frames();
	}
	return 0;
}


// GUI controls from here on down

FrameRootGui * FrameRoot::controls (QWidget *parent)
{
	FrameRootGui *g = new FrameRootGui (parent);
	return g;
}

const QIcon * FrameRootGui::icon()
{
	static const QIcon * ic = new QIcon();
	return ic;
}


FrameRootGui::~FrameRootGui()
{
	slog()->debugStream() << "Deleting FrameRoot GUI "<< this;
}

FrameRootGui::FrameRootGui(QWidget* parent): FrameGui(parent)
{
}
