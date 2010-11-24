/* driver_portaudio_ilda.h is part of lucifer a laser show controller.

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

#ifndef DRIVER_PORTAUDIO_ILDA
#define DRIVER_PORTAUDIO_ILDA
#include "driver.h"
#include <portaudio.h>
#include <boost/circular_buffer.hpp>

/// Soundcard DAC using portaudio for ILDA

#define BUFFER_SZ (1024)

int static paCallback (const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData);

class PA_ILDA : public Driver
{
public:
    PA_ILDA();
    ~PA_ILDA();
    Driver::FLAGS flags();
    std::vector<std::string> enumerateHardware();
    bool connect (unsigned int index);
    bool connnected();
    bool disconnect();

    bool ILDAShutter (bool);
    bool ILDAInterlock (bool);
    size_t ILDABufferFillStatus();
    size_t ILDANewPoints (std::vector<PointF> &pts, size_t offset);
    unsigned int ILDAHwPointsPerSecond();
private:
    PaStream *stream;
    std::vector<int> valid_devices;
    unsigned int sr;
    bool shutter;
    unsigned int channels;
    boost::circular_buffer<PointF> *ringbuffer;
    friend int paCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);

};

#endif
