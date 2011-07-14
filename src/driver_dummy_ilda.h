/* driver_dummy_ilda.h is part of lucifer a laser show controller.

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
#ifndef DRIVER_DUMMY_ILDA
#define DRIVER_DUMMY_ILDA
#include "driver.h"
#include <qtimer.h>
/// Dummy ILDA output device for preparing shows offline

class Dummy_ILDA : public Driver
{
    Q_OBJECT
public:
    Dummy_ILDA();
    ~Dummy_ILDA();
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
    bool connected_;
    QTimer *timer_;
    size_t buffer_fill_;
private slots:
    void timeout();
};

#endif
