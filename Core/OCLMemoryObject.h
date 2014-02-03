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


#ifndef __OCLMEMORYOBJECT_CLASS__
#define __OCLMEMORYOBJECT_CLASS__

#include "CL\cl.h"
#include <assert.h>
#include "Log.h"


/* I know that there're more types, for now I'll only use these*/
enum MemoryType
{
	ReadWrite = CL_MEM_READ_WRITE,
	ReadOnly = CL_MEM_READ_ONLY,
	WriteOnly = CL_MEM_WRITE_ONLY
};


class OCLContext;

// to fit into a std::list
// TODO: check if there's a better way to handle this
class OCLMemoryObjectBase
{
public:
	OCLMemoryObjectBase(){ ; }
	virtual ~OCLMemoryObjectBase(){};
	virtual bool SyncHostToDevice() = 0;
	virtual bool SyncDeviceToHost() = 0;
};

/* Memory object class stores any kind of data into an OpenCL device, contains methods for transferring data
	between device memory and host memory. Those objects are created by a OCLContext only */
template <class T>
class OCLMemoryObject : public OCLMemoryObjectBase
{
public:
	
	/* Copy data to this object; since it's being copied, you are free to delete afterwards.
		Bear in mind that this function WON'T transfer the data to the device, call the sync functions to do that. 
		If there's another data previously loaded, the old one will be DELETED*/
	void SetData(const T* data)
	{
		memorySet = true;
		// copy data
		memcpy(data_host, data, sizeof(T)* size);
	}
	/* Get raw data currently on the host memory */
	inline const T* GetData()const
	{
		return data_host;
	}

	/* Get pointer to device memory */
	inline const cl_mem GetDeviceMemory()const
	{
		return data_device;
	}

	/* Sync functions*/

	/* Copy memory from host to device. WARNING: the task will only be completed when the current command queue is flushed,
		call ExecuteCommands on OCLContext to guarantee a copy, 
		you CAN'T use the data right after calling this (only after the queue is flushed)
		return false if the allocation failed
		*/
	bool SyncHostToDevice()
	{
		assert(memorySet && "You must set this memory before syncing with the device");
		// add an command to the current command queue
		/* I'll use a NON BLOCKING call for now, that means it's NOT safe to use data on host after calling this,
		I'm using that way just to try to improve the speed of the program
		TODO: test with blocking calls to see if there's a difference in performance
		*/
		if (clEnqueueWriteBuffer(queue, data_device, CL_FALSE, 0, sizeof(T)* size, data_host,0, NULL, NULL) != CL_SUCCESS)
		{
			Log::Error("Couldn't alloc enough memory on " + context->GetDevice()->GetName() + " device");
			return false;
		}

		return true;
	}
	/* Copy memory from device to host, this causes an intrinsic flush on the command queue,
		return false if the allocation failed*/
	bool SyncDeviceToHost()
	{
		if (clEnqueueReadBuffer(queue, data_device, CL_TRUE, 0, sizeof(T)* size, data_host,0, NULL, NULL) != CL_SUCCESS)
		{
			Log::Error("Couldn't read the memory on " + context->GetDevice()->GetName() + " device");
			return false;
		}

		return true;
	}

	~OCLMemoryObject()
	{
		// dealloc resources and free memory on both host and device
		clReleaseMemObject(data_device);
		delete[] data_host;
	}

private:

	/* Only OCLContext is able to create those objects
	size is the number of elements, NOT the size in bytes*/
	OCLMemoryObject(OCLContext* context, cl_command_queue queue, int size, MemoryType type, cl_bool* error)
	{
		assert(size > 0 && "Size should be at least 1!");
		cl_int l_error = false;

		this->context = context;
		this->queue = queue;
		this->size = size;


		// create OpenCL memory
		data_device = clCreateBuffer((context->GetCLContext()), type, size * sizeof(T), NULL, &l_error);

		if (l_error != CL_SUCCESS)
		{
			Log::Error("Couldn't alloc enough memory on " + context->GetDevice()->GetName() + " device");
			if (error != NULL) // set error flag
				*error = true;
			return;
		}

		// set error flag
		if (error != NULL)
			*error = false;

		// reserve the space required
		data_host = new T[size];

		memorySet = false;
	}

	friend class OCLContext;

	// context to which this block of data is associated
	OCLContext* context;
	cl_command_queue queue;

	// raw host data
	T* data_host;
	// raw device data
	cl_mem data_device;
	// number of elements 
	int size;

	// control if the memory has been at least once before syncronizing, useful only in debug
	bool memorySet;

};



#endif // __OCLMEMORYOBJECT_CLASS__