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

	this->program = program;
	this->name = name;

	cl_int error;
	kernel = clCreateKernel(program->GetCLProgram(), name.c_str(), &error);
	
	if (error != CL_SUCCESS)
	{
		Log::Error("There was an error while creating an OpenCL kernel.");
		if (error == CL_INVALID_KERNEL_NAME)
			Log::Error("There's no kernel called " + name);
		kernelOk = false;
		return;
	}

	clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), (void*)&argumentSize, NULL);

	// init variables to kernel dispatch
	workDim = 1;
	globalWorkSize = 1;

	kernelOk = true;
}

bool OCLKernel::EnqueueExecution()
{
	/* For now, I'll let OpenCL implementation decide the size of each work-group */

	cl_int error = CL_SUCCESS;

	assert(kernelOk && "Kernel must be ready to execution");

	error = clEnqueueNDRangeKernel(program->GetContext()->GetCLQueue(), kernel, workDim,
			NULL, // should always be NULL, this is from the OpenCL specification
			&globalWorkSize, // the total amount of threads (work-itens)
			NULL, // passing NULL on the size of the work-groups, OpenCL will hopefully pick the proper size
			0,NULL, NULL); // events syncronization stuff

	if (error != CL_SUCCESS)
	{
		std::string errorString = "Kernel being executed on " + program->GetContext()->GetDevice()->GetName()
			+ " report the following error: ";
		// print the error codes, I'm not taking care of all of them
		switch (error)
		{
		case CL_INVALID_KERNEL_ARGS:
			errorString += "CL_INVALID_KERNEL_ARGS";
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			errorString += "CL_INVALID_KERNEL_ARGS";
			break;
		case CL_INVALID_WORK_ITEM_SIZE:// for now will never get here
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
			break; // yeah I just copy-pasted all of them, I'm lazy
		default:
			errorString += "generic error";  // OK that was even MORE lazy
		}

		Log::Error(errorString);

		return false;
	}

	return true;
}

OCLKernel::~OCLKernel()
{
	clReleaseKernel(kernel);
}

