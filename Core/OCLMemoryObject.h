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


#ifndef __OCLMEMORYOBJECT_CLASS__
#define __OCLMEMORYOBJECT_CLASS__

#include "CL\cl.h"
#include <assert.h>
#include <algorithm>
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
		if (m_data_host != NULL)
			delete[] m_data_host; // delete old content

		if (copy)
		{
			m_data_host = new T[m_size];
			// copy data
			memcpy(m_data_host, data, sizeof(T)* m_size);
		}
		else
		{
			// copy pointer only
			m_data_host = data;
		}
	}
	/* Get raw data currently on the host memory */
	inline const T* GetData()const
	{
		return m_data_host;
	}

	/* Get number of elements on this memory */
	inline int GetSize()const
	{
		return m_size;
	}

	/* Get pointer to device memory */
	inline const cl_mem GetDeviceMemory()const
	{
		return m_data_device;
	}

	/* Copy memory from a given device buffer to this device buffer. 
		amount is the amount of elements to copy.
		source is the source buffer to be copied.
		sourceOffset and destOffset are the offset of the copy for source buffer and dest buffer,
		in the amount of elements, NOT the size in bytes.
		WARNING: the task will only be completed when the current command queue is flushed,
		call ExecuteCommands on OCLContext to guarantee a copy. Return FALSE if the copy failed */
	bool CopyFromMemoryBuffer(const OCLMemoryObject<T>* source, const int amount,
		const int sourceOffset = 0, const int destOffset = 0)
	{
		assert(source != NULL && "Parameter source cannot be NULL");
		assert(destOffset + amount <= m_size && "You can't copy more memory than the buffer size");
		assert(sourceOffset + amount <= source->GetSize() && "You can't copy more memory than the buffer size");

		cl_int error;

		error = clEnqueueCopyBuffer(m_queue, source->GetDeviceMemory(), m_data_device, sourceOffset * sizeof(T),
			destOffset * sizeof(T), amount * sizeof(T), NULL, NULL, NULL);

		if (error != CL_SUCCESS)
		{
			Log::Error("Couldn't copy memory on " + m_context->GetDevice()->GetName() + " device");
			return false;
		}

		return true;
		
	}

	/* Differs from the above function only in the arguments that receives. The amount of elements copied will be
		the size of the smallest buffer */
	inline bool CopyFromMemoryBuffer(const OCLMemoryObject<T>* source, const int sourceOffset = 0, const int destOffset = 0)
	{
		int amount = std::min(source->GetSize(), m_size);

		return this->CopyFromMemoryBuffer(source, amount, sourceOffset, destOffset);
	}

	/* Sync functions*/

	/* Copy memory from host to device. WARNING: the task will only be completed when the current command queue is flushed,
		call ExecuteCommands on OCLContext to guarantee a copy. Return FALSE if the allocation failed
		*/
	bool SyncHostToDevice()
	{
		assert(m_data_host != NULL && "You must set this memory before syncing with the device");
		// add an command to the current command queue
		//TODO: try using NON-blocking calls
		if (clEnqueueWriteBuffer(m_queue, m_data_device, CL_TRUE, 0, sizeof(T)* m_size, m_data_host,0, NULL, NULL) != CL_SUCCESS)
		{
			Log::Error("Couldn't alloc enough memory on " + m_context->GetDevice()->GetName() + " device");
			return false;
		}
		
		return true;
	}
	/* Copy memory from device to host, this causes an intrinsic flush on the command queue.
		return FALSE if the allocation failed*/
	bool SyncDeviceToHost()
	{
		if (clEnqueueReadBuffer(m_queue, m_data_device, CL_TRUE, 0, sizeof(T)* m_size, m_data_host,0, NULL, NULL) != CL_SUCCESS)
		{
			Log::Error("Couldn't read the memory on " + m_context->GetDevice()->GetName() + " device");
			return false;
		}

		return true;
	}

	inline T &operator[](const int index)
	{
		return m_data_host[index];
	}

private:

	/* Only OCLContext is able to create those objects
	size is the number of elements, NOT the size in bytes*/
	OCLMemoryObject(const OCLContext* context,const cl_command_queue queue,const int size,const MemoryType type,cl_bool* error)
	{
		assert(size > 0 && "Size should be at least 1!");
		assert(context != NULL);
		assert(context->IsReady() && " Context must be ready!");
		cl_int l_error = false;

		this->m_context = context;
		this->m_queue = queue;
		this->m_size = size;


		// create OpenCL memory
		m_data_device = clCreateBuffer((context->GetCLContext()), type, size * sizeof(T), NULL, &l_error);

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

		m_data_host = NULL;
	}

	/* prevent copy by not implementing those */
	OCLMemoryObject(OCLMemoryObject<T> const&);
	void operator=(OCLMemoryObject<T> const&);

	virtual ~OCLMemoryObject()
	{
		// dealloc resources and free memory on both host and device
		clReleaseMemObject(m_data_device);
		if (m_data_host != NULL)
			delete[] m_data_host;
	}

	friend class OCLContext;

	// context to which this block of data is associated
	const OCLContext* m_context;
	cl_command_queue m_queue;

	// raw host data
	T* m_data_host;
	// raw device data
	cl_mem m_data_device;
	// number of elements 
	int m_size;
};



#endif // __OCLMEMORYOBJECT_CLASS__