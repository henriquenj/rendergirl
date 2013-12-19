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


#ifndef __OCLDEVICE_CLASS__
#define __OCLDEVICE_CLASS__

#include <string>

#include "CL\cl.h"
#include "Log.h"

// types of devices accepted by OpenCL
enum DeviceType
{
	Default = CL_DEVICE_TYPE_DEFAULT,
	CPU = CL_DEVICE_TYPE_CPU,
	GPU = CL_DEVICE_TYPE_GPU,
	Accelerator = CL_DEVICE_TYPE_ACCELERATOR,
	All = CL_DEVICE_TYPE_ALL
};

/* OCLDevice class encapsules the OpenCL device information*/
class OCLDevice
{
public:
	// Init device and put it ready to receive a context
	void InitDevice(cl_device_id id);

private:
	// helper function to query information about devices (thanks QT project for this hint)
	const std::string GetStringFromDevice(cl_device_info name)const;
	const cl_ulong GetULongFromDevice(cl_device_info name) const;
	const cl_uint GetUIntFromDevice(cl_device_info name)const;

	cl_device_id id;
	// information about the device
	DeviceType type;
	std::string name;
	std::string vendor;
	std::string version;
	std::string extensions;
	std::string cldriverVersion;

	// size of global memory in bytes
	cl_ulong memSize;
	// clock of this device
	cl_uint clock;
	//maximum number of opencl cores
	cl_uint clCores;
	
};



#endif // __OCLDEVICE_CLASS__