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
	/* Function type used for registering callbacks on Python side*/
	typedef void (*c_log_callback)(const char* message, bool error);

	/* Start log subsystem providing a pointer to function to call when
		there's a new message to be trasmitted */
	void StartLogSystem(const c_log_callback callback);

	/* Start rendergirl, probe the computer for OpenCL capable devices
		Return 0 for success
	*/
	int StartRendergirl();

	/* Add a scenegroup to rendergirl core.
		To keep arguments simple, pointers do not point to array objects, instead
		all data must be transmitted as a single array. For instance:
		vertex information will be transmitted as
		float* = {1.0f , 0f , 2.0f , 4.0f ,-1.0f , 1.0f}
		and wrapped internally as
		{cl_float3(1.0f , 0f , 2.0f ), cl_float3(4.0f ,-1.0f , 1.0f)}
	*/
	int AddSceneGroup(
		const char* name, /* null terminated string with the name of this scene group*/
		/* vertex buffer as a sequence of XYZ values */
		const float* vertex,
		/* vertex buffer size, the total number of vertices can be computed as vertex_size / 3 */
		const int vertex_size,
		/* faces buffer, each element points to a position on the vertex buffer,
		 each face is composed of three vertices */
		const int* faces[3],
		/* the amounf of faces on the faces buffer */
		const int faces_size,
		/* position of this group in the scene */
		const float position[3],
		/* rotation of this group in radians */
		const float rotation[3],
		/* scale of this group (0.0f - 1.0f) */
        const float scale[3]);

	/* Clear all geometry loaded on the core */
	void ClearScene();

	/* Render the scene current loaded on RenderGirl core
		Return 0 for no errror, -1 otherwise
	*/
	int Render(
		const float camera_pos[3], // camera XYZ position (eye)
		const float camera_look_at[3], // camera looking at vector
		const float light_pos[3], // light XYZ position
		const float color[3], // RGB color component of the light
		const float light_ks, // amount of specular light
		const float light_ka // amount of ambient light
		);

	/* Finish RenderGirl and release resources from OpenCL devices */
	void FinishRenderGirl();

	/* Finish log system*/
	void FinishLogSystem();

	/* Set source path where RenderGirl will look for the .cl source files */
	void SetSourcePath(const char* path);
}

#endif //__RENDERGIRL_BLENDER__
