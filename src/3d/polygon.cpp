#include "../log.h"
#include <limits.h>
#include <assert.h>
#include "polygon.h"
#include <boost/make_shared.hpp>
#include "BSPnode.h"

Winding::Winding(VertexListPtr vlist)
{
    slog()->debugStream() <<"Creating Winding " << this;
    vertex = vlist;
}

Winding::~Winding()
{
    slog()->debugStream() << "Deleting Winding " << this;
}

float Winding::area () const
{
    Vector3 d1, d2, cross;
    float total;

    total = 0;
    for (unsigned int i=2 ; i<vertex->size() ; i++)
    {
        d1=(*vertex)[edges[i-1]].v - (*vertex)[edges[0]].v;
        d2=(*vertex)[edges[i]].v - (*vertex)[edges[0]].v;
        cross = vector3_cross<float>(d1,d2);
        total += 0.5 * (cross.x() * cross.x() +
                        cross.y() * cross.y() +
                        cross.z() * cross.z());
    }
    return total;
}

Plane3 Winding::plane() const
{
    Vector3 p1,p2, normal;
    float dist;
    if (edges.size() <3) {
        slog()->errorStream() << "Winding : " << this <<
        " Tried to find plane with only "<< vertex->size() <<
        " points.";
    }
    assert (vertex->size() >2);
    p1 = (*vertex)[edges[1]].v - (*vertex)[edges[0]].v;
    p2 = (*vertex)[edges[2]].v - (*vertex)[edges[0]].v;
    normal = vector3_cross<float>(p2,p1);
		dist = vector3_dot<float>((*vertex)[0].v,normal);
    Plane3 p = Plane3(normal,dist);
		p=plane3_normalised(p);

    //slog()->debugStream() << "Plane normal [ "<< p.a <<","
		//	<< p.b<<"," << p.c <<" ]";
    //slog()->debugStream() << "Plane Distance " << p.d;
    return p;
}

Vector3 Winding::centre() const
{
    Vector3 c(0.,0.,0.);
    if (!edges.size()) {
        return c;
    }
    for (unsigned int i=0 ; i < edges.size(); i++) {
        c +=(*vertex)[edges[i]].v;
    }
    c/= edges.size();
    return c;
}

void Winding::removeCoLinearPoints()
{
    Vector3 v1,v2;
    std::vector<unsigned int> res;

    for (unsigned int i=0; i < edges.size(); i++) {
        int j = (i+1)%edges.size();
        int k = (i+edges.size()-1)%edges.size();
        v1 = (*vertex)[edges[j]].v - (*vertex)[edges[i]].v;
        v2 = (*vertex)[edges[i]].v - (*vertex)[edges[k]].v;
        v1=vector3_normalised<float>(v1);
        v2=vector3_normalised<float>(v2);
        if (vector3_dot<float>(v1,v2) < 0.999) {
            res.push_back(edges[i]);
        }
        edges.clear();
        edges = res;
    }
}

AABB Winding::volume()
{
    Vector3 min(FLT_MAX,FLT_MAX,FLT_MAX);
    Vector3 max(-FLT_MAX,-FLT_MAX,-FLT_MAX);

    for (unsigned int i=0; i < edges.size(); i++) {

        const Vector3 v=(*vertex)[edges[i]].v;

        if (v[0] > min[0]) min[0] = v[0];
        if (v[1] > min[1]) min[1] = v[1];
        if (v[2] > min[2]) min[2] = v[2];

        if (v[0] < max[0]) max[0]= v[0];
        if (v[1] < max[1]) max[1]= v[1];
        if (v[2] < max[2]) max[2]= v[2];
    }
    return aabb_for_minmax(min, max);
}

Winding Winding::reverse()
{
    Winding w(vertex);
    if (edges.size()) {
        unsigned int i=edges.size();
        do {
            w.addVertex(edges[--i]);
        } while (i >0);
    }
    return w;
}

