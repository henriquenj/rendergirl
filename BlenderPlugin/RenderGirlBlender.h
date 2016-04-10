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

	/* Fetch the amount of devices available on this computer */
	int FetchDevicesSize();

	/*
	Fetch device names from RenderGirl core.
	devices_out parameter is a pointer to an array of null terminated read-only strings containing the name of the devices,
	the array must have been previously allocated. To get the size of the array, call FetchDevicesSize;
	Example usage: 
		int device_size = FetchDevicesSize();
		char const ** devices_out = (const char**)malloc(device_size * sizeof(const char **));
		FetchDevicesName(devices_out);
		for (int i = 0; i < device_size; i++)
		{
			printf("%s\n", devices_out[i]);
		}
		free(devices_out);
	*/
	void FetchDevicesName(char const ** devices_out);

	/* Select an OpenCL capalable device
	device parameter is the index of the device within the "devices_out" argument from FetchDevices function
	return 0 if the selection was successfully, return -1 for error
	*/
	int SelectDevice(const int device);

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
		Return 0 for no errror, -1 otherwise.
	*/
	int Render(
		const int width, // width of the frame in pixels
		const int height, // height of the frame in pixels
		const float camera_pos[3], // camera XYZ position (eye)
		const float camera_up[3], // camera up vector
		const float camera_dir[3], // camera looking at vector
		const float light_pos[3], // light XYZ position
		const float color[3], // RGB color component of the light
		unsigned char* frame_out /* The rendered frame as an array of chars, each pixels composed by 4 values (reg, green, blue and alpha).
									This memory must have been previously allocated by the caller, otherwise there might
									be a segfault. The amount of memory used is width * height * 4 bytes. In case of error, no change
									to the contents of the memory pointed by frame_out is performed. */
		);

	/* Finish RenderGirl and release resources from OpenCL devices */
	void FinishRenderGirl();

	/* Finish log system*/
	void FinishLogSystem();

	/* Set source path where RenderGirl will look for the .cl source files */
	void SetSourcePath(const char* path);
}

#endif //__RENDERGIRL_BLENDER__
