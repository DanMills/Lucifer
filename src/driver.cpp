/* driver.cpp is part of lucifer a laser show controller.

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

#include "driver.h"
#include "log.h"
#include "assert.h"

#include <map>
#include <vector>

Driver::Driver()
{
    slog()->debugStream() << "Created output driver " << this;
}

Driver::~Driver()
{
    slog()->debugStream() << "Deleted output driver " << this;
}

std::vector< unsigned char> Driver::getDMX()
{
    assert (!(flags() & Driver::INPUTS_DMX));
    std::vector<unsigned char> res;
    return res;
}

void Driver::setDMX(std::vector< unsigned char >&)
{
    assert (!(flags() & Driver::OUTPUTS_DMX));
}

unsigned int Driver::getGPICount()
{
    return 0;
}

unsigned int Driver::getGPOCount()
{
    return 0;
}

std::vector< bool> Driver::readGPI()
{
    assert (!(flags() & Driver::INPUTS_GPI));
    std::vector<bool> res;
    return res;
}

bool Driver::setGPO(unsigned int, bool)
{
    assert (!(flags() & Driver::OUTPUTS_GPO));
    return false;
}

std::vector< unsigned char> Driver::readMIDI()
{
    assert (!(flags() & Driver::INPUTS_MIDI));
    std::vector<unsigned char> res;
    return res;
}

bool Driver::sendMIDI(std::vector<unsigned char>&)
{
    assert (!(flags() & Driver::OUTPUTS_MIDI));
    return false;
}

bool Driver::ILDAShutter(bool )
{
    assert (false);
    return false;
}

bool Driver::ILDAInterlock(bool )
{
    assert (false);
    return false;
}

unsigned int Driver::ILDAHwPointsPerSecond()
{
    assert (false);
    return 0;
}

size_t Driver::ILDABufferFillStatus()
{
    assert (false);
    return 0;
}

size_t Driver::ILDANewPoints(std::vector< PointF >& pts, size_t offset)
{
    assert (false);
    return 0;
}

bool Driver::ILDARequestMorePoints()
{
    emit ILDARequestMoreData();
    return true;
}

std::vector<float> Driver::readAudio()
{
    assert (false);
    std::vector<float> res;
    return res;
}

bool Driver::writeAudio(std::vector< float >&)
{
    assert (false);
    return false;
}

size_t Driver::audioBufferSpaceFrames()
{
    assert (false);
    return 0;
}

size_t Driver::audioChannels()
{
    return 0;
}

size_t Driver::audioSampleRate()
{
    return 0;
}

bool Driver::audioMoreframesAvailable()
{
    assert (false);
    return false;
}

bool Driver::connected()
{
    return false;
}

bool Driver::audioRequestMoreframes()
{ /// TODO
    return true;
}

/// driver registration methods

static std::map <std::string, DriverPtr (*)()> *drivergen = NULL;

void Driver::registerDriverFactory (const std::string name, DriverPtr (*generator)())
{
    assert (generator);
    if (!drivergen) {
        drivergen = new std::map<std::string,DriverPtr(*)()>;
        drivergen->clear();
        slog()->infoStream() <<"Created driver factory mapping :" << drivergen;
    }
    if (drivergen->find(name) != drivergen->end()) {
        slog()->errorStream() << "Attemped to register a non unique frame generator function : " << name;
        assert (0);
    } else {
        (*drivergen)[name] = generator;
        slog()->infoStream() <<"Registered factory for " << name <<" objects : " << (void*)generator;
    }
}

DriverPtr Driver::newDriver (const std::string name)
{
    assert (drivergen);
    std::map<std::string,DriverPtr(*)()>::iterator i;
    i = drivergen->find(name);
    if (i != drivergen->end()) {
        DriverPtr (*fg)() = i->second;
        DriverPtr dr = fg();
        slog()->debugStream() << "Created IO driver " << dr << " as a " << name;
        return dr;
    } else {
        slog()->errorStream() << "Could not find a generator for a " << name << " IO driver";
        return DriverPtr();
    }
}

std::vector<std::string> Driver::enemerateDrivers()
{
    std::vector<std::string> s;
    if (drivergen) {
        for (std::map<std::string,DriverPtr (*)()>::iterator it = drivergen->begin();
                it != drivergen->end(); it++) {
            s.push_back(it->first);
        }
    }
    return s;
}

bool Driver::exists (const std::string s)
{
    assert (drivergen);
    return (drivergen->find(s) != drivergen->end());
}

