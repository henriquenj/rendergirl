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

#ifndef __OCLDEVICE_CLASS__
#define __OCLDEVICE_CLASS__

#include <string>
#include <assert.h>

#include "CL\cl.h"
#include "Log.h"
#include "OCLContext.h"


/* OCLDevice class encapsules the OpenCL device information */
class OCLDevice
{
public:
	// Init device and put it ready to receive a context
	void InitDevice(cl_device_id id);
	// Create context within this device, return false if there was an error
	bool CreateContext();

	// Release this context from use, freeing all the memory and resources used in the process
	void ReleaseContext();
	
	// return name of this device
	inline const std::string& GetName()const
	{
		return name;
	}
	// get vendor of this vendor
	inline const std::string& GetVendor()const
	{
		return vendor;
	}
	// get version of software on this device
	inline const std::string& GetVersion()const
	{
		return version;
	}
	// get extensions of this aviable on this device
	inline const std::string& GetExtensions()const
	{
		return extensions;
	}
	// get OpenCL version
	inline const std::string& GetClVersion()const
	{
		return cldriverVersion;
	}
	// Get memory size of this device in BYTES
	inline const cl_ulong GetMemSize()const
	{
		return memSize;
	}

	// get clock of this device in MHz
	inline const cl_uint GetClock()const
	{
		return clock;
	}

	// Get number of OpenCL cores on this platform
	inline const cl_uint GetCLCores()const
	{
		return clCores;
	}

	// Get maximum number of work itens in a work group (additional limitations may apply)
	inline const size_t GetMaxWorkItens()const
	{
		return maxWorkItens;
	}
	// return if this device is ready for execute (with a context and all the rest)
	inline const bool IsReady()const
	{
		return isReady;
	}
	// get cl_device_id of this device
	inline const cl_device_id GetID()const
	{
		return id;
	}
	// get context associated with this device
	inline OCLContext* GetContext()
	{
		return &context;
	}


	// types of devices accepted by OpenCL
	enum DeviceType
	{
		Default = CL_DEVICE_TYPE_DEFAULT,
		CPU = CL_DEVICE_TYPE_CPU,
		GPU = CL_DEVICE_TYPE_GPU,
		Accelerator = CL_DEVICE_TYPE_ACCELERATOR,
		All = CL_DEVICE_TYPE_ALL
	};


private:
	// helper function to query information about devices (thanks QT project for this hint)
	const std::string GetStringFromDevice(cl_device_info name)const;
	const cl_ulong GetULongFromDevice(cl_device_info name) const;
	const cl_uint GetUIntFromDevice(cl_device_info name) const;
	const size_t GetSizeTFromDevice(cl_device_info name) const;

	// a device is considered ready when there's a context ready to use
	bool isReady;

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
	// maximum number of work itens in a work group (additional limitations may apply)
	size_t maxWorkItens;

	// context inside this device
	OCLContext context;
	
};



#endif // __OCLDEVICE_CLASS__