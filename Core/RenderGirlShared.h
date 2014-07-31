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

enum AntiAliasing
{
	noAA,
	FXAA
};
/* Singleton class encapsules the OpenCL status and the renderer status.
	This singleton architecture was kindly sugested by Loki Astari at
	http://stackoverflow.com/questions/270947/can-any-one-provide-me-a-sample-of-singleton-in-c/271104#271104
	*/
class RenderGirlShared
{
public:
	/* get shared instance of this singleton */
	static RenderGirlShared& GetRenderGirlShared()
	{
		static RenderGirlShared instance;
		return instance;
	}

	~RenderGirlShared();

	/*init OpenCL platforms, return TRUE for success or return FALSE for failure*/
	bool InitPlatforms();
	/* init all devices for all platforms, return FALSE if at least one device failed to initialize
	   param type define which type of device will be initialized */
	bool InitDevices(OCLDevice::DeviceType type = OCLDevice::All);

	/* Prepare a given device for executing the OpenCL program, return FALSE if there's an error with device*/
	bool SelectDevice(const OCLDevice* select);

	/* PrepareRaytracer function prepare the OpenCL raytracer to work on the selected device.
		You got to have a selected device to call this. Return FALSE if there's an error with the device. */
	bool PrepareRaytracer();

	/* Send 3D data to the renderer. This should be called after PrepareRaytracer.
		The renderer will copy the data, so you are free to use it afterwards.
		Return FALSE if there's an error */
	bool Set3DScene(Scene3D* pscene);

	/* Render a frame. You should only call this with a kernel ready and a 3D scene.
		This is a blocking call.
		Param resolution is the resolution of the resulting image.
		Return FALSE for an error */
	bool Render(int width, int height, Camera &camera, Light &light, AntiAliasing AAOption = FXAA);

	/* Release the selected device from use, deallocing all memory used */
	void ReleaseDevice();

	/* Return selected device, return NULL for no device */
	const OCLDevice* GetSelectedDevice()
	{
		return m_selectedDevice;
	}

	/* return list of avaiable platforms */ 
	inline const std::vector<OCLPlatform*>& ReturnPlatforms()
	{
		return m_platforms;
	}

	/* Get rendered buffer. This memory belongs to the renderer, so don't delete it.*/
	inline const cl_uchar4* GetFrame()
	{
		return m_frame->GetData();
	}

	/* return number of avaiable platforms */
	inline const int GetPlatformsSize()
	{
		return m_platforms.size();
	}

private:
	RenderGirlShared();

	/* DELIO: WRITE WHAT YOUR FUNCTION IS DOING!
		From our wiki: Always put a comment just above the function signature telling what it does, 
						what each parameter means and what is returning.
	*/
	bool PrepareAntiAliasing();
	bool ExecuteAntiAliasing(int width, int height);
	// prevent copy by not implementing this methods
	RenderGirlShared(RenderGirlShared const&);
	void operator=(RenderGirlShared const&);

	// OpenCL stuff for internal control
	std::vector<OCLPlatform*> m_platforms;

	// device selected for doing the computation
	OCLDevice* m_selectedDevice;

	OCLProgram* m_program;
	OCLKernel* m_kernel;
	OCLKernel* m_kernel_AA;
	SceneInformation m_scene;
	bool m_sceneLoaded;
	OCLMemoryObject<cl_uchar4>* m_frame;
	OCLMemoryObject<cl_uchar4>* m_frame_AA;
};



#endif // __RENDERGIRL_SHARED_CLASS__