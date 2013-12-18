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


// Static class encapsules all the OpenCL status and interacts with the OpenCL device
class OpenCLShared
{
public:
	/*init OpenCL on a given device*/
	static void InitOpenCL(DeviceType deviceType = CPU);
	/* return true if the OpenCL device is initialized and ready to run*/
	static bool IsOpenCLOk();
	~OpenCLShared();

private:
	OpenCLShared(){;}

	// OpenCL stuff for internal control
	static cl_platform_id platformId;
	static cl_device_id deviceId;
	static cl_context context;

	// set this flag if the OpenCL device is ok and running
	static bool isOk; 
};



#endif // __OPENCLSHARED_CLASS__