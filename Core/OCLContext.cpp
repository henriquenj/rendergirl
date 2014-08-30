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

#include "OCLContext.h"
#include "OCLDevice.h"


// callback function to capture errors on this context
static void __stdcall ImplementationError(const char* errinfo, const void* private_info, size_t cb, void* user_data)
{
	OCLContext* context = (OCLContext*)user_data;
	Log::Error("OpenCL context on device " + context->GetDevice()->GetName() + " report the following error: " + errinfo);
}

bool OCLContext::InitContext(const OCLDevice *device)
{
	m_isReady = false;
	this->m_device = device;

	Log::Message("");
	Log::Message("Creating context on device "  + device->GetName());
	// get platform 
	cl_platform_id platform;
	clGetDeviceInfo(device->GetID(), CL_DEVICE_PLATFORM, sizeof(cl_device_id), &platform, NULL);
	cl_context_properties props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	cl_int error = CL_SUCCESS;

	cl_device_id id_t = device->GetID();
	m_context = clCreateContext(props, 1, &id_t, ImplementationError, this, &error);

	if (error != CL_SUCCESS)
	{
		if (error == CL_OUT_OF_HOST_MEMORY)
		{
			Log::Error("There's not enough memory on the host to alloc the OpenCL implementation.");
			return false;
		}
		else // some other error
		{
			Log::Error("Couldn't create a context inside " + device->GetName());
			return false;
		}
	}

	m_queue = clCreateCommandQueue(m_context, device->GetID(), NULL, &error);

	if (error != CL_SUCCESS)
	{
		if (error == CL_OUT_OF_HOST_MEMORY)
		{
			Log::Error("There's not enough memory on the host to alloc the OpenCL implementation.");
			return false;
		}
		else // some other error
		{
			Log::Error("Couldn't create a command queue inside " + device->GetName());
			return false;
		}
	}

	Log::Message("Context was created without errors.");

	m_isReady = true;
	return true;
}

bool OCLContext::ExecuteCommands()
{
	assert(m_isReady && "You cannot flush a queue if the context is not ready!");

	// the flush!
	cl_int error = clFinish(m_queue);
	if (error != CL_SUCCESS)
	{
		std::string errorString("Failed to flush the command queue on the device " + m_device->GetName() + ": ");
		if (error == CL_OUT_OF_RESOURCES)
			errorString += "CL_OUT_OF_HOST_MEMORY";
		else if (error == CL_INVALID_COMMAND_QUEUE)
			errorString += "CL_INVALID_COMMAND_QUEUE";
		else
			errorString += "unknown error";
		
		Log::Error(errorString);
		return false;
	}

	return true;
}

bool OCLContext::SyncAllMemoryDeviceToHost()
{
	bool error = true;
	std::list<OCLMemoryObjectBase*>::iterator it;
	for (it = m_memList.begin(); it != m_memList.end(); it++)
	{
		if (!(*it)->SyncDeviceToHost())
		{
			error = false;
		}
	}

	return error;
}

bool OCLContext::SyncAllMemoryHostToDevice()
{
	bool error = true;
	std::list<OCLMemoryObjectBase*>::iterator it;
	for (it = m_memList.begin(); it != m_memList.end(); it++)
	{
		if (!(*it)->SyncHostToDevice())
		{
			error = false;
		}
	}

	return error;
}

void OCLContext::ReleaseContext()
{
	// dealloc all memory associated with this device
	std::list<OCLMemoryObjectBase*>::iterator it;
	for (it = m_memList.begin(); it != m_memList.end(); it++)
	{
		delete *it;
	}
	m_memList.clear();

	// release OpenCL stuff
	clReleaseCommandQueue(m_queue);
	clReleaseContext(m_context);
	/* Deleting the queue after deleting the context is considered a memory leak
		by gDEBugger, weird */

	m_isReady = false;
}