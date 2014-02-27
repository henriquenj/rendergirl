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

//Any change on those structs should be copied back to the device code on Raytracer.cl* /

/* Stores the concept of a Camera */
typedef struct Camera
{
	cl_float3 pos;
	cl_float3 dir;
	cl_float4 screenCoordinates; //defines where the screen begins in world space
	// and some math stuff to help calculate rays
	cl_float delta_x;
	cl_float delta_y;
}Camera;

/* SceneInformation struct holds important information related to the 3D scene and
	how it should be rendered.*/
typedef struct SceneInformation
{
	cl_int resolution;
	cl_int pixelCount;
	cl_int verticesSize;
	cl_int normalSize;
	cl_int facesSize;
} SceneInformation;

/*Struct to control material properties */
typedef struct Material
{
	cl_float3 ambientColor; //KA
	cl_float3 diffuseColor; //KD
	cl_float3 specularColor;//KS
}Material;

/*here ends the cl structures*/

/* file loaders (or other software) should fill this class with 3d information*/
class Scene3D
{
public:

	/* You may notice that this class is a work in progress (all members on public).
	It's suppose to work as a bridge between the 3D softwares and the renderer, so
	I'm only going to fully implement it once I start developing the plugins */
	Scene3D()
	{
		materials = NULL;
	};

	cl_float3* vertices;
	cl_int verticesSize;

	cl_int3* faces; // only triangulated meshs are supported
	cl_int facesSize;

	cl_float3* normal;
	cl_int normalSize;

	Material* materials;
	cl_int materialSize;

	~Scene3D()
	{
		delete vertices;
		delete faces;
		delete normal;
		if (materials != NULL)
			delete materials;
	}
};



#endif //__CLSTRUCTS_HEADER__