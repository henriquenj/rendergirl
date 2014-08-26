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


#ifndef __SCENEMANAGERCLASS__
#define __SCENEMANAGERCLASS__

#include "RenderGirlCore.h"
#include "OBJLoader.h"
#include <list>
#include <assert.h>


class SceneGroup;

/* Singleton class that controls the scene creation and management for the raytracer,
	ultimately converting the scene data into an OpenCL capable format.
	It's duties includes creating and deleting groups, materials, cameras and lights */
class SceneManager
{

/*This singleton architecture was kindly sugested by Loki Astari at
http://stackoverflow.com/questions/270947/can-any-one-provide-me-a-sample-of-singleton-in-c/271104#271104 */

public:
	static SceneManager& GetSharedManager()
	{
		static SceneManager instance;
		return instance;
	}

	~SceneManager();
	
	/* Creates a scene group, you can delete this memory using SceneManager::DeleteSceneGroup.
		For more information about scene groups, check the documentation at SceneGroup.h */
	SceneGroup* CreateSceneGroup(const std::string& name);

	/* Delete a SceneGroup inside this scene. */
	void DeleteSceneGroup(SceneGroup* group);

	/* Load an OBJ file into the scene providing a path, return FALSE if there was an error */
	bool LoadSceneFromOBJ(const std::string& path);

	/* set the scene manager to perform an update on the geometry loaded on the device.
		Called by SceneGroups if there's any changes */
	inline void SetOutadatedGeometry()
	{
		m_geometryUpdated = false;
	}

	/* Remove all the memory associeated with the scene, including all the groups */
	void ClearScene();

	/* Return the amount of scene groups associated with the scene.
		To render a scene, you must have at least one group loaded. */
	inline const int GetGroupsCount()const
	{
		return m_groups.size();
	}
	
private:
	
	SceneManager();
	// prevent copy by not implementing those methods
	SceneManager(SceneManager const&);
	void operator=(SceneManager const&);

	friend class RenderGirlShared;

	/* set the current working context, filled by RenderGirlShared */
	void SetContext(const OCLContext* context);

	/* prepare scene for OpenCL, called by RenderGirlShared, kernel arguments are filled by SceneManager.
		Return false for an error */
	bool PrepareScene(OCLKernel* kernel);

	/* booleans to control if a given part of the scene is updated with the OpenCL device */
	bool m_geometryUpdated;
	bool m_materialsUpdated;

	/* buffers for this scene */
	OCLMemoryObject<cl_int3>* m_facesBuffer;
	OCLMemoryObject<cl_float3>* m_verticesBuffer;
	OCLMemoryObject<SceneGroupStruct>* m_groupsBuffer;
	OCLMemoryObject<Material>* m_materials;

	std::list<SceneGroup*> m_groups;
	
	/* copy of context currently being used, filled by RenderGirlShared upon the first rendering */
	OCLContext* m_context;
};



#endif //__SCENEMANAGERCLASS__