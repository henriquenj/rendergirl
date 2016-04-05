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
	OCLDevice(cl_device_id id);
	~OCLDevice();
	// Create context within this device, return false if there was an error
	bool CreateContext();

	// Release this context from use, freeing all the memory and resources used in the process
	void ReleaseContext();
	
	// return name of this device
	inline std::string GetName()const
	{
		return m_name;
	}

	// return name of this device as a read-only C string
	inline const char* GetCName()const
	{
		return m_name.c_str();
	}

	// get vendor of this vendor
	inline std::string GetVendor()const
	{
		return m_vendor;
	}
	// get version of software on this device
	inline std::string GetVersion()const
	{
		return m_version;
	}
	// get extensions of this aviable on this device
	inline std::string GetExtensions()const
	{
		return m_extensions;
	}
	// get OpenCL version
	inline std::string GetClVersion()const
	{
		return m_cldriverVersion;
	}
	// Get memory size of this device in BYTES
	inline cl_ulong GetMemSize()const
	{
		return m_memSize;
	}

	// get clock of this device in MHz
	inline cl_uint GetClock()const
	{
		return m_clock;
	}

	// Get number of OpenCL cores on this platform
	inline cl_uint GetCLCores()const
	{
		return m_clCores;
	}

	// Get maximum number of work itens in a work group (additional limitations may apply)
	inline size_t GetMaxWorkItens()const
	{
		return m_maxWorkItens;
	}
	// return if this device is ready for execute (with a context and all the rest)
	inline bool IsReady()const
	{
		return m_isReady;
	}
	// get cl_device_id of this device
	inline const cl_device_id GetID()const
	{
		return m_id;
	}
	// get context associated with this device
	inline OCLContext* GetContext()
	{
		return &m_context;
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
	std::string GetStringFromDevice(cl_device_info name)const;
	cl_ulong GetULongFromDevice(cl_device_info name) const;
	cl_uint GetUIntFromDevice(cl_device_info name) const;
	size_t GetSizeTFromDevice(cl_device_info name) const;

	// a device is considered ready when there's a context ready to use
	bool m_isReady;

	cl_device_id m_id;
	// information about the device
	DeviceType m_type;
	std::string m_name;
	std::string m_vendor;
	std::string m_version;
	std::string m_extensions;
	std::string m_cldriverVersion;

	// size of global memory in bytes
	cl_ulong m_memSize;
	// clock of this device
	cl_uint m_clock;
	//maximum number of opencl cores
	cl_uint m_clCores;
	// maximum number of work itens in a work group (additional limitations may apply)
	size_t m_maxWorkItens;

	// context inside this device
	OCLContext m_context;
	
};



#endif // __OCLDEVICE_CLASS__