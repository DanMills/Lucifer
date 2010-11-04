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
