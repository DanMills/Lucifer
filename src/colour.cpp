
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
