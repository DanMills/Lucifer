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
