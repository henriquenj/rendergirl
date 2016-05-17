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

#include <algorithm>

#include "AABB.h"

AABB::AABB()
{
}

AABB::~AABB()
{
}

AABB::AABB(const std::vector<cl_float3>& vertices)
{
	this->Create(vertices);
}

AABB AABB::operator+(const AABB& other) const
{
	AABB result;

	/*
	The greatest and smallest XYZ point of each AABB
	will become the new AABB.
	*/

	result.m_point_max.s[0] = std::max(this->m_point_max.s[0], other.m_point_max.s[0]);
	result.m_point_max.s[1] = std::max(this->m_point_max.s[1], other.m_point_max.s[1]);
	result.m_point_max.s[2] = std::max(this->m_point_max.s[2], other.m_point_max.s[2]);

	result.m_point_min.s[0] = std::min(this->m_point_min.s[0], other.m_point_min.s[0]);
	result.m_point_min.s[1] = std::min(this->m_point_min.s[1], other.m_point_min.s[1]);
	result.m_point_min.s[2] = std::min(this->m_point_min.s[2], other.m_point_min.s[2]);

	/* mid point must be calculated */
	result.ComputeCenter();

	return result;
}

void AABB::Create(const std::vector<cl_float3>& vertices)
{
	float x_max = -std::numeric_limits<float>::max();
	float x_min = std::numeric_limits<float>::max();

	float y_max = -std::numeric_limits<float>::max();
	float y_min = std::numeric_limits<float>::max();

	float z_max = -std::numeric_limits<float>::max();
	float z_min = std::numeric_limits<float>::max();

	/* the AABB is described simply as the maximum and minimum XYZ coordinates
	from the vertices in this object */

	for (int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i].s[0] > x_max)
			x_max = vertices[i].s[0];
		if (vertices[i].s[0] < x_min)
			x_min = vertices[i].s[0];

		if (vertices[i].s[1] > y_max)
			y_max = vertices[i].s[1];
		if (vertices[i].s[1] < y_min)
			y_min = vertices[i].s[1];

		if (vertices[i].s[2] > z_max)
			z_max = vertices[i].s[2];
		if (vertices[i].s[2] < z_min)
			z_min = vertices[i].s[2];
	}

	m_point_max.s[0] = x_max;
	m_point_max.s[1] = y_max;
	m_point_max.s[2] = z_max;

	m_point_min.s[0] = x_min;
	m_point_min.s[1] = y_min;
	m_point_min.s[2] = z_min;

	this->ComputeCenter();
}

void AABB::ComputeCenter()
{
	/* Fill the m_center member using the midpoint formula */
	m_center.s[0] = (m_point_max.s[0] + m_point_min.s[0]) / 2.0f;
	m_center.s[1] = (m_point_max.s[1] + m_point_min.s[1]) / 2.0f;
	m_center.s[2] = (m_point_max.s[2] + m_point_min.s[2]) / 2.0f;
}