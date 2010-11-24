/* colour.cpp is part of lucifer a laser show controller.

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
#include <math.h>
#include "colour.h"


ColourTrimmer::ColourTrimmer()
{
	min = 0.0f;
	max = 1.0f;
	gamma = 1.0f;
	for (unsigned int i=0; i < 256; i++){
		mapping[i] = i;
	}
}

ColourTrimmer::~ColourTrimmer()
{
}

void ColourTrimmer::set(ColourTrimmer::WHAT what, float value)
{
	switch (what){
		case ColourTrimmer::MIN :
			min = value;
			break;
		case ColourTrimmer::MAX :
			max = value;
			break;
		case ColourTrimmer::GAMMA :
			gamma = value;
			break;
	}
	computeMapping();
}

void ColourTrimmer::computeMapping()
{
	float range = max - min;
	for (unsigned int i=0; i < 255; i++){
		float v = (i/255.0f);
		v = powf (v,gamma);
		if (v > 1.0f) v = 1.0f;
		v = min + range * v;
		if (v <0) v = 0;
		if (v > 255) v = 255;
		mapping[i] = 255.0 * v;
	}
}
