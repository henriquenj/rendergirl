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


#include "OpenCLShared.h"

cl_context		OpenCLShared::context;
cl_device_id	OpenCLShared::deviceId;
cl_platform_id	OpenCLShared::platformId;
bool			OpenCLShared::isOk = false;

void OpenCLShared::InitOpenCL(DeviceType deviceType)
{
	assert(!isOk); //prevent program from initializing OpenCL twice

	isOk = false;
	// create and init OpenCL

	Log::Message("Initializing OpenCL device...");
	// error handling
	cl_uint error = CL_SUCCESS;

	// create plataforms
	error = clGetPlatformIDs(1, &platformId, NULL);
	if (error != CL_SUCCESS)
	{
		Log::Error("OpenCL platform could not be created, please check the OpenCL drivers for your device");
		return;
	}

	Log::Message("");
	Log::Message("Platform created using the specs below: ");
	char platProfile[16];
	char platVersion[16];
	char platName[50];
	char platVendor[50];
	char platExt[50];
	// print information about platform
	clGetPlatformInfo(platformId, CL_PLATFORM_PROFILE, 16 * sizeof(char), platProfile, NULL);
	clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, 16 * sizeof(char), platVersion, NULL);
	clGetPlatformInfo(platformId, CL_PLATFORM_NAME, 50 * sizeof(char), platName, NULL);
	clGetPlatformInfo(platformId, CL_PLATFORM_VENDOR, 50 * sizeof(char), platVendor, NULL);
	error = clGetPlatformInfo(platformId, CL_PLATFORM_EXTENSIONS, 50 * sizeof(char), platExt, NULL);

	std::string platProfile_s("Platform profile: ");
	platProfile_s += platProfile;
	Log::Message(platProfile_s);

	std::string platVersion_s("Version: ");
	platVersion_s += platVersion;
	Log::Message(platVersion_s);

	std::string platName_s("Name: ");
	platName_s += platName;
	Log::Message(platName_s);

	std::string platVendor_s("Vendor: ");
	platVendor_s += platVendor;
	Log::Message(platVendor_s);

	if (error == CL_SUCCESS)
	{
		std::string platExt_s("Extensions: ");
		platExt_s += platExt;
		Log::Message(platExt_s);
	}
	else Log::Message("No known extensions");


	// ask for the given device (GPU, CPU, ...)
	//error = clGetDeviceIDs(platformId);
	
	
}

OpenCLShared::~OpenCLShared()
{
}