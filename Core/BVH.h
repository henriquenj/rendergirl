/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2016, Henrique Jung, All rights reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library.
*/

#ifndef __BVHCLASS__
#define __BVHCLASS__

#include <vector>

#include "CL\cl.h"
#include "glm\glm\glm.hpp"
#include "AABB.h"
#include "SceneGroup.h"


/* 
	The BVH class will describe a Bounding volume hierarchy as a binary tree. 
	Each instance of this class will build new instances of itself recursively
	provided with a set of objects, until every leaf node has only one object.

	The BVH leaf nodes have an index pointing to an object on the list of objects.
*/
class BVH final
{
public:

	BVH();
	~BVH();

	/*
	Recursevely creates the BVHs
	objects vector contains the pointers to all objects in the scene
	objects_index is a set of indexes pointing to the objects vector that
				  will be split into a new level of the tree
	x_split defines if this split will be on the X or Y axis
	returns an AABB fitting all the geometry of child nodes
	*/
	AABB Create(const std::vector<SceneGroup*>& objects, std::vector<int>& objects_index);

private:

	/* pointers to child nodes, NULL if in a leaf node */
	BVH* m_left;
	BVH* m_right;

	/* The bounding box that fits all geometry of this node and its children */
	AABB m_aabb;
	
	/* Index pointing to an SceneGroup in the list of objects. Only valid on a leaf node */
	int m_object_index;

	/* True if this BVH was sorted in the X coordinated */
	bool m_x_split;
};


#endif // __BVHCLASS__