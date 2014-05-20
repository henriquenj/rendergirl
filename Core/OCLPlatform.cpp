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

#include "OCLPlatform.h"


OCLPlatform::~OCLPlatform()
{
	//dealloc devices
	int size = m_devices.size();
	for (unsigned int a = 0; a < size; a++)
		delete m_devices[a];

	m_devices.clear();
}
OCLPlatform::OCLPlatform(cl_platform_id id)
{
	this->m_id = id;

	// query and store info about the platform
	m_profile = GetStringFromPlatform(CL_PLATFORM_PROFILE);
	m_version = GetStringFromPlatform(CL_PLATFORM_VERSION);
	m_name = GetStringFromPlatform(CL_PLATFORM_NAME);
	m_vendor = GetStringFromPlatform(CL_PLATFORM_VENDOR);
	m_extensions = GetStringFromPlatform(CL_PLATFORM_EXTENSIONS);

	// print
	Log::Message("Name: " + m_name);
	Log::Message("Vendor: " + m_vendor);
	Log::Message("Version: " + m_version);
	Log::Message("Platform profile: " + m_profile);
	if (!m_extensions.empty())
		Log::Message("Extensions: " + m_extensions);
	else Log::Message("No known extensions");
}

bool OCLPlatform::InitDevices(OCLDevice::DeviceType type)
{

	// delete previous devices, if there's any
	if (!m_devices.empty())
	{
		int size = m_devices.size();
		for (unsigned int a = 0; a < size; a++)
			delete m_devices[a];

		m_devices.clear();
	}


	bool allOk = true;

	Log::Message("");
	Log::Message("Retrieving devices on platform " + m_name);

	cl_uint deviceSize;

	// query number of devices for this platform
	if (clGetDeviceIDs(m_id, type, 0, 0, &deviceSize) != CL_SUCCESS)
	{
		Log::Error("Couldn't retrieve devices on platform " + m_name);
		return false;
	}

	cl_device_id* devices_cl = new cl_device_id[deviceSize];
	// query all devices
	if (clGetDeviceIDs(m_id, type, deviceSize, devices_cl, &deviceSize) != CL_SUCCESS)
	{
		Log::Error("Couldn't retrieve devices on platform " + m_name);
		return false;
	}

	for (unsigned int a = 0; a < deviceSize; a++)
	{
		Log::Message("");
		Log::Message("Device " + std::to_string(a + 1) + " created on platform " + m_name);
		OCLDevice* device = new OCLDevice(devices_cl[a]);
		m_devices.push_back(device);
	}


	delete[] devices_cl;
	return allOk;
}

const std::string OCLPlatform::GetStringFromPlatform(cl_platform_info name)
{
	size_t size;
	if (!m_id || clGetPlatformInfo(m_id, name, 0, 0, &size) != CL_SUCCESS)
	{
		return std::string();
	}
	std::string info;
	info.resize(size);
	clGetPlatformInfo(m_id, name, size, (void*)info.data(), &size);

	return info;
}
