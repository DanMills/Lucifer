/* resampler.cpp is part of lucifer a laser show controller.

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

#include <boost/math/common_factor_rt.hpp>
#include "resampler.h"

#define RESAMPLE_SZ (1024)

Resample::Resample()
{
    input_pps = output_pps = 30000;
    set_resampler();
    input_buffer = new float[5 * RESAMPLE_SZ];
    output_buffer = new float[5 * RESAMPLE_SZ];
}

Resample::~Resample()
{
    resampler.clear();
    delete[] input_buffer;
    input_buffer = NULL;
    delete[] output_buffer;
    output_buffer = NULL;
}

void Resample::setInputPPS(const unsigned int pps)
{
    input_pps = pps;
    set_resampler();
}

void Resample::setOutputPPS(const unsigned int pps)
{
    output_pps = pps;
    set_resampler();
}

void Resample::set_resampler()
{
    const unsigned int divisor = boost::math::gcd<unsigned int>(input_pps, output_pps);
    resampler.setup(input_pps/divisor,output_pps/divisor,5,16);
}

std::vector<PointF> Resample::run(std::vector<Point>& input)
{
    std::vector<PointF> res;
    size_t remaining_input = input.size();
    unsigned int block_num = 0;
    res.reserve(input.size() * output_pps / input_pps);
    do {
        size_t block = RESAMPLE_SZ;
        if (remaining_input < block) {
            block = remaining_input;
        }
        remaining_input -= block;
        if (block == 0) {
            return res;
        }
        for (unsigned int i=0; i < block; i++) {
            Point &p = input[i + RESAMPLE_SZ * block_num];
            input_buffer[5*i] = p.x();
            input_buffer[5*i+1] = p.y();
            input_buffer[5*i+2] = p.r/255.0f;
            input_buffer[5*i+3] = p.g/255.0f;
            input_buffer[5*i+4] = p.b/255.0f;
            if (p.blanked) {
                input_buffer[5*i+2] =
                    input_buffer[5*i+3] =
                        input_buffer[5*i+4] = 0.0f;
            }
        }
        block_num ++;
        resampler.inp_count = block;
        resampler.inp_data = input_buffer;
        resampler.out_data = output_buffer;
        resampler.out_count = RESAMPLE_SZ;
        while (resampler.inp_count > 0) {
            resampler.process();
            for (unsigned int i=0; i < (RESAMPLE_SZ - resampler.out_count); i++) {
                PointF p;
                p.x = output_buffer[i];
                p.y = output_buffer[i+1];
                p.r = output_buffer[i+2];
                p.g = output_buffer[i+3];
                p.b = output_buffer[i+4];
                res.push_back(p);
            }
            resampler.out_count = 1024;
            resampler.out_data = output_buffer;
        }
    } while (1);
    return res;
}
