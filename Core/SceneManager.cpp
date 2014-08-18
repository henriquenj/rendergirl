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
#include "OCLContext.h"


SceneManager::SceneManager()
{
	m_geometryUpdated = false;
	m_lightUpdated = false;
	m_materialsUpdated = false;

	m_sceneInfo.facesSize = 0;
	m_sceneInfo.materiaslSize = 0;
	m_sceneInfo.verticesSize = 0;

	m_facesBuffer = NULL;
	m_verticesBuffer = NULL;

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
	}

	m_facesBuffer = NULL;
	m_verticesBuffer = NULL;

	m_geometryUpdated = false;
	m_lightUpdated = false;
	m_materialsUpdated = false;

	m_sceneInfo.facesSize = 0;
	m_sceneInfo.materiaslSize = 0;
	m_sceneInfo.verticesSize = 0;

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
	m_geometryUpdated = false;
	m_lightUpdated = false;
	m_materialsUpdated = false;

	m_context = const_cast<OCLContext*>(context);
}

bool SceneManager::LoadSceneFromOBJ(const std::string& path)
{
	return LoadOBJ(path.c_str());
}