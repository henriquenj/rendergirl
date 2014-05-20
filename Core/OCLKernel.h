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

#ifndef __OCLKERNEL_HEADER__
#define __OCLKERNEL_HEADER__

#include "OCLProgram.h"

/* OCLKernel class dispatches execution for OCLProgram objects, using data avaiable on a given OCLContext,
	must be instantiated using a valid OCLProgram*/
class OCLKernel
{
public:
	OCLKernel(OCLProgram* program,const std::string &name);
	~OCLKernel();

	/*Enqueue execution of this kernel in the current command queue. Warning: the is a non-blocking call that will only
		be executed when ExecuteCommands is called on the command queue. Return FALSE for fail on execution */
	bool EnqueueExecution();


	/* Set argument list for this kernel, kernel must be ready. object parameter is a properly allocated memory,
		index parameter is the index of the argument. Return FALSE for error */
	template<class T>
	bool SetArgument(const int index, const OCLMemoryObject<T> *object)
	{
		// lots of asserts
		assert(object != NULL && "Object cannot be null");
		assert(index < m_argumentSize && "index cannot be higher than argument size");

		const cl_mem memory = object->GetDeviceMemory();
		cl_int error = clSetKernelArg(m_kernel, index, sizeof(cl_mem), &memory);
		if (error != CL_SUCCESS)
		{
			Log::Error("Couldn't set kernel argument on " + m_name);
			return false;
		}

		return true;
	}

	// return FALSE is the kernel was not ok (probrably there's no such kernel in this progrm)
	inline bool GetOk()const
	{
		return m_kernelOk;
	}

	// Set the total amount of work-itens in all work-groups for this kernel
	inline void SetGlobalWorkSize(const size_t size)
	{
		assert(size > 0 && "Work itens should be bigger than 0!");
		m_globalWorkSize = size;
	}

	// get the total amount of work-itens in all work-groups for this kernel
	inline int const GetGlobalWorkSize() const
	{
		return m_globalWorkSize;
	}


private:
	// the program that this kernel will execute
	OCLProgram* m_program;
	// OpenCL kernel pointer
	cl_kernel m_kernel;
	// kernel name
	std::string m_name;
	// is this kernel Ok?
	bool m_kernelOk;
	// number of kernel arguments
	int m_argumentSize;

	/* variable to control kernel execution*/

	// the size of the work dimention, default 1 for now
	int m_workDim;
	// the total amount of work-itens in all work-groups
	size_t m_globalWorkSize;

};





#endif // __OCLKERNEL_HEADER__