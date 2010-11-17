
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

