/*
	RenderGirl - OpenCL raytracer renderer
	Copyright(C) Henrique Jung

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __CLSTRUCTS_HEADER__
#define __CLSTRUCTS_HEADER__


#include "CL\cl.h"


/* file loaders (or other software) should fill this class with 3d information*/
class Scene3D
{
public:

	/* You may notice that this class is a work in progress (all members on public).
		It's suppose to work as a bridge between the 3D softwares and the renderer, so 
		I'm only going to fully implement it once I start developing the plugins */
	Scene3D(){ ; };

	cl_float3* vertices;
	cl_int verticesSize;
	
	cl_int3* faces; // only triangulated meshs are supported
	cl_int facesSize;

	cl_float3* normal;
	cl_int normalSize;

	~Scene3D()
	{
		delete vertices;
		delete faces;
		delete normal;
	}
};

/* SceneInformation struct holds important information related to the 3D scene and
	how it should be rendered.
	Any change on this struct should be copied back to the device code on Raytracer.cl*/
typedef struct SceneInformation
{
	cl_int resolution;
	cl_int pixelCount;
	cl_int verticesSize;
	cl_int normalSize;
	cl_int facesSize;
} SceneInformation;



#endif //__CLSTRUCTS_HEADER__