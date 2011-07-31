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

/// \brief A MIDI IO port for ALSA Rawmidi devices under Linux.
class AlsaMidi : public QIODevice
{
Q_OBJECT
public: 
  AlsaMidi (QObject * parent = NULL);
  virtual ~AlsaMidi ();
  /// \brief Opens a rawmidi device read|write.
  /// @param[in] name is the device to open.
  /// @return true on sucess, false on error.
  bool open (const char *name);
  /// \brief Closes a rawmidi device.
  void close ();
  /// \brief Read up to maxsize bytes from the internal buffer into the array pointed to by data.
  /// \param[in] maxisze is the maximum size (in bytes) to return.
  /// \param[out] data is a pointer to the buffer that will contain the data (at least maxsize in length.
  /// \return the number of bytes read.
  qint64 readData (char *data, qint64 maxsize);
  /// \brief Writes data to the midi transmit buffer.
  /// \param[in] data is the data to write to the midi port.
  /// \param[in] maxisize is the size of the data to write to the midi port.
  /// \return the number of bytes written to the TX buffer.
  /// \note This function writes to an internal transmit queue, and does not block.
  qint64 writeData(const char *data, qint64 maxsize);
  bool isSequential () const;
  /// \brief This function enumerates all avallable rawmidi ports.
  /// \return a vector of pairs where pair.first is the hardware name of the port (ex "hw:2:0:0"), and pair.second is the harware name aassigned by the driver (ex. "USB UNO MIDI 1").
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