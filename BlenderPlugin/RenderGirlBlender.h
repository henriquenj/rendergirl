/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2015, Henrique Jung, All rights reserved.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this program.
*/

/* This project links with RenderGirlCore in order to provided a 
	shared library that can be called from python ctypes module.
*/

#ifndef __RENDERGIRL_BLENDER__
#define __RENDERGIRL_BLENDER__


extern "C" // make it callable from Ctypes
{

bool start_rendergirl();
}

#endif //__RENDERGIRL_BLENDER__