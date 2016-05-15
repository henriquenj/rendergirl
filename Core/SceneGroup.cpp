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

#include "glm/glm/mat4x4.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtx/transform.hpp"
#include "glm/glm/gtx/quaternion.hpp"

SceneGroup::SceneGroup(const std::string& name)
{
	m_material = s_defaultMaterial;
	m_name = name;

	/* Default transformation matrix */
	m_pos = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	m_scale = { { 1.0f, 1.0f, 1.0f } };
	m_rotation = { { 0.0f, 0.0f, 0.0f } };

	m_local_vertices = true;
}

SceneGroup::~SceneGroup()
{

}

void SceneGroup::SetFaces(const cl_int3* faces, const int size)
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_faces.assign(faces, faces + size);
	m_local_vertices = true;
}


void SceneGroup::AddFace(const cl_int3& face)
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_faces.push_back(face);
}

void SceneGroup::AddVertex(const cl_float3& vertex)
{
	assert(m_local_vertices == true && "You can't add new vertices to a geometry in global space");
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_vertices.push_back(vertex);
}


void SceneGroup::SetVertices(const cl_float3* vertices, const int size)
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetOutadatedGeometry();

	m_vertices.assign(vertices, vertices + size);
	m_local_vertices = true;
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

void SceneGroup::TransformLocalToGlobalVertices()
{
	assert(m_local_vertices == true && "Vertices already in global space");

	/****************************************************************************************
	* Apply all transformations vertex buffer, this will convert all local
	* coordinates of each vertex info from Scenegroup to global coordinates,
	* so we can save these computations on OpenCL device.
	* This code can reduce a lot when there's native support for OpenCL types on GLM
	******************************************************************************************/

	/* build transformation matrix to apply to vertex data, starting with scaling */
	glm::mat4x4 scale = glm::scale(glm::vec3(m_scale.s[0], m_scale.s[1], m_scale.s[2]));
	// rotation using quaternions
	glm::quat rot(glm::vec3(m_rotation.s[0], m_rotation.s[1], m_rotation.s[2]));
	// translation
	glm::mat4x4 translation = glm::translate(glm::vec3(m_pos.s[0], m_pos.s[1], m_pos.s[2]));

	// build transformation matrix
	glm::mat4x4 transform = translation * glm::mat4x4(rot) * scale;

	/* apply object transformations on vertex data */
	for (int i = 0; i < m_vertices.size(); i++)
	{
		/* vertex to be transformed */
		glm::vec4 temp(m_vertices[i].s[0],
				m_vertices[i].s[1],
				m_vertices[i].s[2],
				1.0f /* identity */);

		glm::vec4 translated_vertex = transform * temp;
		m_vertices[i] = { { translated_vertex.x, translated_vertex.y, translated_vertex.z } };
	}

	m_local_vertices = false;
}
