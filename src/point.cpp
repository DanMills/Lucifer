#include "point.h"

std::ostream&  operator << (std::ostream&  s, const Point & p)
{
	s<< "[" << p.x() << "," <<p.y() << "," << p.z()<<"," << p.w() << "]";
	return s;
}