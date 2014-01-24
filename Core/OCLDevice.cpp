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


#include "OCLDevice.h"
#include "UtilitiesFuncions.h"

void OCLDevice::InitDevice(cl_device_id id)
{
	isReady = false;
	this->id = id;
	// query type
	clGetDeviceInfo(id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, 0);

	// print information about device
	name =		GetStringFromDevice(CL_DEVICE_NAME);
	vendor =	GetStringFromDevice(CL_DEVICE_VENDOR);
	version = GetStringFromDevice(CL_DEVICE_VERSION);
	cldriverVersion = GetStringFromDevice(CL_DRIVER_VERSION);
	extensions = GetStringFromDevice(CL_DEVICE_EXTENSIONS);

	memSize = GetULongFromDevice(CL_DEVICE_GLOBAL_MEM_SIZE);
	clock = GetUIntFromDevice(CL_DEVICE_MAX_CLOCK_FREQUENCY);
	clCores = GetUIntFromDevice(CL_DEVICE_MAX_COMPUTE_UNITS);
	maxWorkItens = GetSizeTFromDevice(CL_DEVICE_MAX_WORK_GROUP_SIZE);

	

	// type as a string
	std::string type_s;
	switch (type)
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
	name = TrimString(name);

	Log::Message("Name: " + name);
	Log::Message("Vendor: " + vendor);
	Log::Message("Version: " + version);
	Log::Message("Type: " + type_s);
	Log::Message("Driver Version: " + cldriverVersion);
	Log::Message("Memory: " + std::to_string(memSize / 1048576) + "MB");
	Log::Message("Clock: " + std::to_string(clock) + "MHz");
	Log::Message("Max OpenCL Cores: " + std::to_string(clCores));
	Log::Message("Max work itens: " + std::to_string(maxWorkItens));
	//Log::Message("Extensions: " + extensions);

}

void OCLDevice::ReleaseContext()
{
	assert(isReady);

	// make sure there's a context
	if (isReady)
	{
		context.ReleaseContext();
	}
}

bool OCLDevice::CreateContext()
{
	assert((!context.IsReady()) && "This device already has a working context!");

	// init context
	if (!context.InitContext(this))
	{
		// something went wrong
		return false;
	}

	isReady = true;
	return true;
}

const std::string OCLDevice::GetStringFromDevice(cl_device_info name)const
{
	size_t size;
	if (!id || clGetDeviceInfo(id, name, 0, 0, &size) != CL_SUCCESS)
	{
		return std::string();
	}
	std::string info;
	info.resize(size);
	clGetDeviceInfo(id, name, size, (void*)info.data(), &size);

	return info;
}


const cl_ulong OCLDevice::GetULongFromDevice(cl_device_info name)const
{
	cl_ulong value;
	clGetDeviceInfo(id, name, sizeof(value), &value, 0);

	return value;
}

const cl_uint OCLDevice::GetUIntFromDevice(cl_device_info name)const
{
	cl_uint value;
	clGetDeviceInfo(id, name, sizeof(value), &value, 0);

	return value;
}

const size_t OCLDevice::GetSizeTFromDevice(cl_device_info name)const
{
	size_t value;
	clGetDeviceInfo(id, name, sizeof(value), &value, 0);

	return value;
}