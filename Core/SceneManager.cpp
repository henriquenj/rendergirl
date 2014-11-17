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

#include "SceneManager.h"


SceneManager::SceneManager()
{
	m_geometryUpdated = false;
	m_materialsUpdated = false;

	m_facesBuffer = NULL;
	m_groupsBuffer = NULL;
	m_verticesBuffer = NULL;
	m_materials = NULL;

	m_context = NULL;
}

SceneManager::~SceneManager()
{
	this->ClearScene();
}

void SceneManager::ClearScene()
{
	/* clear all groups in scene */
	std::list<SceneGroup*>::iterator it;
	for (it = m_groups.begin(); it != m_groups.end(); it++)
	{
		delete *it;
	}
	m_groups.clear();

	if (m_context != NULL)
	{
		if (m_facesBuffer != NULL)
			m_context->DeleteMemoryObject(m_facesBuffer);
		if (m_verticesBuffer != NULL)
			m_context->DeleteMemoryObject(m_verticesBuffer);
		if (m_groupsBuffer != NULL)
			m_context->DeleteMemoryObject(m_groupsBuffer);
		if (m_materials != NULL)
			m_context->DeleteMemoryObject(m_materials);
			
	}

	m_facesBuffer = NULL;
	m_verticesBuffer = NULL;
	m_groupsBuffer = NULL;
	m_materials = NULL;

	m_geometryUpdated = false;
	m_materialsUpdated = false;

}

SceneGroup* SceneManager::CreateSceneGroup(const std::string& name)
{
	SceneGroup* newGroup = new SceneGroup(name);
	m_groups.push_back(newGroup);

	return newGroup;
}

void SceneManager::DeleteSceneGroup(SceneGroup* group)
{
	// check if the memory belongs to this context
	assert((std::find(m_groups.begin(), m_groups.end(), group) != m_groups.end())
	&& "This scene group is not part of this scene!");

	delete group;
	m_groups.remove(group);

}

void SceneManager::SetContext(const OCLContext* context)
{
	/* changes in the context imply that those memory objects no longer exist */
	m_facesBuffer = NULL;
	m_verticesBuffer = NULL;
	m_groupsBuffer = NULL;
	m_materials = NULL;
	m_geometryUpdated = false;
	m_materialsUpdated = false;

	m_context = const_cast<OCLContext*>(context);
}

bool SceneManager::LoadSceneFromOBJ(const std::string& path)
{
	return LoadOBJ(path.c_str());
}

bool SceneManager::PrepareScene(OCLKernel* kernel)
{
	assert(m_context != NULL && "Context must be set");
	assert(kernel->GetOk() && "Kernel must be ready");

	/* we have to setup  the 4 first arguments of the kernel: vertices, faces, groups and materials */
	std::list<SceneGroup*>::iterator it;
	int groupCount = 0;
	/* check if we need to check the groups for chances in the geometry */
	if (!m_geometryUpdated)
	{
		if (m_facesBuffer != NULL)
			m_context->DeleteMemoryObject(m_facesBuffer);
		if (m_verticesBuffer != NULL)
			m_context->DeleteMemoryObject(m_verticesBuffer);
		if (m_groupsBuffer != NULL)
			m_context->DeleteMemoryObject(m_groupsBuffer);

		int facesCount = 0;
		int vertexCount = 0;
		/* query the groups for face count and vertex count */
		for (it = m_groups.begin(); it != m_groups.end(); it++)
		{
			vertexCount += (*it)->GetVerticesNumber();
			facesCount += (*it)->GetFaceNumber();
		}

		/* alloc enought memory */
		cl_bool error;

		m_facesBuffer = m_context->CreateMemoryObject<cl_int3>(facesCount, ReadOnly, &error);
		if (error)
			return false;

		m_verticesBuffer = m_context->CreateMemoryObject<cl_float3>(vertexCount, ReadOnly, &error);
		if (error)
			return false;

		m_groupsBuffer = m_context->CreateMemoryObject<SceneGroupStruct>(m_groups.size(), ReadOnly, &error);
		if (error)
			return false;

		cl_int3* facesRaw = new cl_int3[facesCount];
		cl_float3* vertexRaw = new cl_float3[vertexCount];
		SceneGroupStruct* groupsRaw = new SceneGroupStruct[m_groups.size()];
		
		int facesOffset = 0;
		int vertexOffset = 0;
		groupCount = 0;
		/* fill the buffers */
		for (it = m_groups.begin(); it != m_groups.end(); it++, groupCount++)
		{
			memcpy(&facesRaw[facesOffset], &((*it)->m_faces[0]), (*it)->GetFaceNumber() * sizeof(cl_int3));
			groupsRaw[groupCount].facesSize = (*it)->GetFaceNumber();
			groupsRaw[groupCount].facesStart = facesOffset;
			groupsRaw[groupCount].vertexSize = (*it)->GetVerticesNumber();
			facesOffset += (*it)->GetFaceNumber();

			memcpy(&vertexRaw[vertexOffset], &((*it)->m_vertices[0]), (*it)->GetVerticesNumber() * sizeof(cl_float3));
			vertexOffset += (*it)->GetVerticesNumber();
			
		}

		// set date on memroy objects
		m_verticesBuffer->SetData(vertexRaw, false);
		m_facesBuffer->SetData(facesRaw, false);
		m_groupsBuffer->SetData(groupsRaw, false);

	}

	if (m_materials != NULL)
		m_context->DeleteMemoryObject(m_materials);
	/* alloc memory dedicated to the materials */
	m_materials = m_context->CreateMemoryObject<Material>(m_groups.size(), ReadOnly);

	// TODO: make the scenemanager be warned in changes in the materials, preventing redundant transfers
	// build material array
	groupCount = 0;
	Material* materials = new Material[m_groups.size()];
	for (it = m_groups.begin(); it != m_groups.end(); it++,groupCount++)
	{
		materials[groupCount] = (*it)->GetMaterial();
	}

	m_materials->SetData(materials, false);

	/* all done, now setup kernel arguments */
	kernel->SetArgument(0, m_verticesBuffer);
	kernel->SetArgument(1, m_facesBuffer);
	kernel->SetArgument(2, m_groupsBuffer);
	kernel->SetArgument(3, m_materials);

	m_context->SyncAllMemoryHostToDevice();
	m_geometryUpdated = true;
		
	return true;
}

void SceneManager::RemoveEmptyGroups()
{
	std::list<SceneGroup*>::iterator it;
	std::vector<SceneGroup*> toDelete;
	for (it = m_groups.begin(); it != m_groups.end(); it++)
	{
		if ((*it)->GetVerticesNumber() == 0 || (*it)->GetFaceNumber() == 0)
			toDelete.push_back((*it));
	}

	for (int p = 0; p < toDelete.size(); p++)
		m_groups.remove(toDelete[p]);
}