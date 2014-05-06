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


#include "RenderGirlShared.h"
#include "CLMath.h"
#include <chrono>

std::vector<OCLPlatform>	RenderGirlShared::platforms;
OCLDevice*					RenderGirlShared::selectedDevice = NULL;
OCLProgram*					RenderGirlShared::program = NULL;
OCLKernel*					RenderGirlShared::kernel = NULL;
SceneInformation			RenderGirlShared::scene;
bool						RenderGirlShared::sceneLoaded = false;
OCLMemoryObject<cl_uchar4>*	RenderGirlShared::frame = NULL;


bool RenderGirlShared::InitPlatforms()
{

	// do not query for new platforms if it has queried before
	assert(platforms.empty());

	// create and init OpenCL
	Log::Message("Initializing OpenCL platforms...");

	// create plataforms
	cl_uint platSize = 0;	// number of loaded platforms

	// empty query to see the size of platforms
	if (clGetPlatformIDs(0, 0, &platSize) != CL_SUCCESS)
	{
		Log::Error("OpenCL platform could not be created, please check the OpenCL drivers for your device");
		return false;
	}

	cl_platform_id* platforms_cl = new cl_platform_id[platSize];

	// now the real query
	if (clGetPlatformIDs(platSize, platforms_cl, &platSize) != CL_SUCCESS)
	{
		Log::Error("OpenCL platform could not be created, please check the OpenCL drivers for your device");
		return false;
	}

	Log::Message(std::to_string(platSize) + " platforms were found.");

	// put all in the platforms vector
	for (unsigned int a = 0; a < platSize; a++)
	{
		Log::Message("");
		Log::Message("Platform " + std::to_string(a + 1) + " created using the specs below: ");
		OCLPlatform platform;
		platform.Init(platforms_cl[a]);
		platforms.push_back(platform);
	}

	delete[] platforms_cl;

	return true;
}

bool RenderGirlShared::InitDevices(OCLDevice::DeviceType type)
{
	bool allOk = true;
	int size = platforms.size();
	for (int a = 0; a < size; a++)
	{
		allOk = platforms[a].InitDevices(type);
	}

	return allOk;
}

bool RenderGirlShared::SelectDevice(const OCLDevice* select)
{
	assert(select != NULL);

	bool error = true;
	if (selectedDevice != NULL)
	{
		RenderGirlShared::ReleaseDevice();
	}

	selectedDevice = const_cast<OCLDevice*>(select);
	if (!selectedDevice->IsReady())
	{
		// prepare this device
		error = selectedDevice->CreateContext();
		Log::Message("Selected device: " + selectedDevice->GetName());
	}

	return error;
}

bool RenderGirlShared::PrepareRaytracer()
{
	assert(selectedDevice != NULL);
	assert(program == NULL);
	OCLContext* context = selectedDevice->GetContext();

	/* Prepare this device compiling the OpenCL kernels*/

	program = new OCLProgram(context);
	if (!program->LoadProgramWithSource("Raytracer.cl"))
	{
		delete program;
		program = NULL;
		return false;
	}

	if (!program->BuildProgram())
	{
		delete program;
		program = NULL;
		return false;
	}

	kernel = new OCLKernel(program, std::string("Raytrace"));
	if (!kernel->GetOk())
	{
		delete program;
		delete kernel;
		kernel = NULL;
		program = NULL;
		return false;
	}

	Log::Message("Device ready for execution.");
	return true;
}

bool RenderGirlShared::Set3DScene(Scene3D* pscene)
{

	assert(kernel != NULL);
	assert(kernel->GetOk());
	assert(pscene != NULL);

	scene.facesSize = pscene->facesSize;
	scene.normalSize = pscene->normalSize;
	scene.verticesSize = pscene->verticesSize;
	scene.materiaslSize = pscene->materialSize;

	OCLContext* context = selectedDevice->GetContext();

	cl_bool error = false;
	/* Send data to the OpenCL device*/
	OCLMemoryObject<cl_float3>* vertices = context->CreateMemoryObject<cl_float3>(scene.verticesSize, ReadOnly, &error);
	if (error)
		return false;
	OCLMemoryObject<cl_float3>* normals = context->CreateMemoryObject<cl_float3>(scene.normalSize, ReadOnly, &error);
	if (error)
		return false;
	OCLMemoryObject<cl_int4>* faces = context->CreateMemoryObject<cl_int4>(scene.facesSize, ReadOnly, &error);
	if (error)
		return false;
	OCLMemoryObject<Material>* materials = context->CreateMemoryObject<Material>(scene.materiaslSize, ReadOnly, &error);
	if (error)
		return false;


	vertices->SetData(pscene->vertices);
	normals->SetData(pscene->normal);
	faces->SetData(pscene->faces);
	materials->SetData(pscene->materials);

	if (!context->SyncAllMemoryHostToDevice())
		return false;

	/* Set kernel arguments */
	if (!kernel->SetArgument(0, vertices))
		return false;
	if (!kernel->SetArgument(1, normals))
		return false;
	if (!kernel->SetArgument(2, faces))
		return false;
	if (!kernel->SetArgument(3, materials))
		return false;

	sceneLoaded = true;
	return true;
}

