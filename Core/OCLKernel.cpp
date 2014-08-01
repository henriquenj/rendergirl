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

#include "OCLKernel.h"
#include "OCLDevice.h"


OCLKernel::OCLKernel(OCLProgram* program,const std::string &name)
{
	assert(program != NULL);
	assert(program->IsCompiled() && "Must be a valid program object");
	assert(!name.empty() && "Kernel name should not be empty");

	this->m_program = program;
	this->m_name = name;

	cl_int error;
	m_kernel = clCreateKernel(program->GetCLProgram(), name.c_str(), &error);
	
	if (error != CL_SUCCESS)
	{
		Log::Error("There was an error while creating an OpenCL kernel.");
		if (error == CL_INVALID_KERNEL_NAME)
			Log::Error("There's no kernel called " + name);
		m_kernelOk = false;
		return;
	}

	clGetKernelInfo(m_kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), (void*)&m_argumentSize, NULL);

	// init variables to kernel dispatch
	m_workDim = 1;
	m_globalWorkSize = 1;

	m_kernelOk = true;
}

bool OCLKernel::EnqueueExecution()
{
	/* For now, I'll let OpenCL implementation decide the size of each work-group */

	cl_int error = CL_SUCCESS;

	assert(m_kernelOk && "Kernel must be ready to execution");

	error = clEnqueueNDRangeKernel(m_program->GetContext()->GetCLQueue(), m_kernel, m_workDim,
			NULL, // should always be NULL, this is from the OpenCL specification
			&m_globalWorkSize, // the total amount of threads (work-itens)
			NULL, // passing NULL on the size of the work-groups, OpenCL will hopefully pick the proper size
			0,NULL, NULL); // events syncronization stuff

	if (error != CL_SUCCESS)
	{
		std::string errorString = "Kernel being executed on " + m_program->GetContext()->GetDevice()->GetName()
			+ " report the following error: ";
		// print the error codes, I'm not taking care of all of them
		switch (error) // yeah I just copy-pasted all of them, I'm lazy
		{
		case CL_INVALID_KERNEL_ARGS:
			errorString += "CL_INVALID_KERNEL_ARGS";
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			errorString += "CL_INVALID_KERNEL_ARGS";
			break;
		case CL_INVALID_WORK_ITEM_SIZE:
			errorString += "CL_INVALID_WORK_ITEM_SIZE";
			break;
		case CL_OUT_OF_RESOURCES:
			errorString += "CL_OUT_OF_RESOURCES";
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			errorString += "CL_MEM_OBJECT_ALLOCATION_FAILURE";
			break;
		case CL_OUT_OF_HOST_MEMORY:
			errorString += "CL_OUT_OF_HOST_MEMORY";
			break;
		case CL_INVALID_PROGRAM_EXECUTABLE:
			errorString += "CL_INVALID_PROGRAM_EXECUTABLE";
			break;
		case CL_INVALID_COMMAND_QUEUE:
			errorString += "CL_INVALID_COMMAND_QUEUE";
			break;
		case CL_INVALID_KERNEL:
			errorString += "CL_INVALID_KERNEL";
			break;
		case CL_INVALID_WORK_DIMENSION:
			errorString += "CL_INVALID_WORK_DIMENSION";
			break;
		case CL_INVALID_GLOBAL_OFFSET:
			errorString += "CL_INVALID_GLOBAL_OFFSET";
			break;
		case CL_INVALID_EVENT_WAIT_LIST:
			errorString += "CL_INVALID_EVENT_WAIT_LIST";
			break;
		default:
			errorString += "generic error";
		}

		Log::Error(errorString);

		return false;
	}

	return true;
}

OCLKernel::~OCLKernel()
{
	clReleaseKernel(m_kernel);
}

