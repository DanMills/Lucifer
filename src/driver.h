#ifndef DRIVER_INCL
#define DRIVER_INCL

#include <string>
#include <vector>
#include "point.h"

/// Virtual base class for the hardware IO drivers

/// Note if you claim to support something in flags then you MUST overload the appropriate
/// functions or the base class will throw an assertion failure.

class Driver
{
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
    virtual size_t ILDANewPoints(std::vector<Point> &pts);
    /// Called to find out how many points per second the hardware can manage
    virtual unsigned int ILDAHwPointsperSecond();
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
};

#endif
