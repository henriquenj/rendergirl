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


#include "OCLContext.h"
#include "OCLDevice.h"

// callback function to capture errors on this context
static void ImplementationError(const char* errinfo, const void* private_info, size_t cb, void* user_data)
{
	OCLContext* context = (OCLContext*)user_data;
	Log::Error("OpenCL context on device " + context->GetDevice()->GetName() +
		" report the following error: " + errinfo);
}

bool OCLContext::InitContext(OCLDevice *device)
{
	isReady = false;
	this->device = device;

	Log::Message("");
	Log::Message("Creating context on " + device->GetName() + " device");
	// get platform 
	cl_platform_id platform;
	clGetDeviceInfo(device->GetID(), CL_DEVICE_PLATFORM, sizeof(cl_device_id), &platform, NULL);
	cl_context_properties props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	cl_int error = CL_SUCCESS;
	context = clCreateContext(props, 1, &device->GetID(), ImplementationError, this, &error);

	if (error != CL_SUCCESS)
	{
		if (error == CL_OUT_OF_HOST_MEMORY)
		{
			Log::Error("There's not enough memory on the host to alloc the OpenCL implementation.");
			return false;
		}
		else // some other error
		{
			Log::Error("Couldn't create a context inside " + device->GetName() + " device");
			return false;
		}
	}

	Log::Message("Context was created without errors.");
	Log::Message("Creating command queue on " + device->GetName());

	queue = clCreateCommandQueue(context, device->GetID(), NULL, &error);

	if (error != CL_SUCCESS)
	{
		if (error == CL_OUT_OF_HOST_MEMORY)
		{
			Log::Error("There's not enough memory on the host to alloc the OpenCL implementation.");
			return false;
		}
		else // some other error
		{
			Log::Error("Couldn't create a command queue inside " + device->GetName() + " device");
			return false;
		}
	}


	OCLMemoryObject<char>* charMemorie = this->CreateMemoryObject<char>(10);

	isReady = true;
	return true;
}

//template<class T>
//OCLMemoryObject<T>* OCLContext::CreateMemoryObject(int size, MemoryType type)
//{
//	OCLMemoryObject<T>* newMem = new OCLMemoryObject<T>(this, &queue, size, type);
//	memList.push_back(newMem);
//
//	return newMem;
//}

//template<class T>
//void OCLContext::DeleteMemoryObject(OCLMemoryObject<T>* memObject)
//{
//	delete memObject;
//	memList.remove(memObject);
//}