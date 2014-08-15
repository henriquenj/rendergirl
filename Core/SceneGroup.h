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
#include <string.h>

/* SceneGroup class handles a given portion of geometry (an object, for instance) and
	can share vertices between faces. Each group can be assotiated with a material */
class SceneGroup
{
public:

	/* Set faces on this object. Size is the amount of faces.
		Each face is described as an three int values acting as indexes in the
		vertices array. Parameter copy defines if the memory will be copied or just the pointer; if just the
		pointer is copied, SceneGroup will take care of freeing this memory. 
		Peivous loaded data will be deleted. */
	void SetFaces(cl_int3* faces, const int size, const bool copy = true);

	/* Set vertices on this object. Size is the amount of vertices.
	Each vertice is described as an three floating point values that should be referenced in the faces array. 
	Parameter copy defines if the memory will be copied or just the pointer; if just the
	pointer is copied, SceneGroup will take care of freeing this memory.
	Peivous loaded data will be deleted. */
	void SetVertices(cl_float3* vertices, const int size, const bool copy = true);

	/* return TRUE if this group is updated with the scene manager (inside device memory) */
	inline const bool IsUpdated()const
	{
		return m_isUpdated;
	}

	/* Return the amount of faces in this group */
	inline const int GetFaceNumber()const
	{
		return m_facesSize;
	}
	/* Return the amount of vertices in this group */
	inline const int GetVerticesNumber()const
	{
		return m_verticesSize;
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
	cl_int3* m_faces;
	cl_int m_verticesSize;
	cl_int m_facesSize;
	/* controls if this group is updated with the SceneManager*/
	bool m_isUpdated;
};


#endif //__SCENEGROUPCLASS__