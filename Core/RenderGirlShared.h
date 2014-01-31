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

#ifndef __RENDERGIRL_SHARED_CLASS__
#define __RENDERGIRL_SHARED_CLASS__

#include <assert.h>
#include <string>

#include "CL\cl.h"
#include "Log.h"
#include "OCLPlatform.h"
#include "OCLDevice.h"
#include "OCLKernel.h"


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
	static bool SelectDevice(OCLDevice* select);

	/* Release the selected device from use, deallocing all memory used  */
	static void ReleaseDevice();

	/* Return selected device */
	static const OCLDevice* GetSelectedDevice()
	{
		assert(selectedDevice != NULL && "There's no selected device to get!");
		return selectedDevice;
	}

	/* return list of avaiable platforms */ 
	static inline const std::vector<OCLPlatform>& ReturnPlatforms()
	{
		return platforms;
	}

	/* return number of avaiable platforms */
	static inline const int GetPlatformsSize()
	{
		return platforms.size();
	}
	~RenderGirlShared();

private:
	RenderGirlShared(){ ; }

	// OpenCL stuff for internal control
	static std::vector<OCLPlatform> platforms;

	// device selected for doing the computation
	static OCLDevice* selectedDevice;
};



#endif // __RENDERGIRL_SHARED_CLASS__