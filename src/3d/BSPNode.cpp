/*BSPNode.cpp is part of lucifer a laser show controller.

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

#include <boost/make_shared.hpp>
#include <assert.h>
#include <limits.h>

#include "../log.h"
#include "BSPnode.h"
#include "frustum.h"

BSPNode::BSPNode()
{
	slog()->debugStream() << "Creating BSPNode : " << this;
}

BSPNode::~BSPNode()
{
	slog()->debugStream() << "Deleting BSPNode : " << this;
}

// Danger O(n)
int BSPNode::cost(WindingPtrVector& polygons, ConstWindingPtr splitter)
{
	unsigned int f,b,s;
	f = b = s = 0;
	Plane3 split = splitter->plane();
	for (unsigned int i=0; i < polygons.size(); i++) {
		if (polygons[i] == splitter) continue;
		switch (polygons[i]->planeside(split)) {
			case Winding::FRONT:
				++f;
				break;
			case Winding::BACK:
				++b;
				break;
			case Winding::SPLIT:
				++s;
			default:
				break;
		}
	}
	// Split polygons are a higher cost then merely an unbalanced tree
	int res =  abs(f-b) + 3 * s;
	slog()->debugStream() << "unbalance " << abs(f-b) << " splits " << s;
	slog()->debugStream() << "cost "<< res;
	return res;
}


// Danger O(n^2 log n), dont do this often.
BSPNodePtr BSPNode::BuildBSP(WindingPtrVector& polygons)
{
	if (polygons.size() == 1){
		slog()->debugStream() << "Creating leaf node for BSP tree";
		BSPNodePtr node = boost::make_shared<BSPNode>();
		node->winding = polygons[0];
		node->bounding_box = polygons[0]->volume();
		return node;
	}
	slog()->debugStream() << "Creating BSP tree from " << polygons.size() <<"Polygons.";
	// First test all polygons to find the best splitter for this level
	// Danger O(n^2)
	int lowest_cost = INT_MAX;
	unsigned int pos = 0;
	for (unsigned int i=0; i < polygons.size(); i++) {
		int c = cost (polygons,polygons[i]);
		if (c < lowest_cost) {
			lowest_cost = c;
			pos = i;
		}
	}
	slog()->debugStream() <<"Lowest cost split is polygon " << pos <<" costing " << lowest_cost;
	// for each polygon in the input vector, split into front or back (possibly splitting the poly
	// in the process
	BSPNodePtr node = boost::make_shared<BSPNode>();
	node->split_plane = polygons[pos]->plane();
	WindingPtrVector bv;
	WindingPtrVector fv;
	for (unsigned int i=polygons.size()-1; i > 0; i--) {
		Winding * b;
		Winding * f;
		polygons[i]->splitEpsilon(node->split_plane,EPSILON,&f,&b);
		if (b) {
			bv.push_back(WindingPtr(b));
		}
		if (f) {
			fv.push_back(WindingPtr(f));
		}
		// Consume the polygons as we go to try to shave some memory usage
		polygons.erase(polygons.begin()+i);
	}
	// Bv and fv are vectors of windings (possibly including split fagments)
	if (!bv.empty()) {
		// we need another node as the back tree.
		slog()->debugStream() << node << " Building back tree";
		node->back = BuildBSP(bv);
		aabb_extend_by_aabb_safe(node->bounding_box,node->back->bounding_box);
	}
	if (!fv.empty()) {
		// we need another node as the front tree.
		slog()->debugStream() << node << " Building front tree";
		node->front = BuildBSP(fv);
		aabb_extend_by_aabb_safe(node->bounding_box,node->front->bounding_box);
	}
	return node;
}


