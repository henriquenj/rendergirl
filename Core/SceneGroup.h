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


#ifndef __SCENEGROUPCLASS__
#define __SCENEGROUPCLASS__

#include "CL\cl.h"

/* SceneGroup class handles a given portion of geometry (an object, for instance) and
	can share vertices between faces. Each group can be assotiated with a material */
class SceneGroup
{
public:
	/* return TRUE if this group is updated with the scene manager (inside device memory) */
	inline const bool IsUpdated()const
	{
		return m_isUpdated;
	}
private:

	SceneGroup();
	~SceneGroup();

	/* prevent copy by not implementing this */
	SceneGroup(SceneGroup const&);
	void operator=(SceneGroup const&);

	/* mark this group as updated with SceneManager */
	inline void SetUpdateTrue()
	{
		m_isUpdated = true;
	}

	friend class SceneManager;

	/* geometry associated with this object */
	cl_float3* m_vertices;
	cl_float3* m_faces;
	cl_int m_verticesSize;
	cl_int m_facesSize;
	/* controls if this group is updated with the SceneManager*/
	bool m_isUpdated;
};


#endif //__SCENEGROUPCLASS__