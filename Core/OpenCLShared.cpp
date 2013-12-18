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

cl_context					OpenCLShared::context;
cl_device_id				OpenCLShared::deviceId;
std::vector<OCLPlatform>	OpenCLShared::platformIds;

bool OpenCLShared::InitPlatforms()
{
	// do not query for new platforms if it has queried before
	assert(platformIds.empty());

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
	
	cl_platform_id* platforms = new cl_platform_id[platSize];

	// now the real query
	if (clGetPlatformIDs(platSize, platforms, &platSize) != CL_SUCCESS)
	{
		Log::Error("OpenCL platform could not be created, please check the OpenCL drivers for your device");
		return false;
	}

	// put all in the platforms vector
	for (int a = 0; a < platSize; a++)
	{
		OCLPlatform platform;
		platform.id = platforms[a];
		platformIds.push_back(platform);
		// name will be filled later
	}

	Log::Message(std::to_string(platSize) + " platforms were created.");

	//for every platform, print information
	int size = platformIds.size();
	for (int a = 0; a < size; a++)
	{
		Log::Message("");
		Log::Message("Platform " + std::to_string(a+1) + " created using the specs below: ");
		std::string platProfile;
		std::string platVersion;
		std::string platName;
		std::string platVendor;
		std::string platExt;

		// query info about the platform
		platProfile = OpenCLShared::GetStringFromPlatform(platformIds[a].id, CL_PLATFORM_PROFILE);
		platVersion = OpenCLShared::GetStringFromPlatform(platformIds[a].id, CL_PLATFORM_VERSION);
		platName = OpenCLShared::GetStringFromPlatform(platformIds[a].id, CL_PLATFORM_NAME);
		platVendor = OpenCLShared::GetStringFromPlatform(platformIds[a].id, CL_PLATFORM_VENDOR);
		platExt = OpenCLShared::GetStringFromPlatform(platformIds[a].id, CL_PLATFORM_EXTENSIONS);

		// print
		Log::Message("Name: " + platName);
		Log::Message("Vendor: " + platVendor);
		Log::Message("Version: " + platVersion);
		Log::Message("Platform profile: " + platProfile);
		if (!platExt.empty())
			Log::Message("Extensions: " + platExt);
		else Log::Message("No known extensions");

		// update name on vector
		platformIds[a].name = platName;
	}
	
	return true;
}

const std::string OpenCLShared::GetStringFromPlatform(cl_platform_id id, cl_platform_info name)
{
	size_t size;
	if (!id || clGetPlatformInfo(id, name, 0, 0, &size) != CL_SUCCESS)
	{
		return std::string();
	}
	std::string info;
	info.resize(size);
	clGetPlatformInfo(id, name, size, (void*)info.data(), &size);

	return info;
}

OpenCLShared::~OpenCLShared()
{
}