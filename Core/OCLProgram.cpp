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

std::string OCLProgram::s_path;

OCLProgram::OCLProgram(OCLContext* context)
{
	assert(context != NULL);
	this->m_context = context;

	assert(context->IsReady() && "Context must be ready to execute kernels!");

	m_isCompiled = false;
}

OCLProgram::~OCLProgram()
{
	// delete source code
	int size = m_sourceCodeList.size();
	for (unsigned int a = 0; a < size; a++)
	{
		delete m_sourceCodeList[a];
	}
	clReleaseProgram(m_program);
}

bool OCLProgram::LoadSource(const std::string &sourceFile)
{

	assert(!m_isCompiled && "Can't load another source code if the program is already compiled");

	std::string sourcePath = s_path + sourceFile;

	// load kernel code from file
	FILE* kernelCodeFile = fopen(sourcePath.c_str(), "rb");
	if (kernelCodeFile == NULL)
	{
		Log::Error("The program couldn't find the source file " + sourcePath);
		return false;
	}

	// look for file size
	fseek(kernelCodeFile, 0L, SEEK_END);
	long size = ftell(kernelCodeFile);
	fseek(kernelCodeFile, 0L, SEEK_SET);

	//alloc enough memory
	char* kernelCode = new char[size];
	memset((char*)kernelCode, 0, size);
	fread((char*)kernelCode, sizeof(char), size, kernelCodeFile);
	// end file stuff
	fclose(kernelCodeFile);

	// add to the list as a loaded source code
	m_sourceFilePathList.push_back(sourceFile);
	m_sourceCodeList.push_back(kernelCode);
	//keep size of the file
	m_sourceSizes.push_back(size);

	return true;
}

bool OCLProgram::BuildProgram(const std::string &options)
{
	// now the building phase

	cl_int error;

	// send source code to OpenCL for each source file
	int sizeVector = m_sourceCodeList.size();
	char** sourcePointers = (char**)malloc(sizeof(char*) * sizeVector);
	size_t* lengths = (size_t*)malloc(sizeof(size_t) * sizeVector);
	std::string messageLog("Compiling: ");
	// group all pointers on the same array, set source size array and build log message
	for (unsigned int a = 0; a < sizeVector; a++)
	{
		sourcePointers[a] = m_sourceCodeList[a];
		lengths[a] = m_sourceSizes[a];
		messageLog += m_sourceFilePathList[a];
		messageLog += " ";
	}
	
	Log::Message(messageLog);

	m_program = clCreateProgramWithSource(m_context->GetCLContext(), sizeVector, (const char**)sourcePointers, lengths, &error);
	free(sourcePointers);
	free(lengths);

	if (error != CL_SUCCESS)
	{
		Log::Error("There was an error when sending the source code to the OpenCL implementation.");
		return false;
	}

	std::string options_str = options;

	if (!s_path.empty())
	{
		// We must provide the include paths for OpenCL compiler for includes
		// within .cl files (e.g. FXAA.cl)
		options_str += " -I \"" + s_path + "\"";
	}

	error = clBuildProgram(m_program, 0, NULL, options_str.c_str(), NULL, NULL);

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
			Log::Error("Error while compiling OpenCL C code.");
		}
		return false;
	}

	// ok, cool
	m_isCompiled = true;

	return true;

}

