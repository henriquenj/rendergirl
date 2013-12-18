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

#ifndef __OPENCLSHARED_CLASS__
#define __OPENCLSHARED_CLASS__

#include <assert.h>
#include <string>

#include "CL\cl.h"
#include "Log.h"


// types of devices accepted by OpenCL
enum DeviceType
{
	Default = 0x00000001,
	CPU = 0x00000002,
	GPU = 0x00000004,
	Accelerator = 0x00000008,
	All = 0xFFFFFFFF
};

struct OCLPlatform
{
	cl_platform_id id;
	std::string name;
};


// Static class encapsules all the OpenCL status and interacts with the OpenCL device
class OpenCLShared
{
public:
	/*init OpenCL platforms, return TRUE for success or return FALSE for failure*/
	static bool InitPlatforms();
	/* return list of avaiable platforms */ 
	static inline const std::vector<OCLPlatform>& ReturnPlatforms()
	{
		return platformIds;
	}
	/* return number of avaiable platforms*/
	static inline const int GetPlatformsSize()
	{
		return platformIds.size();
	}
	~OpenCLShared();

private:
	OpenCLShared(){;}

	// OpenCL stuff for internal control
	static std::vector<OCLPlatform> platformIds;
	static cl_device_id deviceId;
	static cl_context context; 

	// helper function to query information about platforms (thanks QT project for the hint =D)
	static const std::string GetStringFromPlatform(cl_platform_id id, cl_platform_info name);
};



#endif // __OPENCLSHARED_CLASS__