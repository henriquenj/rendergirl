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
	m_kernel_AA = NULL;
	m_frame = NULL;

	m_efficiencyInfo = false;
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

bool RenderGirlShared::PrepareRaytracer(const bool efficiency)
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

	std::string program_options = std::string();
	if (efficiency)
	{
		/* tells the OpenCL compiler that code marked within
		EFFICIENCY_METRICS should be compiled
		*/
		program_options += "-D EFFICIENCY_METRICS";
		this->m_efficiencyInfo = true;
	}
	else
	{
		this->m_efficiencyInfo = false;
	}

	if (!m_program->BuildProgram(program_options))
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

bool RenderGirlShared::PrepareAntiAliasing()
{
	m_kernel_AA = new OCLKernel(m_program, std::string("AntiAliasingFXAA"));
	if (!m_kernel_AA->GetOk())
	{
		delete m_program;
		delete m_kernel_AA;
		m_kernel_AA = NULL;
		delete m_kernel;
		m_kernel = NULL;
		m_program = NULL;
		return false;
	}
	return true;
}


bool RenderGirlShared::ExecuteAntiAliasing( int width, int height)
{
	OCLContext* context = m_selectedDevice->GetContext();

	cl_bool error = false;

	cl_int temp = width; 

	OCLMemoryObject<cl_int>* widthMem = context->CreateMemoryObject<cl_int>(1, ReadOnly, &error);
	widthMem->SetData(&temp, true);
	widthMem->SyncHostToDevice();
	temp = height;
	OCLMemoryObject<cl_int>* heightMem = context->CreateMemoryObject<cl_int>(1, ReadOnly, &error);
	heightMem->SetData(&temp, true);
	heightMem->SyncHostToDevice();

	if (error)
		return false;

	if (!m_kernel_AA->SetArgument(0, m_frame))
		return false;
	if (!m_kernel_AA->SetArgument(1, m_frame_AA))
		return false;
	if (!m_kernel_AA->SetArgument(2, widthMem))
		return false;
	if (!m_kernel_AA->SetArgument(3, heightMem))
		return false;


	m_kernel_AA->SetGlobalWorkSize(width * height); // one work-iten per pixel

	if (!m_kernel_AA->EnqueueExecution())
		return false;

	return true;
}

bool RenderGirlShared::Render(int width, int height, Camera &camera, Light &light, AntiAliasingMethod AAOption)
{
	assert(m_selectedDevice != NULL && "You must have a working context to call this");

	// start counter
	auto pretime = std::chrono::high_resolution_clock::now();

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

	cl_uchar4* frameRawAA;
	if (AAOption != noAA)		//if there is antialiasing
	{
		if (!PrepareAntiAliasing())
			return false;

		if (m_frame_AA != NULL)
		{
			context->DeleteMemoryObject<cl_uchar4>(m_frame_AA);
			m_frame_AA = NULL;
		}
		m_frame_AA = context->CreateMemoryObject<cl_uchar4>(pixelCount, WriteOnly, &error);
		if (error)
			return false;


		frameRawAA = new cl_uchar4[pixelCount];
		m_frame_AA->SetData(frameRawAA, false);
		m_frame_AA->SyncHostToDevice();
	}

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
	if (camera.from_lookAt)
	{ 
		// compute direction form lookAt (temporary workarounf until camera API is ready)
		camera.dir = subtract(camera.lookAt, camera.pos);
	}
	camera.dir = normalize(camera.dir);
	camera.right = cross(camera.dir, camera.up);
	camera.up = cross(camera.right, camera.dir); //This corrects for any slop in the choice of "up"
	

	OCLMemoryObject<Camera>* mem_cam = context->CreateMemoryObjectWithData(1, &camera, true, ReadOnly);
	mem_cam->SyncHostToDevice();

	/*
	Efficiency metrics for now consist of two integer counters. The intersectCounter will count the 
	amount of calls to the Intersect function within the OpenCL kernel. 
	The intersectHitCounter will count the amount of intersecitons that actually hit a triangle. 
	To count, we will use OpenCL atomic operatations.

	KNOWN ISSUE: Unsigned integers will easily overflow on the intersect counter since a couple of 
	thousand faces on a reasonably low resolution (740p) can generate 4 billion intersects. 
	This will be currently a limitation since 64 bits atomic operations in OpenCL are done using 
	an extension, which to the best of our knowledge NVIDIA does not implement.
	*/
	cl_uint temp = 0;
	OCLMemoryObject<cl_uint>* mem_intersectCounter = context->CreateMemoryObject<cl_uint>(1);
	mem_intersectCounter->SetData(&temp);

	OCLMemoryObject<cl_uint>* mem_intersectHitCounter = context->CreateMemoryObject<cl_uint>(1);
	mem_intersectHitCounter->SetData(&temp);
	
	if (m_efficiencyInfo)
	{
		mem_intersectCounter->SyncHostToDevice();
		mem_intersectHitCounter->SyncHostToDevice();
	}

	// set remaining arguments
	m_kernel->SetArgument(4, sceneInfoMem);
	m_kernel->SetArgument(5, m_frame);
	m_kernel->SetArgument(6, mem_cam);
	m_kernel->SetArgument(7, mem_light);
	m_kernel->SetArgument(8, mem_intersectCounter);
	m_kernel->SetArgument(9, mem_intersectHitCounter);

	m_kernel->SetGlobalWorkSize(pixelCount); // one work-iten per pixel

	if (!m_kernel->EnqueueExecution())
		return false;

	if (AAOption != noAA)
	{
		if (!ExecuteAntiAliasing(width, height))
			return false;

		m_frame->CopyFromMemoryBuffer(m_frame_AA);
	}

	if (!context->ExecuteCommands())
		return false;

	if (AAOption != noAA)
	{
		context->DeleteMemoryObject(m_frame_AA);
		m_frame_AA = NULL;
	}

	m_frame->SyncDeviceToHost();

	// finish timer
	auto postime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(postime - pretime);
	Log::Message("Rendering took " + std::to_string((float)(ns.count() / 1000000000.0f)) + " seconds.");

	if (m_efficiencyInfo)
	{
		float hitPercentage = 0;
		mem_intersectCounter->SyncDeviceToHost();
		mem_intersectHitCounter->SyncDeviceToHost();
		/* compute efficiency of ray collisions */
		cl_uint intersectCounter = (*mem_intersectCounter)[0];
		cl_uint intersectHitCounter = (*mem_intersectHitCounter)[0];

		if (intersectCounter > 0 && intersectHitCounter > 0)
		{
			hitPercentage = (100.0f * intersectHitCounter) / intersectCounter;
		}

		Log::Message("Amount of intersect tests: " + std::to_string(intersectCounter));
		Log::Message("Amount of hits on intersect tests " + std::to_string(intersectHitCounter));
		Log::Message("Percentage of successful hits is " + std::to_string(hitPercentage) + "%");
	}


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
	if (m_kernel_AA != NULL)
	{
		delete m_kernel_AA;
		m_kernel_AA = NULL;
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
	if (m_frame_AA != NULL)
	{
		/* this will get deallocated anyway on ReleaseContext so there's no need to delete it here
		just remove the reference */
		m_frame_AA = NULL;
	}

	m_selectedDevice->ReleaseContext();
	m_selectedDevice = NULL;
}