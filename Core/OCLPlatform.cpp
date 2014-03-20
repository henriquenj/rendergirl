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

#include "OCLPlatform.h"


OCLPlatform::~OCLPlatform()
{
	devices.clear();
}
void OCLPlatform::Init(cl_platform_id id)
{
	this->id = id;

	// query and store info about the platform
	profile = GetStringFromPlatform(CL_PLATFORM_PROFILE);
	version = GetStringFromPlatform(CL_PLATFORM_VERSION);
	name = GetStringFromPlatform(CL_PLATFORM_NAME);
	vendor = GetStringFromPlatform(CL_PLATFORM_VENDOR);
	extensions = GetStringFromPlatform(CL_PLATFORM_EXTENSIONS);

	// print
	Log::Message("Name: " + name);
	Log::Message("Vendor: " + vendor);
	Log::Message("Version: " + version);
	Log::Message("Platform profile: " + profile);
	if (!extensions.empty())
		Log::Message("Extensions: " + extensions);
	else Log::Message("No known extensions");
}

bool OCLPlatform::InitDevices(OCLDevice::DeviceType type)
{
	bool allOk = true;

	Log::Message("");
	Log::Message("Retrieving devices on platform " + name);

	cl_uint deviceSize;

	// query number of devices for this platform
	if (clGetDeviceIDs(id, type, 0, 0, &deviceSize) != CL_SUCCESS)
	{
		Log::Error("Couldn't retrieve devices on platform " + name);
		return false;
	}

	cl_device_id* devices_cl = new cl_device_id[deviceSize];
	// query all devices
	if (clGetDeviceIDs(id, type, deviceSize, devices_cl, &deviceSize) != CL_SUCCESS)
	{
		Log::Error("Couldn't retrieve devices on platform " + name);
		return false;
	}

	for (unsigned int a = 0; a < deviceSize; a++)
	{
		Log::Message("");
		Log::Message("Device " + std::to_string(a + 1) + " created on platform " + name);
		OCLDevice device;
		device.InitDevice(devices_cl[a]);
		devices.push_back(device);
	}


	delete[] devices_cl;
	return allOk;
}

const std::string OCLPlatform::GetStringFromPlatform(cl_platform_info name)
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
