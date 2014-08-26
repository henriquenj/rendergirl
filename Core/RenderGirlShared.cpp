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

RenderGirlShared::RenderGirlShared()
{
	m_selectedDevice = NULL;
	m_program = NULL;
	m_kernel = NULL;
	m_frame = NULL;
}
RenderGirlShared::~RenderGirlShared()
{
	if (m_selectedDevice != NULL)
		this->ReleaseDevice();

	int size = m_platforms.size();
	for (unsigned int p = 0; p < size; p++)
	{
		delete m_platforms[p];
	}
	m_platforms.clear();
}
bool RenderGirlShared::InitPlatforms()
{
	
	//delete previous selected device, if there's any
	if (m_selectedDevice != NULL)
		this->ReleaseDevice();

	// delete previous loaded platforms, if there's any
	if (!m_platforms.empty())
	{
		int size = m_platforms.size();
		for (unsigned int p = 0; p < size; p++)
		{
			delete m_platforms[p];
		}
		m_platforms.clear();
	}

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
		OCLPlatform* platform = new OCLPlatform(platforms_cl[a]);
		m_platforms.push_back(platform);
	}

	delete[] platforms_cl;

	return true;
}

bool RenderGirlShared::InitDevices(OCLDevice::DeviceType type)
{
	//delete previous selected device, if there's any
	if (m_selectedDevice != NULL)
		this->ReleaseDevice();

	bool allOk = true;
	int size = m_platforms.size();
	for (int a = 0; a < size; a++)
	{
		allOk = m_platforms[a]->InitDevices(type);
	}

	return allOk;
}

bool RenderGirlShared::SelectDevice(const OCLDevice* select)
{
	assert(select != NULL);

	bool error = true;
	if (m_selectedDevice != NULL)
	{
		this->ReleaseDevice();
	}

	m_selectedDevice = const_cast<OCLDevice*>(select);
	if (!m_selectedDevice->IsReady())
	{
		// prepare this device
		error = m_selectedDevice->CreateContext();
		Log::Message("Selected device: " + m_selectedDevice->GetName());

		/* send context do scene manager */
		SceneManager& manager = SceneManager::GetSharedManager();
		manager.SetContext(m_selectedDevice->GetContext());
	}

	return error;
}

bool RenderGirlShared::PrepareRaytracer()
{
	assert(m_selectedDevice != NULL);
	assert(m_program == NULL);
	OCLContext* context = m_selectedDevice->GetContext();

	/* Prepare this device compiling the OpenCL kernels*/

	m_program = new OCLProgram(context);
	if (!m_program->LoadSource("Raytracer.cl"))
	{
		delete m_program;
		m_program = NULL;
		return false;
	}

	if (!m_program->BuildProgram())
	{
		delete m_program;
		m_program = NULL;
		return false;
	}

	m_kernel = new OCLKernel(m_program, std::string("Raytrace"));
	if (!m_kernel->GetOk())
	{
		delete m_program;
		delete m_kernel;
		m_kernel = NULL;
		m_program = NULL;
		return false;
	}

	Log::Message("Device ready for execution.");
	return true;
}

bool RenderGirlShared::Render(int width, int height, Camera &camera, Light &light)
{
	assert(m_selectedDevice != NULL && "You must have a working context to call this");

	if (height < 1 || width < 1)
	{
		Log::Error("You have to choose a positive resolution");
		return false;
	}

	OCLContext* context = m_selectedDevice->GetContext();
	cl_bool error = false;

	/* setup scene */
	SceneManager& sceneManager = SceneManager::GetSharedManager();
	if (!sceneManager.PrepareScene(m_kernel))
		return false;

	/* Setup render frame */

	int pixelCount = width * height; // total amount of pixels

	// delete old frame
	if (m_frame != NULL)
	{
		context->DeleteMemoryObject<cl_uchar4>(m_frame);
		m_frame = NULL;
	}

	m_frame = context->CreateMemoryObject<cl_uchar4>(pixelCount, WriteOnly, &error);
	if (error)
		return false;

	cl_uchar4* frameRaw = new cl_uchar4[pixelCount];
	m_frame->SetData(frameRaw,false);

	/* Setup render info */
	m_scene.width = width;
	m_scene.height = height;
	m_scene.pixelCount = pixelCount;
	m_scene.groupsSize = sceneManager.GetGroupsCount();
	m_scene.proportion_x = (float)width / (float)height;
	m_scene.proportion_y = (float)height / (float)width;

	OCLMemoryObject<SceneInformation>* sceneInfoMem = context->CreateMemoryObjectWithData(1, &m_scene, true, ReadOnly);
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
	m_kernel->SetArgument(4, sceneInfoMem);
	m_kernel->SetArgument(5, m_frame);
	m_kernel->SetArgument(6, mem_cam);
	m_kernel->SetArgument(7, mem_light);

	m_kernel->SetGlobalWorkSize(pixelCount); // one work-iten per pixel

	Log::Message("Rendering...");

	// start counter
	auto pretime = std::chrono::high_resolution_clock::now();

	if (!m_kernel->EnqueueExecution())
		return false;

	if (!context->ExecuteCommands())
		return false;


	// finish timer
	// got that from here http://stackoverflow.com/questions/1487695/c-cross-platform-high-resolution-timer
	auto postime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(postime - pretime);
	Log::Message("Rendering took " + std::to_string((float)(ns.count() / 1000000000.0f)) + " seconds.");

	m_frame->SyncDeviceToHost();

	return true;
}

void RenderGirlShared::ReleaseDevice()
{
	assert(m_selectedDevice != NULL);

	Log::Message("Freeing resources on device " + m_selectedDevice->GetName());

	SceneManager& manager = SceneManager::GetSharedManager();
	manager.SetContext(NULL); /* update context reference of the manager */

	if (m_kernel != NULL)
	{
		delete m_kernel;
		m_kernel = NULL;
	}
	if (m_program != NULL)
	{
		delete m_program;
		m_program = NULL;
	}
	if (m_frame != NULL)
	{
		/* this will get deallocated anyway on ReleaseContext so there's no need to delete it here
			just remove the reference */
		m_frame = NULL;
	}

	m_selectedDevice->ReleaseContext();
	m_selectedDevice = NULL;
}