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
#include "SceneManager.h"

SceneGroup::SceneGroup(const std::string& name)
{
	m_material = s_defaultMaterial;
	m_name = name;
}

SceneGroup::~SceneGroup()
{

}

void SceneGroup::SetFaces(cl_int3* faces, const int size)
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_faces.assign(faces, faces + size);
}


void SceneGroup::AddFace(const cl_int3& face)
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_faces.push_back(face);
}

void SceneGroup::AddVertex(const cl_float3& vertex)
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_vertices.push_back(vertex);
}


void SceneGroup::SetVertices(const cl_float3* vertices, const int size)
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_vertices.assign(vertices, vertices + size);
}

bool SceneGroup::CheckCorruptedFaces()
{
	int facesAmount = m_faces.size();
	int verticeAmount = m_vertices.size();
	for (int a = 0; a < facesAmount; a++)
	{
		if (m_faces[a].s[0] >= verticeAmount || m_faces[a].s[1] >= verticeAmount || m_faces[a].s[2] >= verticeAmount)
		{
			Log::Error("The group " + m_name + " seems to be indexing inexistent vertices.");
			return false;
		}

	}

	return true;
}
