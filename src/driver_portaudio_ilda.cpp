/* driver_portaudio_ilda.cpp is part of lucifer a laser show controller.

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

#include "driver_portaudio_ilda.h"

#include <portaudio.h>
#include <boost/make_shared.hpp>
#include "log.h"
#include <string.h>


/// This bit of bletcherousnes is because portaudio needs to be
/// initialised precisely once....
int pa_usecount = 0;

PA_ILDA::PA_ILDA() : Driver()
{
    /// PortAudio is a single instance library, so we cheat.
    if (pa_usecount == 0) {
        pa_usecount ++;
        PaError err = Pa_Initialize();
        if (err != paNoError) {
            pa_usecount--;
            slog()->errorStream() << "Couldn't initialise portaudio";
        }
    }
    stream = NULL;
    sr = 44100;
    shutter = false;
		ringbuffer = new boost::circular_buffer<PointF>(BUFFER_SZ);
}

PA_ILDA::~PA_ILDA()
{
    if (pa_usecount) {
        pa_usecount --;
    }
    if (!pa_usecount) {
        Pa_Terminate();
    }
    delete ringbuffer;
}

std::vector<std::string> PA_ILDA::enumerateHardware()
{
    std::vector<std::string> res;
    if (!pa_usecount) {
        return res;
    }
    int deviceCount = Pa_GetDeviceCount();
    if (deviceCount < 0) {
        slog()->errorStream() << "Port audio returned negative device count";
        return res;
    }
    for (int i=0; i < deviceCount; i++) {
        const PaDeviceInfo * deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo->maxOutputChannels >=2) {
            res.push_back(std::string(deviceInfo->name));
            valid_devices.push_back(i);
            slog()->infoStream() << "Found device : '" << std::string(deviceInfo->name) << "'";
        }
    }
    slog()->debugStream() <<"Found " << res.size() << " Possible laser output devices";
    return res;
}

bool PA_ILDA::connect(unsigned int index)
{
    if (index >= valid_devices.size()) {
        slog()->errorStream() << "Attempted to connect to invalid portaudio device";
        return false;
    }
    index = valid_devices[index];
    // index is the port audio device to use
    PaStreamParameters outputParameters;
    //PaStreamParameters inputParameters;
		slog()->infoStream() << "Connecting to device : " << std::string(Pa_GetDeviceInfo(index)->name);
		channels = Pa_GetDeviceInfo(index)->maxOutputChannels;
		if (channels > 6) channels = 6; // Pulse audio playing silly buggers most likely
		slog()->infoStream() << "With " << channels << " channels.";

    //memset(&inputParameters, 0,sizeof(inputParameters));
		// Turns out that not asking for any inputs does not work real well
		//inputParameters.channelCount = 2;
    //inputParameters.device = index;
    //inputParameters.hostApiSpecificStreamInfo = NULL;
    //inputParameters.sampleFormat = paFloat32;
    //inputParameters.suggestedLatency = Pa_GetDeviceInfo(index)->defaultLowInputLatency ;
    //inputParameters.hostApiSpecificStreamInfo = NULL;

    memset( &outputParameters, 0, sizeof(outputParameters));
    outputParameters.channelCount = channels;
    outputParameters.device = index;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(index)->defaultLowOutputLatency ;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    /// Find the highest rate this card is capable of.
    unsigned int srates[]={192000,176400,96000,88200,48000,44100,0};
    int j=0;
    PaError err;
    while ((sr = srates[j])) {
        err = Pa_IsFormatSupported(/*&inputParameters*/NULL, &outputParameters,sr);
				slog()->debugStream() << std::string(Pa_GetErrorText(err));
        if (err == paFormatIsSupported) {
            break;
        }
        j++;
    }
    if (err != paFormatIsSupported) {
        slog ()->errorStream() << "Card supports no useful sample rates";
        return false;
    }
    emit ILDAHwPPSChanged(sr);
    slog()->infoStream() << "Card supports " << sr <<" pps";
    // Now set up the callback and start the thing running
    err = Pa_OpenStream(
              &stream,
              NULL/*&inputParameters*/,
              &outputParameters,
              sr,
              256,
              paNoFlag, //flags that can be used to define dither, clip settings and more
              paCallback,
              (void *)this );
    if (err != paNoError) {
        slog()->errorStream() << "Portaudio reported "<< Pa_GetErrorText(err);
        return false;
    }

    return true;
}

bool PA_ILDA::connnected()
{
    return stream ? true : false;
}

unsigned int PA_ILDA::ILDAHwPointsPerSecond()
{
    return sr;
}

Driver::FLAGS PA_ILDA::flags()
{
    return Driver::OUTPUTS_ILDA;
}

bool PA_ILDA::ILDAInterlock(bool state)
{
    return state;
}

bool PA_ILDA::ILDAShutter(bool state)
{
    shutter = state;
    return state;
}

bool PA_ILDA::disconnect()
{
    if (stream) {
        Pa_CloseStream(stream);
        stream = NULL;
        return true;
    }
		return false;
}

size_t PA_ILDA::ILDABufferFillStatus()
{
    return ringbuffer->capacity() - ringbuffer->size();
}

size_t PA_ILDA::ILDANewPoints(std::vector< PointF >& pts, size_t offset)
{
		size_t transfer = pts.size() - offset;
		if (transfer > (ringbuffer->capacity()- ringbuffer->size())){
			transfer = ringbuffer->capacity() - ringbuffer->size();
		}
    for (unsigned i=0; i < transfer; i++) {
        ringbuffer->push_back(pts[i+offset]);
    }
    return transfer;
}

static int paCallback(const void *, void *outputBuffer,
               unsigned long framesPerBuffer,
               const PaStreamCallbackTimeInfo*,
               PaStreamCallbackFlags,
               void *userData )
{
    PA_ILDA *t = (PA_ILDA*) userData;
    float *out = (float*) outputBuffer;
    unsigned int nf = framesPerBuffer;
    if (t->ringbuffer->size() < nf) {
        nf = t->ringbuffer->size();
    }
    for (unsigned int i=0; i < nf; i++) {
        PointF p = t->ringbuffer->front();
        t->ringbuffer->pop_front();
        for (unsigned int j=0; j < t->channels; j++) {
            switch (j) {
            case 0:
                *out = p.x;
                break;
            case 1:
                *out = p.y;
                break;
            case 2:
                *out = p.r;
                break;
            case 3:
                *out = p.g;
                break;
            case 4:
                *out = p.b;
                break;
            case 5: // Intensity - TODO a different matrix here might be better
                *out = (p.r+p.g+p.b)/3.0;
                break;
            default:
                break;
            }
            out ++;
        }
    }
    if (nf < framesPerBuffer) {
        memset(out,0,sizeof(float) * t->channels * (framesPerBuffer - nf));
    }
	return 0;
};

/// This boilerplate registers the driver with the system so that it can appear in menus and the like

static DriverPtr makeSoundCardILDA()
{
	return boost::make_shared<PA_ILDA>();
}

class GenPA_ILDA
{
	public:
		GenPA_ILDA () {
			Driver::registerDriverFactory ("SoundCard (ILDA)",makeSoundCardILDA);
		}
};

static GenPA_ILDA pa_ilda;





