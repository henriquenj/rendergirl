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


#include "RenderGirlCore.h"
#include "CL\cl.h"
#include "Log.h"
#include "AABB.h"
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
	void SetFaces(const cl_int3* faces, const int size);

	/* add a face to this group. Must be a combination of three integer pointing to somewhere in the vertex array */
	void AddFace(const cl_int3& face);

	/* add a vertex do this group. */
	void AddVertex(const cl_float3& vertex);

	/* Set vertices on this object. Size is the amount of vertices.
	Each vertice is described as an three floating point values that should be referenced in the faces array.
	Data will be copied, so you are free to use the memory afterwards.
	Peivous loaded data will be deleted. */
	void SetVertices(const cl_float3* vertices, const int size);

	/* 
		Get AABB of this object. If called twice, the result will be cached and no new computing will be performed. 
		Pay attention if the vertices are in local or global mode, which will also apply to the resulting AABB.
	*/
	AABB GetAABB();

	/* Set position on this scene group. It will be applied to all geometry prior rendering. */
	inline void SetPosition(const cl_float3& pos)
	{
		m_pos = pos;
	}

	/* Get position of this SceneGroup */
	inline cl_float3 GetPosision()const
	{
		return m_pos;
	}

	/* Set rotation of this scene group in radians. It will be applied to all geometry prior rendering. */
	inline void SetRotation(const cl_float3& rot)
	{
		m_rotation = rot;
	}

	/* Get rotation of this SceneGroup */
	inline cl_float3 GetRotation()const
	{
		return m_rotation;
	}

	/* Set scale on a 0-1 scale on this scene group. It will be applied to all geometry prior rendering. */
	inline void SetScale(const cl_float3& scale)
	{
		m_scale = scale;
	}

	/* Get scale of this SceneGroup */
	inline cl_float3 GetScale()const
	{
		return m_scale;
	}

	inline std::string GetName()const
	{
		return m_name;
	}

	/* set material on this group */
	inline void SetMaterial(const Material &material)
	{
		m_material = material;
	}

	/* get material associated with this group */
	inline Material GetMaterial()
	{
		return m_material;
	}

	/* Return the amount of faces in this group */
	inline int GetFaceNumber()const
	{
		return m_faces.size();
	}
	/* Return the amount of vertices in this group */
	inline int GetVerticesNumber()const
	{
		return m_vertices.size();
	}

	/* check this group for corrupted indexes (such as faces pointing to non-existent vertices).
		Return true if no corrupted face was found. */
	bool CheckCorruptedFaces();

	/* Apply the scale, position and transform operations on the geometry of this SceneGroup. */
	void TransformLocalToGlobalVertices();

	/* Return true if the vertices are in local space.*/
	inline bool AreVerticesInLocalSpace()
	{
		return m_local_vertices;
	}
private:

	SceneGroup(const std::string& name);
	~SceneGroup();

	/* prevent copy by not implementing this */
	SceneGroup(SceneGroup const&);
	void operator=(SceneGroup const&);

	friend class SceneManager;


	std::string m_name;
	/* geometry associated with this object */
	std::vector<cl_float3> m_vertices;
	std::vector<cl_int3> m_faces;

	/* Global transformations applied to all geometry prior rendering */
	cl_float3 m_pos;
	cl_float3 m_scale;
	cl_float3 m_rotation; /* rotation around itself given an angle expressed in degrees*/

	// local mateiral of this group
	//TODO: move the material to its own class later on
	Material m_material;

	/* tells if the position, scale and rotation have been applied to vertices yet.
		In other words, if the vertices are in local space or global space. */
	bool m_local_vertices;

	/* The AABB of this object which all vertices lie, generated on demand only */
	AABB* m_aabb;
};


#endif //__SCENEGROUPCLASS__