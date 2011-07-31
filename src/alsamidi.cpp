/* ALSA Rawmidi driver for the Lucifer laser show controller

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
    if (snd_rawmidi_open(&input,&output,name,SND_RAWMIDI_NONBLOCK) <0) {
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
    for (int i=0; i < num_read_fds; i++) {
        if (pfd[i].events &POLLIN) {
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
    for (int i=0; i < num_write_fds; i++) {
        if (pfd[i].events & POLLOUT) {
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
    while ((res = snd_rawmidi_read(input,buf,64)) > 0) {
        for (int n=0; n < res; n++) {
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
        for (int n=0; n < avail; n++) {
            buf[n] = txqueue[n];
        }
        res = snd_rawmidi_write (output,buf,avail);
        if (res > 0) {
            for (int n = 0; n < res; n++) {
                txqueue.pop_front();
            }
        }
    } while (res > 0);
    bool en = (txqueue.size() !=0);
    // disable the polling if queue empty
    for (int i = 0; i < num_write_fds; i++) {
        write_fds[i]->setEnabled(en);
    }
}

void AlsaMidi::close()
{
    if (input) {
        snd_rawmidi_close(input);
        input = NULL;
    }
    if (output) {
        snd_rawmidi_close (output);
        output = NULL;
    }
    if (write_fds) {
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
    if ( r > maxsize) {
        r = maxsize;
    }
    for (int i=0; i < r; i++) {
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

std::vector<std::pair<QString,QString> > AlsaMidi::enumeratePorts()
{
    std::vector<std::pair<QString,QString> > results;
    // iterate over all the sound cards
    slog()->debugStream() <<"Enumerating Midi hardware";
    int card = -1;
    int err;
    if ((err = snd_card_next(&card)) < 0) {
        slog()->errorStream()<< "MIDI:cannot determine card number: " << snd_strerror(err);
        return results;
    }
    if (card < 0) {
        slog()->infoStream() << "MIDI:no sound card found";
        return results;
    }
    do {
        snd_ctl_t *ctl;
        char card_name[32];
        int device;
        int err;

        sprintf(card_name, "hw:%d", card);
        if ((err = snd_ctl_open(&ctl, card_name, 0)) < 0) {
            slog()->errorStream() << "cannot open control for card " << card_name <<" :" <<snd_strerror(err);
            return results;
        }
        slog()->debugStream() << "Enumerating card " << card <<" :: " << card_name;
        device = -1;
        for (;;) {
            if ((err = snd_ctl_rawmidi_next_device(ctl, &device)) < 0) {
                slog()->errorStream()<<"cannot determine device number: "<< snd_strerror(err);
                return results;
            }
            slog()->debugStream() << "Enumerating device " << device;

            if (device < 0) {
                goto NEXT_CARD;
            }
            //list_device(ctl, card, device);
            snd_rawmidi_info_t *info;
            const char *name;
            const char *sub_name;
            int subs, subs_in, subs_out;
            int sub;
            int err;

            snd_rawmidi_info_alloca(&info);
            snd_rawmidi_info_set_device(info, device);

            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
            err = snd_ctl_rawmidi_info(ctl, info);
            if (err >= 0) {
                subs_in = snd_rawmidi_info_get_subdevices_count(info);
            } else {
                subs_in = 0;
            }
            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
            err = snd_ctl_rawmidi_info(ctl, info);
            if (err >= 0) {
                subs_out = snd_rawmidi_info_get_subdevices_count(info);
            } else {
                subs_out = 0;
            }
            // Note we are only interested in devices that can handle both input and output
            subs = subs_in;
            if (subs_out < subs) {
                subs = subs_out;
            }
            if (!subs) {
                continue;;
            }
            for (sub = 0; sub < subs; ++sub) {
                // At this point the first subs subdevices are know to have both input and output
                snd_rawmidi_info_set_stream(info,SND_RAWMIDI_STREAM_INPUT);
                snd_rawmidi_info_set_subdevice(info, sub);
                err = snd_ctl_rawmidi_info(ctl, info);
                if (err < 0) {
                    slog()->errorStream()<<"cannot get rawmidi information" << card <<" " << device << " " << sub << snd_strerror(err);
                    continue;
                }
                name = snd_rawmidi_info_get_name(info);
                sub_name = snd_rawmidi_info_get_subdevice_name(info);
                std::pair <QString,QString> p;
                if (subs == 0 && sub_name[0] == '\0') {
                    p.first = QString().sprintf("hw:%d%d",card,device);
                    p.second = QString().fromUtf8(name);
                    results.push_back(p);
                } else {
                    p.first = QString().sprintf("hw:%d,%d,%d",card,device,sub);
                    p.second = QString().fromUtf8(sub_name);
                    results.push_back(p);
                }
            }

        }
NEXT_CARD:
        snd_ctl_close(ctl);
        if ((err = snd_card_next(&card)) < 0) {
            slog()->errorStream()<< "MIDI cannot determine card number:" <<snd_strerror(err);
            break;
        }
    } while (card >= 0);
    return results;
}























