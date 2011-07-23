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
  std::vector<std::pair<QString,QString> > enumeratePorts() const;
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