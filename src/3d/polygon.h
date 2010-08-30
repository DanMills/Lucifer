#ifndef POLYGON_INC
#define POLYGON_INC

#include <vector>
#include <assert.h>
#include <boost/shared_ptr.hpp>

#include "vector.h"
#include "plane.h"
#include "aabb.h"

static const float EPSILON = 1.0e-6;

//typedef std::vector<Vector3> VertexList;

class Winding
{
public:
    Winding();
    ~Winding();
    int addVertex (const Vector3 &v)
    {
        vertex.push_back(v);
				return vertex.size()-1;
    };
    float area() const;
    Plane3 plane() const;
    Vector3 centre() const;
    void removeCoLinearPoints();
    AABB volume();
    enum CHECK {NO_ERROR,INSUFFICIENT_POINTS,NOT_COPLANAR,DEGENERATE_EDGE,NON_CONVEX};
    CHECK check () const;
    enum PLANESIDE {FRONT=0,BACK=1,ON=2,SPLIT};
    PLANESIDE planeside(const Plane3& plane) const;
    Winding reverse();
    void splitEpsilon (const Plane3 &split, const float epsilon, Winding **front, Winding **back) const;
		Winding * front(const Plane3 &split) const;
		Winding * back(const Plane3 &split) const;

private:
    std::vector<Vector3> vertex;
};

typedef boost::shared_ptr<Winding> WindingPtr;
typedef boost::shared_ptr<const Winding> ConstWindingPtr;

typedef std::vector<WindingPtr> WindingPtrVector;

#endif
