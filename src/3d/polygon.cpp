#include "../log.h"
#include <limits.h>

#include "polygon.h"

Winding::Winding()
{
    slog()->debugStream() <<"Creating Winding " << this;
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
    for (unsigned int i=2 ; i<vertex.size() ; i++)
    {
        d1=vertex[i-1] - vertex[0];
        d2=vertex[i] - vertex[0];
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
    if (vertex.size() <3) {
        slog()->errorStream() << "Winding : " << this <<
        " Tried to find plane with only "<< vertex.size() <<
        " points.";
    }
    assert (vertex.size() >2);
    p1 = vertex[1] - vertex[0];
    p2 = vertex[2] - vertex[0];
    normal = vector3_cross<float>(p2,p1);
    normal = vector3_normalised<float>(normal);
    dist = vector3_dot<float>(vertex[0],normal);
    return Plane3(normal,dist);
}

Vector3 Winding::centre() const
{
    Vector3 c(0.,0.,0.);
    if (!vertex.size()) {
        return c;
    }
    for (unsigned int i=0 ; i < vertex.size(); i++) {
        c +=vertex[i];
    }
    c/= vertex.size();
    return c;
}

void Winding::removeCoLinearPoints()
{
    Vector3 v1,v2;
    std::vector<Vector3> res;

    for (unsigned int i=0; i < vertex.size(); i++) {
        int j = (i+1)%vertex.size();
        int k = (i+vertex.size()-1)%vertex.size();
        v1 = vertex[j] - vertex[i];
        v2 = vertex[i] - vertex[k];
        v1=vector3_normalised<float>(v1);
        v2=vector3_normalised<float>(v2);
        if (vector3_dot<float>(v1,v2) < 0.999) {
            res.push_back(vertex[i]);
        }
        vertex.clear();
        vertex = res;
    }
}

AABB Winding::volume()
{
    Vector3 min(FLT_MAX,FLT_MAX,FLT_MAX);
    Vector3 max(-FLT_MAX,-FLT_MAX,-FLT_MAX);

    for (unsigned int i=0; i < vertex.size(); i++) {

        if (vertex[i][0] > min[0]) min[0] = vertex[i][0];
        if (vertex[i][1] > min[1]) min[1] = vertex[i][1];
        if (vertex[i][2] > min[2]) min[2] = vertex[i][2];

        if (vertex[i][0] < max[0]) max[0]= vertex[i][0];
        if (vertex[i][1] < max[1]) max[1]= vertex[i][1];
        if (vertex[i][2] < max[2]) max[2]= vertex[i][2];
    }
    return aabb_for_minmax(min, max);
}

Winding Winding::reverse()
{
    Winding w;
    if (vertex.size()) {
        unsigned int i=vertex.size();
        do {
            w.addVertex(vertex[--i]);
				} while (i >0);
		}
    return w;
}

Winding::CHECK Winding::check() const
{
    if (vertex.size() < 3) {
        slog()->errorStream() << "Winding : "<< this << " checked with insufficient points";
        return INSUFFICIENT_POINTS;
    }
    Plane3 pl = plane();
		Vector3 normal = pl.normal();
    for (unsigned int i=0; i < vertex.size(); i++) {
        Vector3 p1 = vertex[i];
        float d = vector3_dot<float>(p1,normal) - pl.dist();
        if (fabs (d) > EPSILON) {
            slog()->errorStream() << "Winding : "<< this << " checked with points that are not coplanar";
            return NOT_COPLANAR;
        }
        int j = ((i+1) == vertex.size() ? 0: i+1);
        Vector3 p2 = vertex[j];
        Vector3 dir = p2 - p1;
        if (vector3_length<float>(dir)<EPSILON) {
            slog()->errorStream() << "Winding : "<< this << " has degenrate edge";
            return DEGENERATE_EDGE;
        }
        Vector3 edgenormal = vector3_cross<float>(normal,dir);
        edgenormal = vector3_normalised<float>(edgenormal);
        float edgedist = vector3_dot<float>(p1,edgenormal) + EPSILON;
        for (unsigned int j=0; i < vertex.size(); j++) {
            if (j==i) continue;
            d=vector3_dot<float>(vertex[j],edgenormal);
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
    for (unsigned int i = 0; i < vertex.size(); i++) {
        float d = vector3_dot<float>(vertex[i],plane.normal()) - plane.dist();
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

void Winding::splitEpsilon (const Plane3 &split, const float epsilon, Winding **front, Winding **back) const
{
    std::vector<float> dists;
    std::vector<enum PLANESIDE> sides;
    int counts[3] = {0};
    dists.reserve (vertex.size()+4);
    sides.reserve (vertex.size()+4);
    // First note what side points are
    for (unsigned int i=0; i < vertex.size(); i++) {
        float dot = vector3_dot<float>(vertex[i],split.normal()) - split.dist();
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
    sides[vertex.size()] = sides[0];
    dists[vertex.size()] = dists[0];
    // Deal with the cases where the polygon is entirely on one side or the other of the split
    *front = *back = NULL;
    if (!counts[FRONT]) {
        // All polys are on the back or co planar
        *back = new Winding;
        **back = *this;
        return;
    }
    if (!counts[BACK]) {
        // All polys are on the front or co planar
        *front = new Winding;
        **front = *this;
        return;
    }
    // Ok it does split.
    Winding *b = *back = new Winding;
    Winding *f = *front = new Winding;
    for (unsigned int i=0; i < vertex.size(); i++) {
        Vector3 p1 = vertex[i];
        if (sides[i] == ON) {
            // On the split plane so copy it down both subtrees
            f->addVertex (p1);
            b->addVertex (p1);
            continue;
        }
        if (sides[i] == FRONT) {
            f->addVertex(p1);
        }
        if (sides[i] == BACK) {
            b->addVertex(p1);
        }
        // If this is not a split, it has been handled, so do the next
        if ((sides[i+i] == sides[i]) || (sides[i+1] = ON))
            continue;
        // Generate a split
        Vector3 p2=vertex[(i+1)%vertex.size()];
        float dot = dists[i]/(dists[i]-dists[i+1]);
        Vector3 mid;
        for (int j=0; j <3; j++) {
            if (split.normal()[j] ==1)
                mid[j] = dists[j];
            else if (split.normal()[j] == -1)
                mid[j]=-dists[j];
            else mid[j]=p1[j]+dot*(p2[j]-p1[j]);
        }
        f->addVertex(mid);
        b->addVertex(mid);
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
