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

#ifndef __CLSTRUCTS_HEADER__
#define __CLSTRUCTS_HEADER__


#include "CL\cl.h"

//Any change on those structs should be copied back to the device code on Raytracer.cl* /

/* Stores the concept of a Camera. The interface only needs to fill the pos, lookAt and the up information. */
typedef struct Camera
{
	cl_double3 pos;
	cl_double3 dir;
	cl_double3 lookAt;
	cl_double3 up; // upvector
	cl_double3 right;
}Camera;

/* Stores the concept of a light */
typedef struct Light
{
	cl_double3 pos;
	cl_double3 color;

	cl_double Ks; // amount of specular
	cl_double Ka; // amount of ambient
}Light;

/* SceneInformation struct holds important information related to the 3D scene and
	how it should be rendered.*/
typedef struct SceneInformation
{
	cl_int width;
	cl_int height;
	cl_int pixelCount;
	cl_int verticesSize;
	cl_int normalSize;
	cl_int facesSize;
	cl_int materiaslSize;
	cl_double proportion_x;
	cl_double proportion_y;
} SceneInformation;

/*Struct to control material properties */
typedef struct Material
{
	cl_double3 ambientColor; //KA
	cl_double3 diffuseColor; //KD
	cl_double3 specularColor;//KS
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

	cl_double3* vertices;
	cl_int verticesSize;

	cl_int4* faces; // only triangulated meshs are supported, the final component is for the material
	cl_int facesSize;

	cl_double3* normal;
	cl_int normalSize;

	Material* materials;
	cl_int materialSize;

	~Scene3D()
	{
		delete[] vertices;
		delete[] faces;
		delete[] normal;
		if (materials != NULL)
			delete[] materials;
	}
};

/* default material to objects that don't have one */
static const Material s_defaultMaterial = 
{
	{/*ambient(KA)*/ {0.0,0.0,0.0} },
	{/*diffuse(KD)*/{0.5,0.5,0.5} },
	{/*specular(KS)*/{0.0,0.0,0.0} }
};

#endif //__CLSTRUCTS_HEADER__