Winding::CHECK Winding::check() const
{
    if (edges.size() < 3) {
        slog()->errorStream() << "Winding : "<< this << " checked with insufficient points";
        return INSUFFICIENT_POINTS;
    }
    Plane3 pl = plane();
    Vector3 normal = pl.normal();
		float distance = pl.dist();
    for (unsigned int i=0; i < edges.size(); i++) {
        assert (edges[i] < (*vertex).size());
        Vector3 p1 = (*vertex)[edges[i]].v;
        float d = fabs(vector3_dot<float>(p1,normal)) - fabs(distance);
        if (fabs (d) > EPSILON) {
            slog()->errorStream() << "Winding : "<< this << " checked with points that are not coplanar";
            slog()->errorStream() << "Dot product = " << vector3_dot<float>(p1,normal);
            slog()->errorStream() << "Distance to plane " << distance;
						for (unsigned int k=0; k < edges.size(); k++){
							Vector3 p = (*vertex)[edges[k]].v;
							slog()->errorStream() << "Point : " <<edges[k]<<" [ " << p.x()<<","<<p.y()<<","<<p.z()<<" ]";
						}
            return NOT_COPLANAR;
        }
        int j = ((i+1) == edges.size() ? 0: i+1);
        assert (edges[j] < (*vertex).size());
        Vector3 p2 = (*vertex)[edges[j]].v;
        Vector3 dir = p2 - p1;
        if (vector3_length<float>(dir)<EPSILON) {
            slog()->errorStream() << "Winding : "<< this << " has degenrate edge";
            return DEGENERATE_EDGE;
        }
        Vector3 edgenormal = vector3_cross<float>(normal,dir);
        edgenormal = vector3_normalised<float>(edgenormal);
        float edgedist = vector3_dot<float>(p1,edgenormal) + EPSILON;
        for (unsigned int j=0; j < edges.size(); j++) {
            if (j==i) continue;
            d=vector3_dot<float>((*vertex)[edges[j]].v,edgenormal);
            if (d > edgedist) {
                slog()->errorStream() << "Winding : "<< this << " is not convex";
                return NON_CONVEX;
            }
        }
    }
    return NO_ERROR;
}

Winding::PLANESIDE Winding::planeside(const Plane3 & plane) const
{
    bool front, back;
    front = back = false;
    for (unsigned int i = 0; i < edges.size(); i++) {
        float d = vector3_dot<float>((*vertex)[edges[i]].v,plane.normal()) - plane.dist();
        if (d < -EPSILON) {
            if (front) {
                return SPLIT;
            }
            back = true;
            continue;
        }
        if (d > EPSILON) {
            if (back) {
                return SPLIT;
            }
            front = true;
            continue;
        }
    }
    if (back) {
        return BACK;
    }
    if (front) {
        return FRONT;
    }
    return ON;
}

void Winding::splitEpsilon (const Plane3 &split, const float epsilon, WindingPtrVector front, WindingPtrVector back) const
{
    std::vector<float> dists;
    std::vector<enum PLANESIDE> sides;
    int counts[3] = {0};
    dists.reserve (edges.size()+4);
    sides.reserve (edges.size()+4);
    // First note what side points are
    for (unsigned int i=0; i < edges.size(); i++) {
        float dot = vector3_dot<float>((*vertex)[edges[i]].v,split.normal()) - split.dist();
        dists[i] = dot;
        if (dot > epsilon) {
            sides[i] = FRONT;
        } else if (dot < -epsilon) {
            sides[i] = BACK;
        } else {
            sides[i] = ON;
        }
        counts[sides[i]]++;
    }
    sides[edges.size()] = sides[0];
    dists[edges.size()] = dists[0];
    // Deal with the cases where the polygon is entirely on one side or the other of the split
    if (!counts[FRONT]) {
        // All polys are on the back or co planar
        back.clear();
        back.push_back(this);
        return;
    }
    if (!counts[BACK]) {
        // All polys are on the front or co planar
        front.clear();
        front.push_back(this);
        return;
    }
    // Ok it does split.
    // Create the new windings for both front and back
    Winding *b = *back = new Winding(vertex);
    Winding *f = *front = new Winding (vertex);
    for (unsigned int i=0; i < edges.size(); i++) {
        Vector3 p1 = (*vertex)[edges[i]].v;
        if (sides[i] == ON) {
            // On the split plane so copy it down both subtrees
            f->addVertex (edges[i]);
            b->addVertex (edges[i]);
            continue;
        }
        if (sides[i] == FRONT) {
            f->addVertex(edges[i]);
        }
        if (sides[i] == BACK) {
            b->addVertex(edges[i]);
        }
        // If this is not a split, it has been handled, so do the next
        if ((sides[i+i] == sides[i]) || (sides[i+1] = ON))
            continue;
        // Generate a split
        Vector3 p2 = (*vertex)[edges[(i+1)%vertex->size()]].v;
        float dot = dists[i]/(dists[i]-dists[i+1]);
        Vertex mid;
        for (int j=0; j <3; j++) {
            if (split.normal()[j] ==1)
                mid.v[j] = dists[j];
            else if (split.normal()[j] == -1)
                mid.v[j]=-dists[j];
            else mid.v[j]=p1[j]+dot*(p2[j]-p1[j]);
        }
        // Add the new point to the master vertex list
        vertex->push_back (mid);
        // TODO - What colour should this be?
        // Add the index to both subtrees
        f->addVertex(vertex->size() - 1);
        b->addVertex(vertex->size() - 1);
    }
}

