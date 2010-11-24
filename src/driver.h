/* driver.h is part of lucifer a laser show controller.

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



#ifndef DRIVER_INCL
#define DRIVER_INCL

#include <string>
#include <vector>
#include <qobject.h>
#include "point.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

/// A fully floating point version of a Point, used in the output pipeline for improved
/// precision when doing gamma calculations and the like and to avoid rounding errors in the resampler.


class PointF
{
public:
    float x;
    float y;
    float r;
    float g;
    float b;
};

/// Virtual base class for the hardware IO drivers

/// Note if you claim to support something in flags then you MUST overload the appropriate
/// functions or the base class will throw an assertion failure.

class Driver;

typedef boost::shared_ptr<Driver> DriverPtr;

class Driver : public QObject
{
	Q_OBJECT
public:
    Driver();
    virtual ~Driver();
    enum FLAGS {OUTPUTS_ILDA =1, OUTPUTS_MIDI=2, OUTPUTS_DMX=4, OUTPUTS_AUDIO=8, OUTPUTS_GPO=16, INPUTS_MIDI=32, INPUTS_AUDIO=64, INPUTS_GPI=128, INPUTS_DMX=256};
    virtual Driver::FLAGS flags() = 0;
    /// Enumerate the hardware that this driver has found and can drive.
    /// return a empty list if nothing suitable found.
    virtual std::vector<std::string> enumerateHardware() = 0;
    /// Connect to one of the units in the enumerated list returned by enumerateHardware
    virtual bool connect(unsigned int index) = 0;
    virtual bool connected();
    /// Disconnect from the hardware
    virtual bool disconnect() = 0;

    /// ILDA Output specific functions
    /// Overload if the driver does ILDA.

    /// This opens and closes the projector shutter, shutter open occurs 250ms before output when running
    /// to a time line and closes 250ms after last point.
    virtual bool ILDAShutter(bool);
    /// This can optionally be used to allow the software to control the safety interlock, set true on startup
    /// and false on failure of any of the self tests, assertions and watchdog timeouts.
    virtual bool ILDAInterlock (bool);
    virtual size_t ILDABufferFillStatus();
    /// This is called to add new points to the output.
    virtual size_t ILDANewPoints(std::vector<PointF> &pts, size_t offset);
    /// Called to find out how many points per second the hardware can manage
    virtual unsigned int ILDAHwPointsPerSecond();
    /// Call this from the driver to request the application to supply some more points
    bool ILDARequestMorePoints();

    /// DMX 512 interfacing functions
    /// Write a new block of values to the DMX TX buffer
    virtual void setDMX (std::vector<unsigned char> &dmx);
    /// Read DMX values from an external interface
    virtual std::vector<unsigned char> getDMX();

    /// MIDI port operations
    virtual bool sendMIDI (std::vector<unsigned char> &data);
    virtual std::vector<unsigned char> readMIDI ();

    /// GPIO Pins
    virtual std::vector<bool> readGPI();
    virtual bool setGPO (unsigned int pin, bool state);
    /// How many pins does this interface support?
    virtual unsigned int getGPICount();
    virtual unsigned int getGPOCount();
    /// Audio IO
    virtual std::vector <float> readAudio();
    virtual bool writeAudio (std::vector <float> &samples);
    virtual size_t audioBufferSpaceFrames ();
    virtual size_t audioSampleRate ();
    virtual size_t audioChannels();
    bool audioRequestMoreframes();
    bool audioMoreframesAvailable();

    /// Driver registration and creation methods
    static void registerDriverFactory (const std::string name, DriverPtr (*generator)());
    static DriverPtr newDriver (const std::string name);
    static std::vector<std::string> enemerateDrivers();
    static bool exists (const std::string s);
	signals:
		void ILDARequestMoreData();
		void ILDAHwPPSChanged(unsigned int pps);
		void AudioRequestMoreData();

    /// User configuration options for the drivers
};
#endif
