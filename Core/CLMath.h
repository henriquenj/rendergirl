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


#ifndef __CLMATH_HEADER__
#define __CLMATH_HEADER__


#include "CL\cl.h"
#include <math.h>


#define R_PI 3.14159265359
#define R_QUARTER_PI 0.785398163397

/* cl math header contains math functions that receives the cl types as arguments, mimicking their OpenCL C counterparts */


cl_float3 cross(const cl_float3& a, const cl_float3& b)
{
	cl_float3 result;

	result.s[0] = (a.s[1] * b.s[2]) - (a.s[2] * b.s[1]);
	result.s[1] = (a.s[2] * b.s[0]) - (a.s[0] * b.s[2]);
	result.s[2] = (a.s[0] * b.s[1]) - (a.s[1] * b.s[0]);

	return result;
}

cl_float3 subtract(const cl_float3& a, const cl_float3& b)
{
	cl_float3 result;

	result.s[0] = a.s[0] - b.s[0];
	result.s[1] = a.s[1] - b.s[1];
	result.s[2] = a.s[2] - b.s[2];

	return result;
}

inline float length(const cl_float3& v)
{
	return sqrt((v.s[0] * v.s[0]) + (v.s[1] * v.s[1]) + (v.s[2] * v.s[2]));
}

cl_float3 normalize(const cl_float3& v)
{
	cl_float3 result;
	float l_length = length(v);
	result.s[0] = v.s[0] / l_length;
	result.s[1] = v.s[1] / l_length;
	result.s[2] = v.s[2] / l_length;

	return result;
}


#endif // __CLMATH_HEADER__