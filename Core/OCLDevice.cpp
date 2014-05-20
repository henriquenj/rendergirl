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


#include "OCLDevice.h"
#include "UtilitiesFuncions.h"

OCLDevice::OCLDevice(cl_device_id id)
{
	m_isReady = false;
	this->m_id = id;
	// query type
	clGetDeviceInfo(id, CL_DEVICE_TYPE, sizeof(cl_device_type), &m_type, 0);

	// print information about device
	m_name =		GetStringFromDevice(CL_DEVICE_NAME);
	m_vendor =	GetStringFromDevice(CL_DEVICE_VENDOR);
	m_version = GetStringFromDevice(CL_DEVICE_VERSION);
	m_cldriverVersion = GetStringFromDevice(CL_DRIVER_VERSION);
	m_extensions = GetStringFromDevice(CL_DEVICE_EXTENSIONS);

	m_memSize = GetULongFromDevice(CL_DEVICE_GLOBAL_MEM_SIZE);
	m_clock = GetUIntFromDevice(CL_DEVICE_MAX_CLOCK_FREQUENCY);
	m_clCores = GetUIntFromDevice(CL_DEVICE_MAX_COMPUTE_UNITS);
	m_maxWorkItens = GetSizeTFromDevice(CL_DEVICE_MAX_WORK_GROUP_SIZE);

	

	// type as a string
	std::string type_s;
	switch (m_type)
	{
	case Accelerator:
		type_s = "Accelerator";
		break;
	case CPU:
		type_s = "CPU";
		break;
	case GPU:
		type_s = "GPU";
		break;
	}

	// intel creates some empty spaces on the beginning of the string that I want to remove
	// must pay attention to other kinds of devices as well
	m_name = TrimString(m_name);

	Log::Message("Name: " + m_name);
	Log::Message("Vendor: " + m_vendor);
	Log::Message("Version: " + m_version);
	Log::Message("Type: " + type_s);
	Log::Message("Driver Version: " + m_cldriverVersion);
	Log::Message("Memory: " + std::to_string(m_memSize / 1048576) + "MB");
	Log::Message("Clock: " + std::to_string(m_clock) + "MHz");
	Log::Message("Max OpenCL Cores: " + std::to_string(m_clCores));
	Log::Message("Max work itens: " + std::to_string(m_maxWorkItens));
	//Log::Message("Extensions: " + extensions);

}

OCLDevice::~OCLDevice()
{
	// clean this just in case hasn't been cleaned
	if (m_isReady)
		m_context.ReleaseContext();
}

void OCLDevice::ReleaseContext()
{
	assert(m_isReady); 
	// make sure there's a context
	if (m_isReady)
	{
		m_context.ReleaseContext();
		m_isReady = false;
	}
}

bool OCLDevice::CreateContext()
{
	assert((!m_context.IsReady()) && "This device already has a working context!");

	// init context
	if (!m_context.InitContext(this))
	{
		// something went wrong
		return false;
	}

	m_isReady = true;
	return true;
}

const std::string OCLDevice::GetStringFromDevice(cl_device_info name)const
{
	size_t size;
	if (!m_id || clGetDeviceInfo(m_id, name, 0, 0, &size) != CL_SUCCESS)
	{
		return std::string();
	}
	std::string info;
	info.resize(size);
	clGetDeviceInfo(m_id, name, size, (void*)info.data(), &size);
	info.pop_back(); // remove the null terminating char that was killing my strings

	return info;
}


const cl_ulong OCLDevice::GetULongFromDevice(cl_device_info name)const
{
	cl_ulong value;
	clGetDeviceInfo(m_id, name, sizeof(value), &value, 0);

	return value;
}

const cl_uint OCLDevice::GetUIntFromDevice(cl_device_info name)const
{
	cl_uint value;
	clGetDeviceInfo(m_id, name, sizeof(value), &value, 0);

	return value;
}

const size_t OCLDevice::GetSizeTFromDevice(cl_device_info name)const
{
	size_t value;
	clGetDeviceInfo(m_id, name, sizeof(value), &value, 0);

	return value;
}