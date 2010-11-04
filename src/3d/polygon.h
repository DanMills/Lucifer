#ifndef POLYGON_INC
#define POLYGON_INC

#include <vector>
#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "vector.h"
#include "plane.h"
#include "aabb.h"

static const float EPSILON = 1.0e-6;

struct Vertex {
    Vector3 v;
    unsigned char r,g,b;
};

typedef std::vector<Vertex> vertexList;
typedef std::vector<unsigned int> edgeList;
typedef std::vector<u_int32_t> flagsList;
typedef boost::shared_ptr<vertexList> VertexListPtr;


class Winding
{
public:
    Winding(VertexListPtr  vlist);
    ~Winding();
    void addVertex (const unsigned int v)
    {
        edges.push_back(v);
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
    void splitEpsilon (const Plane3& split, const float epsilon, WindingPtrVector front, WindingPtrVector back) const;
    Winding * front(const Plane3 &split) const;
    Winding * back(const Plane3 &split) const;

private:
    Winding();
    VertexListPtr vertex;
    edgeList edges;
};

typedef boost::shared_ptr<Winding> WindingPtr;
typedef boost::shared_ptr<const Winding> ConstWindingPtr;
typedef std::vector<WindingPtr> WindingPtrVector;

class Object
{
public:
    Object ();
    unsigned int addVertex (const Vector3 &v, const unsigned char r,
                            const unsigned char g, const unsigned char b);

    void addTriangle (const unsigned int a,
                      const unsigned int b,
                      const unsigned int c);
		WindingPtrVector getWindings()
		{
			return windings;
		};
private:
    VertexListPtr vertex;
    WindingPtrVector windings; // A list of polygon windings
};


/// Run the test harness
class polygonTest
{
	public:
		polygonTest();
};


#endif
