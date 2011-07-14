/* ALSA Rawmidi driver for the Lucifer laser show controller */

#include "alsamidi.h"
#include "log.h"
#include <qsocketnotifier.h>
#include <poll.h>

AlsaMidi::AlsaMidi(QObject* parent): QIODevice(parent)
{
  input = output = NULL;
  num_write_fds = 0;
  txqueue.clear();
  rxqueue.clear();
}

AlsaMidi::~AlsaMidi()
{
  if (input || output) close();
}

bool AlsaMidi::open(const char* name)
{  
  if (input || output) close();
  QIODevice::open(QIODevice::ReadWrite);
  if(snd_rawmidi_open(&input,&output,name,SND_RAWMIDI_NONBLOCK) <0){
    slog()->errorStream() <<"Error opening rawmidi device";
    QIODevice::close();
    return false;
  }
  slog()->infoStream() <<"Opened rawmidi device";
  // next get the read fds monitored

  int num_read_fds = snd_rawmidi_poll_descriptors_count(input);
  pollfd *pfd = new pollfd[num_read_fds];
  snd_rawmidi_poll_descriptors(input,pfd,num_read_fds);
  int n = 0;
  for (int i=0; i < num_read_fds; i++){
    if (pfd[i].events &POLLIN){
      // set up to watch the device in the main loop
      QSocketNotifier *notifier = new  QSocketNotifier (pfd[i].fd,QSocketNotifier::Read,this);
      connect (notifier,SIGNAL(activated(int)),this,SLOT(readAvailable(int)));
      notifier->setEnabled(true);
      n++;
    }
  }
  delete [] pfd;
  slog()->infoStream() << "Listening for midi on " << n;
  
  // and the write file descriptors also need monitoring
  num_write_fds = snd_rawmidi_poll_descriptors_count(output);
  pfd = new pollfd[num_write_fds];
  snd_rawmidi_poll_descriptors(output,pfd,num_write_fds);
  n=0;
  write_fds = new QSocketNotifier*[num_write_fds];
  for (int i=0; i < num_write_fds; i++){
   if (pfd[i].events & POLLOUT){
      write_fds[n] = new QSocketNotifier(pfd[i].fd,QSocketNotifier::Write,this);
      write_fds[n]->setEnabled(false);
      connect (write_fds[n],SIGNAL(activated(int)),this,SLOT(writeAvailable(int)));
      n++;
   } 
  }
  delete[] pfd;
  num_write_fds = n;
  slog()->infoStream() <<"Monitoring " << n <<" midi TX sockets."; 
  return true;
}

void AlsaMidi::readAvailable(int)
{
  int res = 0;
  unsigned char buf[64];
  while ((res = snd_rawmidi_read(input,buf,64)) > 0){
    for (int n=0; n < res; n++){
      rxqueue.push_back(buf[n]); 
    }
  }
  if (rxqueue.size() > 0) {
    emit readyRead();
  }
}

void AlsaMidi::writeAvailable(int)
{
  int res = 0;
  unsigned char buf[64];
  do {
    int avail = (txqueue.size() > 64) ? 64 : txqueue.size();
    for (int n=0; n < avail; n++){
      buf[n] = txqueue[n];
    }  
    res = snd_rawmidi_write (output,buf,avail);
    if (res > 0){ 
      for (int n = 0; n < res; n++){
	txqueue.pop_front();
      } 
    }
  } while (res > 0);
  bool en = (txqueue.size() !=0);
  // disable the polling if queue empty
  for (int i = 0; i < num_write_fds; i++){
      write_fds[i]->setEnabled(en);
  }
}

void AlsaMidi::close()
{
  if (input) {
    snd_rawmidi_close(input);
    input = NULL;
  }
  if (output){
    snd_rawmidi_close (output);
    output = NULL;
  }
  if (write_fds){
      delete [] write_fds;
      write_fds = NULL;
  }
  num_write_fds = 0;
  txqueue.clear();
  rxqueue.clear();
  QIODevice::close();
}

qint64 AlsaMidi::readData (char* data, qint64 maxsize)
{
   qint64 r = rxqueue.size();
   if ( r > maxsize){
     r = maxsize;
   }
   for (int i=0; i < r; i++){
     data[i] = rxqueue.front();
     rxqueue.pop_front();
   }
   return r;
}

qint64 AlsaMidi::writeData(const char* data, qint64 maxsize)
{
  for (int i=0; i < maxsize; i++)
  {
    txqueue.push_back(data[i]);
    writeAvailable(0);
  }
  return maxsize;
}

bool AlsaMidi::isSequential() const
{
    return true;
}

