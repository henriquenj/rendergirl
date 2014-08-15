/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2014, Henrique Jung, All rights reserved.

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


#include "SceneGroup.h"


SceneGroup::SceneGroup()
{
	m_vertices = NULL;
	m_faces = NULL;
	m_isUpdated = false;
	m_verticesSize = 0;
	m_facesSize = 0;
}

SceneGroup::~SceneGroup()
{
	if (m_vertices != NULL)
		delete[] m_vertices;

	if (m_faces != NULL)
		delete[] m_faces;

}

void SceneGroup::SetFaces(cl_int3* faces, const int size, const bool copy)
{
	m_isUpdated = false;
	m_facesSize = size;

	if (m_faces != NULL)
		delete[] m_faces;

	if (copy)
	{
		m_faces = new cl_int3[size];
		memcpy(m_faces, faces, sizeof(cl_int3)* size);
	}
	else
	{
		// copy pointer only
		m_faces = faces;
	}
}

void SceneGroup::SetVertices(cl_float3* vertices, const int size, const bool copy)
{
	m_isUpdated = false;
	m_verticesSize = size;

	if (m_vertices != NULL)
		delete[] m_vertices;

	if (copy)
	{
		m_vertices = new cl_float3[size];
		memcpy(m_vertices, vertices, sizeof(cl_float3)* size);
	}
	else
	{
		// copy pointer only
		m_vertices = vertices;
	}
}