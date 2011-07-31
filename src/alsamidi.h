/* alsamidi.h is part of lucifer a laser show controller.

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

#ifndef ALSAMIDI_INC
#define ALSAMIDI_INC

#include <qiodevice.h>
#include <alsa/asoundlib.h>
#include <deque>
#include <qsocketnotifier.h>
#include <vector>

class AlsaMidi : public QIODevice
{
Q_OBJECT
public: 
  AlsaMidi (QObject * parent = NULL);
  ~AlsaMidi ();
  bool open (const char *name);
  void close ();
  qint64 readData (char *data, qint64 maxsize);
  qint64 writeData(const char *data, qint64 maxsize);
  bool isSequential () const;
  static std::vector<std::pair<QString,QString> > enumeratePorts();
private:
  snd_rawmidi_t *input;
  snd_rawmidi_t *output;
  std::deque <unsigned char> txqueue;
  std::deque <unsigned char> rxqueue;
  QSocketNotifier **write_fds;
  int num_write_fds;
private slots:
  void readAvailable (int socket);
  void writeAvailable (int socket);
  
  
};

#endif