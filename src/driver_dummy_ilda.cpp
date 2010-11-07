#include "driver_dummy_ilda.h"


#define BUFFER_SZ (2000)


Dummy_ILDA::Dummy_ILDA() : Driver()
{
    connected_ = false;
    buffer_fill_ = 0;
    timer_ = new QTimer(this);
    QObject::connect(timer_,SIGNAL(timeout()),this,SLOT(timeout()));
}

Dummy_ILDA::~Dummy_ILDA()
{
}

bool Dummy_ILDA::connect(unsigned int index)
{
    if (index != 0) {
        return false;
    }
    connected_ = true;
    buffer_fill_ = 0;
    timer_->start(33);
    return true;
}

std::vector< std::string > Dummy_ILDA::enumerateHardware()
{
    std::vector<std::string> res;
    res.push_back(std::string("Dummy ILDA Driver"));
    return res;
}

Driver::FLAGS Dummy_ILDA::flags()
{
    return Driver::OUTPUTS_ILDA;
}

bool Dummy_ILDA::disconnect()
{
    connected_ = false;
    timer_->stop();
    return true;
}

unsigned int Dummy_ILDA::ILDAHwPointsPerSecond()
{
    return 30000;
}

bool Dummy_ILDA::ILDAInterlock(bool )
{
    return true;
}

bool Dummy_ILDA::ILDAShutter(bool )
{
    return true;
}

bool Dummy_ILDA::connnected()
{
    return connected_;
}

size_t Dummy_ILDA::ILDANewPoints(std::vector< PointF >& pts, size_t offset)
{
    size_t space = BUFFER_SZ - buffer_fill_;
    size_t used = pts.size() - offset;
    if (space < used) {
        used = space;
    }
    buffer_fill_ += used;
    return used;
}

size_t Dummy_ILDA::ILDABufferFillStatus()
{
    return BUFFER_SZ - buffer_fill_;
}

void Dummy_ILDA::timeout()
{
    if (buffer_fill_ > 1000) {
        buffer_fill_ -= 1000;
    } else {
        buffer_fill_ = 0;
    }
    ILDARequestMorePoints();
}

/// This boilerplate registers the driver with the system so that it can appear in menus and the like

static DriverPtr makeDummyILDA()
{
    return boost::make_shared<Dummy_ILDA>();
}

class GenDummy_ILDA
{
public:
    GenDummy_ILDA () {
        Driver::registerDriverFactory ("Dummy (ILDA)",makeDummyILDA);
    }
};

static GenDummy_ILDA dummy_ilda;


