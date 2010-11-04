/*geometry.cpp is part of lucifer a laser show controller.

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
#include <QtGui>
#include "frame.h"
#include "assert.h"

/// A simple low frequency oscilator used as part of the Modulator
/// class that is used for dynamic controls
class Lfo
{
	public:
		Lfo();
		enum WAVEFORM {SINE, SQUARE, TRIANGLE, NOISE};
		void setWaveform (enum WAVEFORM w) {
			waveform = w;
		}
		void setFrequency (const float freq, unsigned int sr){
		frequency = freq;
		samplerate = sr;
		}
		void setSync (const bool enable){
			syncEnabled = enable;
		}
		void setDutyCycle (const float d){
			dutyCycle = d;
		}
		void setlevel (const float l){
			level = l;
		};
		void setPhase (const float ph){
			initialPhase = ph;
		};
		void setOffset (const float ofs){

		};
		void sync ();
		void value();

		void load (QXmlStreamReader *e);
		void save (QXmlStreamWriter *w);

	private:
		float frequency;
		unsigned int samplerate;
		bool syncEnabled;
		float dutyCycle;
		float level;
		float initialPhase;
		float offset;
		enum WAVEFORM waveform;
		float phase;
};

Lfo::Lfo()
{
	frequency = 1.0;
	sr = 100;
	syncEnabled = false;
	dutyCycle = 1.0;
	level = 0.0f;
	initialPhase = 0.0f;
	offset = 0.0f;
	phase = 0.0f;
	waveform = WAVEFORM::SINE;
}












/// a simple waveform generator for applying time variant effects
class Modulator
{
	Modulator();
	~Modulator();
























/// A simplistic set of geometry controls suitable for embedding in leaf nodes of the tree.

class geometry
{
	public:
		geometry ();
		~geometry ();

		void load (QXmlStreamReader *e);
		void save (QXmlStreamWriter *w);
		QGroupBox & controls();

	private:
		// Geometry
		float startAngleX;
		float startAngleY;
		float startAngleZ;
		float incrementX;
		float incrementY;
		float incrememtZ;
		float centerX;
		float centerY;
		float centreZ;
		float scaleX;
		float scaleY;
		float scaleZ;
};

class geometryData
{
	public:
		geometryData();
		float angleX;
		float angleY;
		float angleZ;
};

geometry::geometry()
{
	startAngleX = startAngleY = startAngleZ = 0.0;
	incrementX = incrementY = incrementX = 0.0;
	centerX = centerY = centreZ = 0.0f;
	scaleX = scaleY = scaleZ = 0.0f;
}

void geometry::save(QXmlStreamWriter* w)
{
	assert (w);
	w->writeStartElement("Geometry");
	w->writeAttribute("startX",QString().number(startAngleX));
	w->writeAttribute("startY",QString().number(startAngleY));
	w->writeAttribute("startZ",QString().number(startAngleZ));

	w->writeAttribute("incrementX",QString().number(incrementX));
	w->writeAttribute("incrementY",QString().number(incrementY));
	w->writeAttribute("incrementZ",QString().number(incrememtZ));

	w->writeAttribute("centreX",QString().number(centerX));
	w->writeAttribute("centreY",QString().number(centerY));
	w->writeAttribute("centreZ",QString().number(centreZ));

	w->writeAttribute("scaleX",QString().number(scaleX));
	w->writeAttribute("scaleY",QString().number(scaleY));
	w->writeAttribute("scaleZ",QString().number(scaleZ));

	w->writeEndElement();
}




