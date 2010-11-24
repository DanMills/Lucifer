/* colour.h is part of lucifer a laser show controller.

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
#ifndef COLOUR_TRIM_INCL
#define COLOUR_TRIM_INCL

/// Colour adjustments
class ColourTrimmer
{
public:
    ColourTrimmer();
    ~ColourTrimmer();
    inline unsigned char run(const unsigned char col)
    {
        return mapping[col];
    }
    enum WHAT {MIN,MAX,GAMMA};
    void set (enum WHAT what, float value);
private:
    float min;
    float max;
    float gamma;
    unsigned char mapping[256];
    void computeMapping();
};


#endif

