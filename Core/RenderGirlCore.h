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


#ifndef __RENDERGIRLCORE_HEADER__
#define __RENDERGIRLCORE_HEADER__

// Master include file for RenderGirlCore


/* Compiler dependent macros (if this gets too long, move to a dedicated header) */
#if defined(_MSC_VER)
#ifdef _M_IX86
	#define RENDERGIRL_ARCH "x86"
#elif _M_X64
	#define RENDERGIRL_ARCH "x64"
#endif
#else
# error Another compiler detect, please include the proper macros
#endif

#define RENDERGIRLCORE_VERSION "0.0.1"
#define RENDERGIRLCORE_COMPILED_DATE __DATE__

/* Core project contains only the core renderer and the OpenCL wrappers.
	Interfaces will be put on another projects */

#include "RenderGirlShared.h"
#include "Log.h"



#endif // __RENDERGIRLCORE_HEADER__