#include "midi.h"
#include "log.h"

MIDIChannel::MIDIChannel(QObject* parent): QObject(parent)
{
}

MIDIChannel::~MIDIChannel()
{
}

void MIDIChannel::setChannnel(int chan)
{
    channel = chan;
}

void MIDIChannel::send(std::vector<unsigned char> data)
{
    if (data.size()) {
        data[0] &= 0xf0;
        data[0] |= channel;
    }
    emit transmit (data);
}


void MIDIChannel::message(const jdksmidi::MIDIMessage& m)
{
    if (m.IsNoteOn()) emit noteOn(m.GetNote(),m.GetVelocity());
    if (m.IsNoteOff()) emit noteOff(m.GetNote(),m.GetVelocity());
    if (m.IsPolyPressure()) emit aftertouch (m.GetNote(),m.GetVelocity());
    if (m.IsControlChange()) emit controller(m.GetController(),m.GetControllerValue());
    if (m.IsProgramChange()) emit programChange (m.GetPGValue());
    if (m.IsChannelPressure()) emit channelPressure (m.GetChannelPressure());
    if (m.IsPitchBend()) emit pitchWheel (m.GetBenderValue());
}

MIDIParser::MIDIParser(QObject *parent) : QObject(parent), iodevice(NULL)
{
    for (int i=0; i < 16; i++) {
        channels[i].setChannnel(i);
        connect (&channels[i],SIGNAL(transmit(std::vector<unsigned char>)),this,SLOT(send(std::vector<unsigned char>)));
    }
}

MIDIParser::~MIDIParser()
{
}

void MIDIParser::parse(char* data, size_t len)
{
    for (size_t i=0; i < len; i++) {
        if (parser.Parse(data[i],&message)) {
            // Got a complete midi message
            char text[64];
            message.MsgToText(text);
            slog()->debugStream() << "MIDI: " << text;
            if (message.IsChannelMsg()) {
                // message for a specific channel handler
                channels [message.GetChannel()].message(message);
            } else {
                if (message.IsMTC()) emit MTCTick (message.GetByte1());
                if (message.IsSongSelect()) emit songSelect (message.GetByte1());
                if (message.IsSongPosition()) emit songPosition (message.GetBenderValue());
                if (message.IsTuneRequest()) emit tuneRequest ();

                // TODO Add some signals for useful things here
            }
        }
    }
}

void MIDIParser::readData()
{
    slog()->debugStream() << "MIDI : readData";
    char data[64];
    int s = iodevice->read(data,64);
    do {
        if (s < 0) {
            slog()->errorStream() << "Midi Read error";
            return;
        }
        slog()->debugStream() << s <<" Bytes";
        parse (data,s);
        s = iodevice->read(data,64);
    } while (s > 0);
}


void MIDIParser::setQIODevice(QIODevice* io)
{
    iodevice = io;
    if (io) {
        connect (iodevice,SIGNAL(readyRead()),this,SLOT(readData()));
        slog()->debugStream() <<" MIDI - new IO device set";
    }
}

void MIDIParser::send(std::vector<unsigned char> data)
{
    slog()->debugStream() << "MidiParser::send called with " << data.size() <<" bytes";
    if (iodevice) iodevice->write((const char *) &data[0], data.size());
}

