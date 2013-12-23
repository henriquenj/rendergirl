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


#include <assert.h>

#include "OCLContext.h"
#include "Log.h"


/* Memory object class stores any kind of data into an OpenCL device, contains methods for transferring data
	between device memory and host memory. Those objects are created by a OCLContext only */
template <class T>
class OCLMemoryObject
{
public:
	/* Copy data to this object; since it's being copied, you are free to delete afterwards.
		Bear in mind that this function WON'T transfer the data to the device, call the sync functions to do that. 
		If there's another data previously loaded, the old one will be DELETED*/
	void SetData(T* data);
	/* Get raw data currently on the host memory */
	inline const T* GetData()const
	{
		return data_host;
	}

	/* Get pointer to device memory */
	inline const cl_mem* GetDeviceMemory()const
	{
		return data_device;
	}

	/* Sync functions*/

	/* Copy memory from host to device. WARNING: the task will only be completed when the current command queue is flushed,
		call ExecuteCommands on OCLContext to guarantee a copy, 
		you CAN'T use the data right after calling this (only after the queue is flushed)
		return false if the allocation failed
		*/
	bool SyncHostToDevice();
	/* Copy memory from device to host, this causes an intrinsic flush on the command queue,
		return false if the allocation failed*/
	bool SyncDeviceToHost();

	~OCLMemoryObject();


	/* I know that there're more types, for now I'll only use these*/
	enum MemoryType
	{
		ReadWrite = CL_MEM_READ_WRITE,
		ReadOnly = CL_MEM_READ_ONLY,
		WriteOnly = CL_MEM_WRITE_ONLY
	};

private:
	/* Only OCLContext is able to create those objects 
		size is the number of elements, NOT the size in bytes*/
	OCLMemoryObject(OCLContext* context, cl_command_queue* queue, int size, bool &error, MemoryType type = ReadWrite);

	friend class OCLContext;

	// context to which this block of data is associated
	OCLContext* context;
	cl_command_queue* queue;

	// raw host data
	T* data_host;
	// raw device data
	cl_mem data_device;
	// number of elements 
	int size;

};



#endif // __OCLMEMORYOBJECT_CLASS__