bool RenderGirlShared::Render(int width, int height, Camera &camera, Light &light)
{

	if (height < 1 || width < 1)
	{
		Log::Error("You have to choose a positive resolution");
		return false;
	}

	if (!sceneLoaded)
	{
		Log::Error("Cannot render since there's no loaded scene!");
		return false;
	}

	OCLContext* context = selectedDevice->GetContext();
	cl_bool error = false;

	/* Setup render frame */

	int pixelCount = width * height; // total amount of pixels

	// delete old frame
	if (frame != NULL)
	{
		context->DeleteMemoryObject<cl_uchar4>(frame);
		frame = NULL;
	}

	frame = context->CreateMemoryObject<cl_uchar4>(pixelCount, WriteOnly, &error);
	if (error)
		return false;

	cl_uchar4* frameRaw = new cl_uchar4[pixelCount];
	frame->SetData(frameRaw,false);

	/* Setup render info */
	scene.width = width;
	scene.height = height;
	scene.pixelCount = pixelCount;
	
	OCLMemoryObject<SceneInformation>* sceneInfoMem = context->CreateMemoryObjectWithData(1, &scene, true, ReadOnly);
	sceneInfoMem->SyncHostToDevice();


	OCLMemoryObject<Light>* mem_light = context->CreateMemoryObjectWithData(1, &light, true, ReadOnly);
	mem_light->SyncHostToDevice();
	
	/* Precompute some camera stuff*/
	// based on the algorithm provided by this user here http://stackoverflow.com/a/13078758/1335511
	camera.dir = subtract(camera.lookAt,camera.pos);
	camera.dir = normalize(camera.dir);
	camera.right = cross(camera.dir, camera.up);
	camera.up = cross(camera.right, camera.dir); //This corrects for any slop in the choice of "up"
	

	OCLMemoryObject<Camera>* mem_cam = context->CreateMemoryObjectWithData(1, &camera, true, ReadOnly);
	mem_cam->SyncHostToDevice();

	// set remaining arguments
	kernel->SetArgument(4, sceneInfoMem);
	kernel->SetArgument(5, frame);
	kernel->SetArgument(6, mem_cam);
	kernel->SetArgument(7, mem_light);

	kernel->SetGlobalWorkSize(pixelCount); // one work-iten per pixel

	Log::Message("Rendering...");

	// start counter
	auto pretime = std::chrono::high_resolution_clock::now();

	if (!kernel->EnqueueExecution())
		return false;

	if (!context->ExecuteCommands())
		return false;


	// finish timer
	// got that from here http://stackoverflow.com/questions/1487695/c-cross-platform-high-resolution-timer
	auto postime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(postime - pretime);
	Log::Message("Rendering took " + std::to_string((float)(ns.count() / 1000000000.0f)) + " seconds.");

	frame->SyncDeviceToHost();

	return true;
}

void RenderGirlShared::ReleaseDevice()
{
	assert(selectedDevice != NULL);

	Log::Message("Freeing resources on device " + selectedDevice->GetName());

	if (kernel != NULL)
	{
		delete kernel;
		kernel = NULL;
	}
	if (program != NULL)
	{
		delete program;
		program = NULL;
	}
	if (frame != NULL)
	{
		/* this will get deallocated anyway on ReleaseContext so there's no need to delete it here
			just remove the reference */
		frame = NULL;
	}

	selectedDevice->ReleaseContext();
	selectedDevice = NULL;
	sceneLoaded = false;
}