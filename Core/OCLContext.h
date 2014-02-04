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


#ifndef __OCLCONTEXT_CLASS__
#define __OCLCONTEXT_CLASS__

#include <list>

#include "CL\cl.h"
#include "OCLMemoryObject.h"
#include "OCLProgram.h"

class OCLDevice;

/* OCLContext class encapsules a context object, managing kernels and command queues, 
	also alloc memory on the device */
class OCLContext
{
public:
	// Init a context inside a given device, return false if there was an error
	bool InitContext(OCLDevice *device);

	// Release this context, freeing all the memory associated with it
	void ReleaseContext();

	/* alloc a memory object on this context, size in the amount of elements, NOT the size in bytes.
		return error = true if there's an error upon allocating*/
	template <class T>
	OCLMemoryObject<T>* CreateMemoryObject(const int size,const MemoryType type = ReadWrite,cl_bool *error = NULL)
	{
		OCLMemoryObject<T>* newMem = new OCLMemoryObject<T>(this, queue, size, type, error);
		memList.push_back(newMem);

		return newMem;
	}
	/* alloc a memory object on this context. Data is a pointer to a block of data,
	copy parameter define if the data will be copied or just the pointer
	size in the amount of elements, NOT the size in bytes.
	return error = true if there's an error upon allocating*/
	template<class T>
	OCLMemoryObject<T>* CreateMemoryObjectWithData(const int size,T* data,bool copy = true,
									const MemoryType type = ReadWrite,cl_bool *error = NULL)
	{
		OCLMemoryObject<T>* newMem =  this->CreateMemoryObject<T>(size, type, error);
		newMem->SetData(data,copy);
		return newMem;
	}

	/* delete a memory object associated with this context */
	template <class T>
	void DeleteMemoryObject(OCLMemoryObject<T>* memObject)
	{
		// check if the memory belongs to this context
		assert((std::find(memList.begin(), memList.end(), memObject) != memList.end()) 
				&& "This piece of memory is not part of this context!");
		delete memObject;
		memList.remove(memObject);
	}

	/* Call SyncDeviceToHost on all memories associated with this context. Return FALSE if at least
		one memory syncronization failed */
	bool SyncAllMemoryDeviceToHost();
	/* Call SyncHostToDevice on all memories associated with this context. Return FALSE if at 
		least one memory syncronization failed */
	bool SyncAllMemoryHostToDevice();


	// Get device where this context is running
	inline const OCLDevice* GetDevice()const
	{
		return device;
	}
	// Get OpenCL context
	inline const cl_context GetCLContext()const
	{
		return context;
	}
	// Get OpenCL command queue of this context
	inline const cl_command_queue GetCLQueue()
	{
		return queue;
	}
	// Is this context ready to receive kernels?
	inline const bool IsReady()const
	{
		return isReady;
	}
	/* execute all commands on the command queue (call a clFlush), this is a blocking call.
		return TRUE for sucess and FALSE for an error */
	bool ExecuteCommands();

private:

	// the device which this context is running
	OCLDevice* device;
	// internal pointer to cl context
	cl_context context;
	// internal pointer to cl command queue
	cl_command_queue queue;
	/*	I'll be using only one command queue for each context to keep things simple, 
		maybe later I'll implement a command queue class and share it among contexts */


	// is this context ready to receive kernels?
	bool isReady;

	// list of memories associated with this context
	std::list<OCLMemoryObjectBase*> memList;
};


#endif //__OCLCONTEXT_CLASS__