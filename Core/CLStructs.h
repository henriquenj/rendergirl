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
	cl_float3 pos;
	cl_float3 dir;
	cl_float3 lookAt;
	cl_float3 up; // upvector
	cl_float3 right;
}Camera;

/* Stores the concept of a light */
typedef struct Light
{
	cl_float3 pos;
	cl_float3 color;

	cl_float Ks; // amount of specular
	cl_float Ka; // amount of ambient
}Light;

/* SceneInformation struct holds important information related to the 3D scene and
	how it should be rendered.*/
typedef struct SceneInformation
{
	cl_int width;
	cl_int height;
	cl_int pixelCount;
	cl_int groupsSize;
	cl_float proportion_x;
	cl_float proportion_y;
} SceneInformation;

/* SceneGroupStruct struct holds info about a particular scene group */
typedef struct SceneGroupStruct
{
	cl_int facesSize; /* amount of faces of this particular group */
	cl_int facesStart;/* the index where the faces of this group start inside the global faces buffer */
	cl_float sphereSize; /* bouding sphere size */
	cl_float3 spherePos; /* bouding sphere position */
}SceneGroupStruct;

/*Struct to control material properties */
typedef struct Material
{
	cl_float3 ambientColor; //KA
	cl_float3 diffuseColor; //KD
	cl_float3 specularColor;//KS
}Material;


/* default material to objects that don't have one */
static const Material s_defaultMaterial = 
{
	{/*ambient(KA)*/ {0.0f,0.0f,0.0f} },
	{/*diffuse(KD)*/{0.5f,0.5f,0.5f} },
	{/*specular(KS)*/{0.0f,0.0f,0.0f} }
};

#endif //__CLSTRUCTS_HEADER__