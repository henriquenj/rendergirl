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

#include "BVH.h"


BVH::BVH()
{
	m_left = nullptr;
	m_right = nullptr;
	m_object_index = -1;
	m_x_split = true; /* root node will perform an X split */
	nodes_amount = 1; /* this BVH has at least its own node */
}


BVH::~BVH()
{
	if (m_left)
		delete m_left;
	if (m_right)
		delete m_right;
}

AABB BVH::Create(const std::vector<SceneGroup*>& objects, std::vector<int>& objects_index)
{
	/* 
	
	This function implements the partitioning scheme proposed by Kay and Kajiya
	in their paper "Ray Tracing Complex Scenes" (1986). At each level, objects 
	are sorted either by their X position or Y position. From this sorted list, 
	we split the objects in half, and each of the two subsets of objects are
	send to the child nodes (two new BVH objects) until it reaches a BVH that
	has only one object (a leaf node).

	*/

	assert(objects_index.size() > 0 && "At lest one object is necessary");
	assert(m_left == nullptr && "BVH can only be created once");
	assert(m_right == nullptr && "BVH can only be created once");


	/* if we have more than one object on this vector, it means we are not
		on a leaf node and the BVH contruction must preceed downwards */
	if (objects_index.size() > 1)
	{
		m_left = new BVH();
		m_right = new BVH();

		/* sort objects_index */
		if (m_x_split)
		{
			/* this is a root node or middle node with an X split, 
			 * so we must sort the objects in objects_index vector in X position */
			std::sort(objects_index.begin(), objects_index.end(), 
				[&](const int a, const int b)
			{
				return objects[a]->GetAABB().GetCenterPoint().s[0] < 
					objects[b]->GetAABB().GetCenterPoint().s[0];
			});
			/* next split is an Y split */
			m_left->m_x_split = false;
			m_right->m_x_split = false;
		}
		else
		{
			/* this is a middle node with an Y split,
		     * so we must sort the objects in objects_index vector in Y position */
			std::sort(objects_index.begin(), objects_index.end(),
				[&](const int a, const int b)
			{
				return objects[a]->GetAABB().GetCenterPoint().s[1] <
					objects[b]->GetAABB().GetCenterPoint().s[1];
			});
			/* next split is an X split, so no action must be done on m_left and m_right */
		}

		/* time for split the objects */
		std::size_t half_size = objects_index.size() / 2;
		std::vector<int> left_objects(objects_index.begin(), objects_index.begin() + half_size);
		std::vector<int> right_objects(objects_index.begin() + half_size, objects_index.end());

		/* generate the two child nodes */
		AABB left_aabb;
		AABB right_aabb;

		left_aabb = m_left->Create(objects, left_objects);
		right_aabb = m_right->Create(objects, right_objects);

		/* combining the two childs node AABBs yields an AABB fiting all geometry of this level */
		m_aabb = left_aabb + right_aabb;
		nodes_amount = m_left->nodes_amount + m_right->nodes_amount + 1; /* +1 for this own node */
	}

	else // this is a leaf node, m_object_index is valid
	{
		m_object_index = objects_index[0];
		/* get AABB and return it to parent node */
		m_aabb = objects[m_object_index]->GetAABB();
	}

	return m_aabb;
}

void BVH::BuildTraversal(BVHTreeNode* traversal_array, int& offset)const
{
	assert(traversal_array != nullptr && "Traversal array must be pre-allocated");
	assert((m_left != nullptr || m_object_index != -1) &&
		"Node must be a leaf, root or middle node. BVH was probably not generated");

	/* 
	* This function builds an array of a fixed traversal over this BVH.
	* This implementation is based on the work Thrane and Simonsen
	* in their master thesis "A Comparison of Acceleration Structures 
	* for GPU Assisted Ray Tracing" (2005), specifically the chapter where 
	* they describe traversal code for BVHs on GPUs.
	*/

	int local_offset = offset;
	traversal_array[offset].aabb.point_max = m_aabb.GetMaxPoint();
	traversal_array[offset].aabb.point_min = m_aabb.GetMinPoint();

	if (m_object_index == -1) // this is a root or middle node
	{
		traversal_array[local_offset].packet_indexes.s[1] = -1;

		offset++;/* traversal resumes on the left node */
		m_left->BuildTraversal(traversal_array, offset);

		/* offset was modified by the left branch, now traversing through the right */
		m_right->BuildTraversal(traversal_array, offset);
		
		/* offset after traversing the right branch  is the scape index */
		traversal_array[local_offset].packet_indexes.s[0] = offset;

	}
	else // this is a leaf node
	{
		traversal_array[local_offset].packet_indexes.s[1] = m_object_index;
		traversal_array[local_offset].packet_indexes.s[0] = ++offset; /* scape index */
		/* on leaf node the escape index is always offset + 1, but
		 * we'll store it here for the sake of simplicity */

	}
}