Winding * Winding::front(const Plane3& split) const
{
    Winding *f;
    Winding *b;
    splitEpsilon(split,EPSILON,&f,&b);
    if (b) {
        delete b;
    }
    return f;
}

Winding * Winding::back(const Plane3& split) const
{
    Winding *f;
    Winding *b;
    splitEpsilon(split,EPSILON,&f,&b);
    if (f) {
        delete f;
    }
    return b;
}


Object::Object ()
{
    slog()->debugStream() << "Created new 3d scene object " << this;
    vertex = boost::make_shared<vertexList>();
    windings.reserve(3);
}

unsigned int Object::addVertex (const Vector3 &v, const unsigned char r,
                                const unsigned char g, const unsigned char b)
{
    Vertex V;
    V.v = v;
    V.r = r;
    V.g = g;
    V.b = b;

    vertex->push_back (V);
    slog()->debugStream() << "Added vertex to list " << this << " now contains "
    << vertex->size() << " verticies";
    return vertex->size() -1;
}


void Object::addTriangle (const unsigned int a,
                          const unsigned int b,
                          const unsigned int c)
{
    assert (vertex.get());
    WindingPtr w = boost::make_shared<Winding>(vertex);
    slog()->debugStream() << "adding triangle to " << this;
    w->addVertex(a);
    w->addVertex(b);
    w->addVertex(c);
#ifndef NDEBUG
    w->check();
#endif
    windings.push_back(w);
}

polygonTest::polygonTest()
{
    slog()->debugStream() << "Starting polygon tests";
    // Build a cube (8 Vertices, 12 triangles)
    Object o;
    o.addVertex (Vector3(-10,-10,10),255,255,255);
    o.addVertex (Vector3(10,-10,10),255,255,255);
    o.addVertex (Vector3(10,-10,-10),255,255,255);
    o.addVertex (Vector3(-10,-10,-10),255,255,255);
    o.addVertex (Vector3(-10,10,10),255,255,255);
    o.addVertex (Vector3(10,10,10),255,255,255);
    o.addVertex (Vector3(10,10,-10),255,255,255);
    o.addVertex (Vector3(-10,10,-10),255,255,255);


    o.addTriangle(0,1,4);
    o.addTriangle(1,5,4);
    o.addTriangle(1,2,5);
    o.addTriangle(2,6,5);
    o.addTriangle(2,3,6);
    o.addTriangle(3,7,6);
    o.addTriangle(3,0,7);
    o.addTriangle(0,4,7);
    o.addTriangle(4,5,7);
    o.addTriangle(5,6,7);
    o.addTriangle(3,2,0);
    o.addTriangle(2,1,0);
		// Created a cube now turn it into a BSP
		BSPNodePtr bsp = BSPNode::BuildBSP(o.getWindings());


}















