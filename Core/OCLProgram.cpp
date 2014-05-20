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

#include "OCLProgram.h"
#include "OCLDevice.h"

OCLProgram::OCLProgram(OCLContext* context)
{
	assert(context != NULL);
	this->m_context = context;

	assert(context->IsReady() && "Context must be ready to execute kernels!");

	m_isCompiled = false;
	m_isLoaded = false;
}

OCLProgram::~OCLProgram()
{
	clReleaseProgram(m_program);
}

bool OCLProgram::LoadProgramWithSource(const std::string &sourceFile)
{
	this->m_sourceFile = sourceFile;
	// load kernel code from file
	FILE* kernelCodeFile = fopen(sourceFile.c_str(), "rb");
	if (kernelCodeFile == NULL)
	{
		Log::Error("The program couldn't find the source file " + sourceFile);
		return false;
	}
	
	// look for file size
	fseek(kernelCodeFile, 0L, SEEK_END);
	long size = ftell(kernelCodeFile);
	fseek(kernelCodeFile, 0L, SEEK_SET);

	//alloc enough memory
	char* kernelCode = new char[size+1]; // +1 for the null-terminanting character
	memset((char*)kernelCode, 0, size);
	fread((char*)kernelCode, sizeof(char), size, kernelCodeFile);
	// end file stuff
	fclose(kernelCodeFile);

	kernelCode[size] = '\0'; // we need this null terminating string otherwise we the OpenCL loader goes nuts


	cl_int error;

	// send source code to OpenCL
	m_program = clCreateProgramWithSource(m_context->GetCLContext(), 1, (const char**)&kernelCode, NULL, &error);
	if (error != CL_SUCCESS)
	{
		Log::Error("There was an error when sending the source code to the OpenCL implementation.");
		return false;
	}

	delete[] kernelCode;

	m_isLoaded = true;

	return true;
}

bool OCLProgram::BuildProgram(const char* options)
{
	// now the building phase
	Log::Message("Compiling " + m_sourceFile);

	cl_int error = clBuildProgram(m_program, 0, NULL, options, NULL, NULL);

	if (error != CL_SUCCESS)
	{

		if (error == CL_BUILD_PROGRAM_FAILURE)
		{
			// Determine the size of the log
			size_t log_size;
			clGetProgramBuildInfo(m_program, m_context->GetDevice()->GetID(), CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char *log = (char *)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(m_program, m_context->GetDevice()->GetID(), CL_PROGRAM_BUILD_LOG, log_size, (void*)log, NULL);

			// Print the log
			Log::Error("OpenCL compiler returned an error: " + std::string(log));
			free(log);
		}
		else
		{
			Log::Error("Error while compiling OpenCL code on file: " + m_sourceFile);
		}
		return false;
	}

	// ok, cool
	m_isCompiled = true;

	return true;

}

