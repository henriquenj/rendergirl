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

private:
	
	SceneManager();
	// prevent copy by not implementing those methods
	SceneManager(SceneManager const&);
	void operator=(SceneManager const&);

	friend class RenderGirlShared;

	/* booleans to control if a given part of the scene is updated with the OpenCL device */
	bool geometryUpdated;
	bool lightUpdated;
	bool materialsUpdated;
};



#endif //__SCENEMANAGERCLASS__