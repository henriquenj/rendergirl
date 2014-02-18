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


#include "RenderGirlShared.h"

std::vector<OCLPlatform>	RenderGirlShared::platforms;
OCLDevice*					RenderGirlShared::selectedDevice = NULL;
OCLProgram*					RenderGirlShared::program = NULL;
OCLKernel*					RenderGirlShared::kernel = NULL;
Scene3D*					RenderGirlShared::scene = NULL;
bool						RenderGirlShared::sceneLoaded = false;
OCLMemoryObject<cl_uchar3>*	RenderGirlShared::frame = NULL;


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
	for (int a = 0; a < platSize; a++)
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

bool RenderGirlShared::SelectDevice(OCLDevice* select)
{
	assert(select != NULL);

	bool error = true;
	if (selectedDevice != NULL)
	{
		RenderGirlShared::ReleaseDevice();
	}

	selectedDevice = select;
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

	kernel = new OCLKernel(program,std::string("Raytrace"));
	if (!kernel->GetOk())
	{
		delete program;
		delete kernel;
		kernel = NULL;
		program = NULL;
		return false;
	}

	return true;
}

bool RenderGirlShared::Set3DScene(Scene3D* pscene)
{

	assert(kernel != NULL);
	assert(kernel->GetOk());
	assert(pscene != NULL);

	scene = pscene;

	OCLContext* context = selectedDevice->GetContext();

	if (frame != NULL)
	{
		context->DeleteMemoryObject(frame);
		frame = NULL;
	}

	cl_bool error = false;
	/* Send data to the OpenCL device*/
	OCLMemoryObject<cl_float3>* vertices = context->CreateMemoryObject<cl_float3>(scene->verticesSize, ReadOnly,&error);
	if (error)
		return false;
	OCLMemoryObject<cl_float3>* normals = context->CreateMemoryObject<cl_float3>(scene->normalSize,ReadOnly,&error);
	if (error)
		return false;
	OCLMemoryObject<cl_int3>* faces = context->CreateMemoryObject<cl_int3>(scene->facesSize, ReadOnly,&error);
	if (error)
		return false;

	vertices->SetData(scene->vertices,false);
	normals->SetData(scene->normal, false);
	faces->SetData(scene->faces, false);

	if (!context->SyncAllMemoryHostToDevice())
		return false;

	/* Set kernel arguments */
	if (!kernel->SetArgument(0, vertices))
		return false;
	if (!kernel->SetArgument(1, normals))
		return false;
	if (!kernel->SetArgument(2, faces))
		return false;

	sceneLoaded = true;
	return true;
}

bool RenderGirlShared::Render(int resolution)
{
	if (!sceneLoaded)
	{
		Log::Error("Cannot render since there's no loaded scene!");
		return false;
	}

	OCLContext* context = selectedDevice->GetContext();
	cl_bool error = false;

	/* Setup render frame */

	int pixelCount = resolution * resolution; // total amount of pixels
	cl_uchar3* frameRaw = NULL;

	// delete old frame if it's on a different resolution, otherwise, reuses it
	if (frame != NULL)
	{
		if (pixelCount != frame->GetSize())
		{
			context->DeleteMemoryObject<cl_uchar3>(frame);
			frame = context->CreateMemoryObject<cl_uchar3>(pixelCount, WriteOnly, &error);
			if (error)
				return false;
			frameRaw = new cl_uchar3[pixelCount];
			frame->SetData(frameRaw);
		}
	}
	else
	{
		frame = context->CreateMemoryObject<cl_uchar3>(pixelCount, WriteOnly, &error);
		if (error)
			return false;
		frameRaw = new cl_uchar3[pixelCount];
		frame->SetData(frameRaw);
	}

	/* Setup render info */
	SceneInformation sceneInfo;
	sceneInfo.resolution = resolution;
	sceneInfo.pixelCount = pixelCount;
	sceneInfo.normalSize = scene->normalSize;
	sceneInfo.facesSize = scene->facesSize;
	sceneInfo.verticesSize = scene->verticesSize;

	OCLMemoryObject<SceneInformation>* sceneInfoMem = context->CreateMemoryObjectWithData(1, &sceneInfo, true, ReadOnly);
	sceneInfoMem->SyncHostToDevice();

	// set remaining arguments
	kernel->SetArgument(3, sceneInfoMem);
	kernel->SetArgument(4, frame);

	kernel->SetGlobalWorkSize(resolution * resolution); // one work-iten per pixel
	if (!kernel->EnqueueExecution())
		return false;

	Log::Message("Rendering...");
	if (!context->ExecuteCommands())
		return false;

	Log::Message("Rendering complete!");

	frame->SyncDeviceToHost();

	return true;
}

void RenderGirlShared::ReleaseDevice()
{
	assert(selectedDevice != NULL);
	
	if (program != NULL)
	{
		delete program;
		program = NULL;
	}
	if (kernel != NULL)
	{
		delete kernel;
		kernel = NULL;
	}
	if (scene != NULL)
	{
		delete scene;
		scene = NULL;
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