/* resampler.h is part of lucifer a laser show controller.

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

#ifndef RESAMPLE_INCL
#define RESAMPLE_INCL

#include <zita-resampler.h>
#include "driver.h"

class Resample
{
public:
    Resample ();
    ~Resample();
    void setInputPPS(const unsigned int pps);
    void setOutputPPS(const unsigned int pps);

    /// Note this converts from Points to PointF structures as the colour
    /// data may no longer match exact values due to the resampling.
    std::vector<PointF> run (std::vector<Point> &input);
private:
    Resampler resampler;
    unsigned int input_pps;
    unsigned int output_pps;
    void set_resampler();
    float *input_buffer;
    float *output_buffer;
};
#endif
