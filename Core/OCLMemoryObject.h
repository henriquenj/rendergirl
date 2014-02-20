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
	
	/* Set data on this memory. Parameter copy define if the data will be copied entirely or just the pointer;
		if just the pointer is copied, the class will DELETE it after using it.
		Bear in mind that this function WON'T transfer the data to the device, call the sync functions to do that. 
		If there's another data previously loaded, the old one will be DELETED*/
	void SetData(T* data, bool copy = true)
	{
		if (data_host != NULL)
			delete[] data_host; // delete old content

		if (copy)
		{
			data_host = new T[size];
			// copy data
			memcpy(data_host, data, sizeof(T)* size);
		}
		else
		{
			// copy pointer only
			data_host = data;
		}
	}
	/* Get raw data currently on the host memory */
	inline const T* GetData()const
	{
		return data_host;
	}

	/* Get number of elements on this memory */
	inline const int GetSize()const
	{
		return size;
	}

	/* Get pointer to device memory */
	inline const cl_mem GetDeviceMemory()const
	{
		return data_device;
	}

	/* Sync functions*/

	/* Copy memory from host to device. WARNING: the task will only be completed when the current command queue is flushed,
		call ExecuteCommands on OCLContext to guarantee a copy. Return FALSE if the allocation failed
		*/
	bool SyncHostToDevice()
	{
		assert(data_host != NULL && "You must set this memory before syncing with the device");
		// add an command to the current command queue
		//TODO: try using NON-blocking calls
		if (clEnqueueWriteBuffer(queue, data_device, CL_TRUE, 0, sizeof(T)* size, data_host,0, NULL, NULL) != CL_SUCCESS)
		{
			Log::Error("Couldn't alloc enough memory on " + context->GetDevice()->GetName() + " device");
			return false;
		}

		return true;
	}
	/* Copy memory from device to host, this causes an intrinsic flush on the command queue.
		return FALSE if the allocation failed*/
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
		if (data_host != NULL)
		delete[] data_host;
	}

	inline T &operator[](const int index)
	{
		return data_host[index];
	}

private:

	/* Only OCLContext is able to create those objects
	size is the number of elements, NOT the size in bytes*/
	OCLMemoryObject(const OCLContext* context,const cl_command_queue queue,const int size,const MemoryType type,cl_bool* error)
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

		data_host = NULL;
	}

	friend class OCLContext;

	// context to which this block of data is associated
	const OCLContext* context;
	cl_command_queue queue;

	// raw host data
	T* data_host;
	// raw device data
	cl_mem data_device;
	// number of elements 
	int size;

};



#endif // __OCLMEMORYOBJECT_CLASS__