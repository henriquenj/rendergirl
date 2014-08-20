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
#include "CLStructs.h"
#include "Log.h"
#include <string.h>
#include <string>
#include <vector>

/* SceneGroup class handles a given portion of geometry (an object, for instance) and
	can share vertices between faces. Each group can be assotiated with a material */
class SceneGroup
{
public:

	/* Set faces on this object. Size is the amount of faces.
		Each face is described as an three int values acting as indexes in the
		vertices array. Data will be copied, so you are free to use the memory afterwards.
		Peivous loaded data will be deleted. */
	void SetFaces(cl_int3* faces, const int size);

	/* add a face to this group. Must be a combination of three integer pointing to somewhere in the vertex array */
	inline void AddFace(const cl_int3& face)
	{
		m_faces.push_back(face);
	}

	/* add a vertex do this group. */
	inline void AddVertex(const cl_float3& vertex)
	{
		m_vertices.push_back(vertex);
	}

	/* Set vertices on this object. Size is the amount of vertices.
	Each vertice is described as an three floating point values that should be referenced in the faces array. 
	Data will be copied, so you are free to use the memory afterwards.
	Peivous loaded data will be deleted. */
	void SetVertices(cl_float3* vertices, const int size);

	/* return TRUE if this group is updated with the scene manager (inside device memory) */
	inline const bool IsUpdated()const
	{
		return m_isUpdated;
	}

	inline const std::string& GetName()const
	{
		return m_name;
	}

	/* set material on this group */
	inline void SetMaterial(const Material &material)
	{
		m_material = material;
	}

	/* get material associated with this group */
	inline Material& GetMaterial()
	{
		return m_material;
	}

	/* Return the amount of faces in this group */
	inline const int GetFaceNumber()const
	{
		return m_faces.size();
	}
	/* Return the amount of vertices in this group */
	inline const int GetVerticesNumber()const
	{
		return m_vertices.size();
	}

	/* check this group for corrupted indexes (such as faces pointing to non-existent vertices) */
	bool CheckCorruptedFaces();
private:

	SceneGroup(const std::string& name);
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

	std::string m_name;
	/* geometry associated with this object */
	std::vector<cl_float3> m_vertices;
	std::vector<cl_int3> m_faces;
	/* controls if this group is updated with the SceneManager*/
	bool m_isUpdated;

	// local mateiral of this group
	//TODO: move the material to its own class later on
	Material m_material;
};


#endif //__SCENEGROUPCLASS__