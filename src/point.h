/*point.h is part of lucifer a laser show controller.

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

#ifndef POINT_INC
#define POINT_INC

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <ostream>
class	Point
{
public:
    unsigned char r;
    unsigned char g;
    unsigned char b;
    bool blanked;

    Point() : v(4) {
        v[3] = 1.0f;
        v[0] = v[1] = v[2] = 0.0f;
        r = g = b = 0.0f;
        blanked = true;
    };
    Point(const Point& p): v(4)
    {
        v = p.v;
        r=p.r;
        g=p.g;
        b=p.b;
        blanked=p.blanked;
    };
    Point(const float x, const float y, const float z) : v(4) {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3]=1.0f;
    };

    inline float x() const {
        return v[0];
    };
    inline float y() const {
        return v[1];
    };
    inline float z() const {
        return v[2];
    };
    inline float w() const {
        return v[3];
    };
    inline void setX(const float f) {
        v[0]=f;
    };
    inline void setY(const float f) {
        v[1]=f;
    };
    inline void setZ(const float f) {
        v[2]=f;
    };
    inline void setW(const float f) {
        v[3]=f;
    };
    inline Point operator-(const Point& rhs) const
    {
        Point res = *this;
        res -= rhs;
        return res;
    }
    inline Point operator+(const Point& rhs) const
    {
        Point res = *this;
        res += rhs;
        return res;
    }
    inline Point &operator+=(const Point& rhs)
    {
        v[0] += rhs.x();
        v[1] += rhs.y();
        v[2] += rhs.z();
        v[3] += rhs.w();
        return *this;
    }
    inline Point &operator-=(const Point& rhs)
    {
        v[0] -= rhs.x();
        v[1] -= rhs.y();
        v[2] -= rhs.z();
        v[3] -= rhs.w();
        return *this;
    }
    inline float operator | (const Point& rhs) const //dot (inner) product
    {
        return x()*rhs.x() + y()*rhs.y() + z()*rhs.z();
    }
    inline Point operator^(const Point& rhs) const // Cross product
    {
        Point res = *this;
        res ^= rhs;
        return res;
    }
    inline Point &operator ^= (const Point& rhs) // cross product
    {//(a2b3 − a3b2, a3b1 − a1b3, a1b2 − a2b1).
        const float x_= y()*rhs.z() - z()*rhs.y();
        const float y_= z()*rhs.x() - x()*rhs.z();
        const float z_= x()*rhs.y() - y()*rhs.x();
        setX(x_);
        setY(y_);
        setZ(z_);
        return *this;
    }
    inline bool operator==(const Point& rhs) const
    {
        if (((x()-rhs.x()) < epsilon) && ((x()-rhs.x()) > -epsilon) &&
                ((y()-rhs.y()) < epsilon) && ((y()-rhs.y()) > -epsilon) &&
                ((z()-rhs.z()) < epsilon) && ((z()-rhs.z()) > -epsilon) &&
                (blanked == rhs.blanked)) {
            return true;
        }
        return false;
    }
    inline bool operator != (const Point &rhs) const
    {
			return !(rhs == *this);
		}
    inline Point operator*(const float scale) const
    {
        Point res = *this;
        res *= scale;
        return res;
    }
    inline Point operator*(const Point& rhs) const
    {
        Point res = *this;
        res *= rhs;
        return res;
    }
    inline Point &operator*=(const float scale)
    {
        setX(x() * scale);
        setY(y() * scale);
        setZ(z() * scale);
        return *this;
    }
    inline Point &operator*=(const Point& rhs)
    {
        setX(x() * rhs.x());
        setY(y() * rhs.y());
        setZ(z() * rhs.z());
        return *this;
    }
    inline Point normalised() const
    {
        Point res = *this;
        float size = 1.0/sqrt(res | res);
        res *= size;
        //res.setW(w()*size);
        return res;
    }
    inline Point &operator *= (const boost::numeric::ublas::matrix <float> mat)
    {
        this->v = boost::numeric::ublas::prod (this->v,mat);
        return *this;
    }
    inline Point operator * (const boost::numeric::ublas::matrix <float> mat)
    {
        Point ret;
        ret.v = boost::numeric::ublas::prod (this->v,mat);
        ret.r = this->r;
        ret.g = this->g;
        ret.b = this->b;
        ret.blanked = this->blanked;
        return ret;
    }
    inline Point &operator /= (const float d)
    {
        assert (d != 0.0f);
        float r = 1.0f/d;
        return *this *=r;
    }

    friend std::ostream&  operator << (std::ostream&  s, const Point & p);

private:
    static const float epsilon = 1.0e-12;
    boost::numeric::ublas::vector<float> v;
};




typedef Point Vector;

#endif
