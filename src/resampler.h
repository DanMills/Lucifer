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
