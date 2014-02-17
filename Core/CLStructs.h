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


/* file loaders (or other software) should fill this struct with 3d information*/
struct Scene3D
{
	cl_float3* vertices;
	cl_int verticesSize;
	
	cl_int3* faces; // only triangulated meshs are supported
	cl_int facesSize;

	cl_float3* normal;
	cl_int normalSize;
};


#endif //__CLSTRUCTS_HEADER__