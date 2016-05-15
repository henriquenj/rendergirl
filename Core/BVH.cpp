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
}


BVH::~BVH()
{
	if (m_left)
		delete m_left;
	if (m_right)
		delete m_right;
}

AABB BVH::Create(std::vector<SceneGroup*>& objects, std::vector<int>& objects_index, bool x_split)
{
	/* TODO: recursive create the BVH*/
	return AABB();
}