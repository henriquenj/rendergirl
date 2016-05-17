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


#ifndef __AABB_CLASS__
#define __AABB_CLASS__

#include <vector>
#include "CL\cl.h"

/* Stores information for describing an axis-aligned bounding box */
class AABB final
{
public:
	AABB();

	/* Constructor that also generated the AABB itself (same as create function) */
	AABB(const std::vector<cl_float3>& vertices);

	~AABB();

	/* Join two AABB together and the resulting AABB can fit both */
	AABB operator+(const AABB& other) const;

	/* given a list of vertices, computes the AABB of the object */
	void Create(const std::vector<cl_float3>& vertices);

	inline cl_float3 GetMaxPoint() const
	{
		return m_point_max;
	}
	inline cl_float3 GetMinPoint() const
	{
		return m_point_min;
	}
	inline cl_float3 GetCenterPoint() const
	{
		return m_center;
	}

private:
	/* the maxium XYZ from all vertices of a given object */
	cl_float3 m_point_min;
	/* the minimum XYZ from all vertices of a given object */
	cl_float3 m_point_max;
	/* the middle point between m_point_max and m_point_min */
	cl_float3 m_center;
	/* compute the m_center for this AABB */
	void ComputeCenter();
};


#endif // __AABB_CLASS__