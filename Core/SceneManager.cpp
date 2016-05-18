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
#include "BVH.h"


SceneManager::SceneManager()
{
	m_geometryUpdated = false;
	m_materialsUpdated = false;

	m_facesBuffer = nullptr;
	m_groupsBuffer = nullptr;
	m_verticesBuffer = nullptr;
	m_materials = nullptr;
	m_bvhTreeNodes = nullptr;

	m_context = nullptr;
}

SceneManager::~SceneManager()
{
	this->ClearScene();
}

void SceneManager::ClearScene()
{
	/* clear all groups in scene */
	std::vector<SceneGroup*>::iterator it;
	for (it = m_groups.begin(); it != m_groups.end(); it++)
	{
		delete *it;
	}
	m_groups.clear();

	if (m_context != nullptr)
	{
		if (m_facesBuffer != nullptr)
			m_context->DeleteMemoryObject(m_facesBuffer);
		if (m_verticesBuffer != nullptr)
			m_context->DeleteMemoryObject(m_verticesBuffer);
		if (m_groupsBuffer != nullptr)
			m_context->DeleteMemoryObject(m_groupsBuffer);
		if (m_materials != nullptr)
			m_context->DeleteMemoryObject(m_materials);
		if (m_bvhTreeNodes != nullptr)
			m_context->DeleteMemoryObject(m_bvhTreeNodes);
			
	}

	m_facesBuffer = nullptr;
	m_verticesBuffer = nullptr;
	m_groupsBuffer = nullptr;
	m_materials = nullptr;
	m_bvhTreeNodes = nullptr;

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
	m_groups.erase(std::remove(m_groups.begin(), m_groups.end(), group),m_groups.end());

}

void SceneManager::SetContext(const OCLContext* context)
{
	/* changes in the context imply that those memory objects no longer exist */
	m_facesBuffer = nullptr;
	m_verticesBuffer = nullptr;
	m_groupsBuffer = nullptr;
	m_materials = nullptr;
	m_bvhTreeNodes = nullptr;
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
	std::vector<SceneGroup*>::iterator it;
	int groupCount = 0;
	/* check if we need to check the groups for chances in the geometry */
	if (!m_geometryUpdated)
	{
		if (m_facesBuffer != nullptr)
			m_context->DeleteMemoryObject(m_facesBuffer);
		if (m_verticesBuffer != nullptr)
			m_context->DeleteMemoryObject(m_verticesBuffer);
		if (m_groupsBuffer != nullptr)
			m_context->DeleteMemoryObject(m_groupsBuffer);
		if (m_bvhTreeNodes != nullptr)
			m_context->DeleteMemoryObject(m_bvhTreeNodes);

		int facesCount = 0;
		int vertexCount = 0;
		/* covert geometry to global space and query for vertex number and faces number */
		for (it = m_groups.begin(); it != m_groups.end(); it++)
		{
			if ((*it)->AreVerticesInLocalSpace())
			{
				(*it)->TransformLocalToGlobalVertices();
			}

			/* query the groups for face count and vertex count */
			vertexCount += (*it)->GetVerticesNumber();
			facesCount += (*it)->GetFaceNumber();
		}

		/* create the BVHs */
		std::vector<int> objects_index;
		objects_index.reserve(m_groups.size());
		for (int i = 0; i < m_groups.size(); i++)
		{
			/* at the root node, the list of indexes contains 
			 * to the whole list of objects in the scene */
			objects_index.push_back(i);
		}

		BVH root_bvh;
		root_bvh.Create(m_groups, objects_index);

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

		m_bvhTreeNodes = m_context->CreateMemoryObject<BVHTreeNode>(root_bvh.GetNodesAmount(), ReadOnly, &error);
		if (error)
			return false;

		cl_int3* facesRaw = new cl_int3[facesCount];
		cl_float3* vertexRaw = new cl_float3[vertexCount];
		SceneGroupStruct* groupsRaw = new SceneGroupStruct[m_groups.size()];
		BVHTreeNode* bvhTreeNodesRaw = new BVHTreeNode[root_bvh.GetNodesAmount()];

		/* build  traversal array, used for traversal within OpenCL device */
		int offset_traversal = 0;
		root_bvh.BuildTraversal(bvhTreeNodesRaw,offset_traversal);

		int facesOffset = 0;
		int vertexOffset = 0;
		groupCount = 0;

		for (it = m_groups.begin(); it != m_groups.end(); it++, groupCount++)
		{
			/* fill the buffers */
			memcpy(&facesRaw[facesOffset], &((*it)->m_faces[0]), (*it)->GetFaceNumber() * sizeof(cl_int3));
			groupsRaw[groupCount].facesSize = (*it)->GetFaceNumber();
			groupsRaw[groupCount].facesStart = facesOffset;
			groupsRaw[groupCount].vertexSize = (*it)->GetVerticesNumber();
			facesOffset += (*it)->GetFaceNumber();

			memcpy(&vertexRaw[vertexOffset], &((*it)->m_vertices[0]), (*it)->GetVerticesNumber() * sizeof(cl_float3));
			vertexOffset += (*it)->GetVerticesNumber();

		}

		// set date on memory objects
		m_verticesBuffer->SetData(vertexRaw, false);
		m_facesBuffer->SetData(facesRaw, false);
		m_groupsBuffer->SetData(groupsRaw, false);
		m_bvhTreeNodes->SetData(bvhTreeNodesRaw, false);
	}

	if (m_materials != NULL)
		m_context->DeleteMemoryObject(m_materials);
	/* alloc memory dedicated to the materials */
	m_materials = m_context->CreateMemoryObject<Material>(m_groups.size(), ReadOnly);

	// TODO: make the scenemanager be warned in changes in the materials, preventing redundant transfers
	// build material array
	groupCount = 0;
	Material* materials = new Material[m_groups.size()];
	for (it = m_groups.begin(); it != m_groups.end(); it++, groupCount++)
	{
		materials[groupCount] = (*it)->GetMaterial();
	}

	m_materials->SetData(materials, false);

	/* all done, now setup kernel arguments */
	kernel->SetArgument(0, m_verticesBuffer);
	kernel->SetArgument(1, m_facesBuffer);
	kernel->SetArgument(2, m_groupsBuffer);
	kernel->SetArgument(3, m_materials);
	kernel->SetArgument(4, m_bvhTreeNodes);

	m_context->SyncAllMemoryHostToDevice();
	m_geometryUpdated = true;

	return true;
}

void SceneManager::RemoveEmptyGroups()
{
	for (int i = 0; i < m_groups.size(); i++)
	{
		if (m_groups[i]->GetVerticesNumber() == 0 || m_groups[i]->GetFaceNumber() == 0)
		{
			m_groups.erase(m_groups.begin() + i);
			i--;
		}
	}

}