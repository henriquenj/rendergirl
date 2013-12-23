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


#include "OCLMemoryObject.h"

template <class T>
OCLMemoryObject<T>::OCLMemoryObject(OCLContext* context, cl_command_queue* queue, 
									int size, bool &error, MemoryType type)
{
	assert(size > 0 && "Size should be at least 1!");
	error = false;

	this->context = context;
	this->queue = queue;
	this->size = size;

	cl_int error = CL_SUCCESS;

	// create OpenCL memory
	data_device = clCreateBuffer(context, type, size * sizeof(T), NULL, &error);

	if (error != CL_SUCCESS)
	{
		Log::Error("Couldn't alloc enough memory on " + context->GetDevice()->GetName() + " device");
		error = true;
		return;
	}

	// reserve the space required
	data_host = new T[size];
}

template <class T>
bool OCLMemoryObject<T>::SyncHostToDevice()
{
	// add an command to the current command queue
	/* I'll use a NON BLOCKING call for now, that means it's NOT safe to use data on host after calling this,
		I'm using that way just to try to improve the speed of the program
		TODO: test with blocking calls to see if there's a difference in performance
	*/
	if (clEnqueueWriteBuffer(queue, data_device, CL_FALSE, 0, sizeof(T)* size, data_host, NULL, NULL) != CL_SUCCESS)
	{
		Log::Error("Couldn't alloc enough memory on " + context->GetDevice()->GetName() + " device");
		return false;
	}

	return true;
}

template <class T>
bool OCLMemoryObject<T>::SyncDeviceToHost()
{
	if (clEnqueueReadBuffer(queue, data_device, CL_TRUE, 0, sizeof(T)* size, data_host, NULL, NULL) != CL_SUCCESS)
	{
		Log::Error("Couldn't read the memory on " + context->GetDevice()->GetName() + " device");
		return false;
	}

	return true;
}

template <class T>
OCLMemoryObject<T>::~OCLMemoryObject()
{
	// dealloc resources and free memory on both host and device
	delete[] data_host;
	clReleaseMemObject(data_device);
}


template <class T>
void OCLMemoryObject<T>::SetData(T* data)
{
	// copy data
	memcpy(data_host, data, sizeof(T) * size);
}