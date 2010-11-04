/*BSPnode.h is part of lucifer a laser show controller.

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

#ifndef BSPNODE_INCLUDED
#define BSPNODE_INCLUDED

#include <boost/shared_ptr.hpp>
#include "aabb.h"
#include "plane.h"
#include "polygon.h"

class BSPNode;
typedef boost::shared_ptr<BSPNode> BSPNodePtr;
typedef boost::shared_ptr<const BSPNode> ConstBSPNodePtr;

/// A Binary space partitioning tree node

class BSPNode
{
public:
    BSPNode();
    ~BSPNode();
		/// Builds a BSP tree for a list of polygons
		static BSPNodePtr BuildBSP (WindingPtrVector polygons);
		/// A world axis aligned bounding box for this node and all children
    AABB bounding_box;
		/// The plane that divides the tree at this level
    Plane3 split_plane;
		/// Polygons that are in front of the split plane are recursvely stored here
    BSPNodePtr front;
		/// Polygons that are behind the split plane are recursively stored here.
    BSPNodePtr back;
		/// In leaf nodes the polygons themselves go here
    WindingPtr winding;
	private:
		/// Returns a 'cost' of using the given polygon as a splitting plane
		static int cost (WindingPtrVector &polygons, ConstWindingPtr splitter);
};

#endif