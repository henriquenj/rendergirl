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

#ifndef __RENDERGIRL_SHARED_CLASS__
#define __RENDERGIRL_SHARED_CLASS__

#include <assert.h>
#include <string>

#include "CL\cl.h"
#include "Log.h"
#include "OCLPlatform.h"
#include "OCLDevice.h"
#include "OCLKernel.h"
#include "CLStructs.h"

// Static class encapsules the OpenCL status and the renderer status
class RenderGirlShared
{
public:
	/*init OpenCL platforms, return TRUE for success or return FALSE for failure*/
	static bool InitPlatforms();
	/* init all devices for all platforms, return FALSE if at least one device failed to initialize
	   param type define which type of device will be initialized */
	static bool InitDevices(OCLDevice::DeviceType type = OCLDevice::All);

	/* Prepare a given device for executing the OpenCL program, return FALSE if there's an error with device*/
	static bool SelectDevice(const OCLDevice* select);

	/* PrepareRaytracer function prepare the OpenCL raytracer to work on the selected device.
		You got to have a selected device to call this. Return FALSE if there's an error with the device. */
	static bool PrepareRaytracer();

	/* Send 3D data to the renderer. This should be called after PrepareRaytracer.
		The renderer will copy the data, so you are free to use it afterwards.
		Return FALSE if there's an error */
	static bool Set3DScene(Scene3D* pscene);

	/* Render a frame. You should only call this with a kernel ready and a 3D scene.
		This is a blocking call.
		Param resolution is the resolution of the resulting image.
		Return FALSE for an error */
	static bool Render(int width, int height, Camera &camera, Light &light);

	/* Release the selected device from use, deallocing all memory used */
	static void ReleaseDevice();

	/* Return selected device, return NULL for no device */
	static const OCLDevice* GetSelectedDevice()
	{
		return selectedDevice;
	}

	/* return list of avaiable platforms */ 
	static const inline std::vector<OCLPlatform>* ReturnPlatforms()
	{
		return &platforms;
	}

	/* Get rendered buffer. This memory belongs to the renderer, so don't delete it.*/
	static inline const cl_uchar4* GetFrame()
	{
		return frame->GetData();
	}

	/* return number of avaiable platforms */
	static inline const int GetPlatformsSize()
	{
		return platforms.size();
	}

private:
	RenderGirlShared(){ ; }

	// OpenCL stuff for internal control
	static std::vector<OCLPlatform> platforms;

	// device selected for doing the computation
	static OCLDevice* selectedDevice;

	static OCLProgram* program;
	static OCLKernel* kernel;
	static SceneInformation scene;
	static bool sceneLoaded;
	static OCLMemoryObject<cl_uchar4>* frame;
};



#endif // __RENDERGIRL_SHARED_CLASS__