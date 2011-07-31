/* midi.h is part of lucifer a laser show controller.

Copyrignt 2011 Dan Mills <dmills@exponent.myzen.co.uk>

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

#ifndef MIDI_INCL
#define MIDI_INCL

#include <jdksmidi/world.h>
#include <jdksmidi/midi.h>
#include <jdksmidi/msg.h>
#include <jdksmidi/sysex.h>
#include <jdksmidi/parser.h>

#include <QtCore>

class MIDIChannel : public QObject
{
  Q_OBJECT;
public:
  MIDIChannel (QObject *parent = NULL);
  virtual ~MIDIChannel();
  void message (const jdksmidi::MIDIMessage &m);
  void setChannnel (int chan);
signals:
  void noteOn (int note, int velocity);
  void noteOff(int note, int velocity);
  void aftertouch (int note, int value);
  void controller (int number, int value);
  void programChange (int value);
  void channelPressure (int value);
  void pitchWheel (int value);
  
  void transmit (std::vector<unsigned char> data);
public slots: 
  void send (std::vector<unsigned char> data);
private:
  int channel;
  
};
  

class MIDIParser : public QObject
{
  Q_OBJECT
public:
  MIDIParser (QObject *parent = NULL);
  virtual ~MIDIParser();
  MIDIChannel channels[16];
  void setQIODevice (QIODevice *io);
private slots:
  void readData();
public slots:
  void send(std::vector <unsigned char> data);
signals:
  void MTCTick (int);
  void songSelect (int);
  void songPosition (int);
  void tuneRequest ();  
private:  
  void parse (char *data, size_t len);
 
  jdksmidi::MIDIParser parser;
  jdksmidi::MIDIMessage message;
  QIODevice *iodevice;
};

#endif 
