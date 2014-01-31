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

#include "OCLProgram.h"
#include "OCLDevice.h"

OCLProgram::OCLProgram(OCLContext* context)
{
	assert(context != NULL);
	this->context = context;

	assert(context->IsReady() && "Context must be ready to execute kernels!");

	isCompiled = false;
	isLoaded = false;
}

OCLProgram::~OCLProgram()
{
	clReleaseProgram(program);
}

bool OCLProgram::LoadProgramWithSource(const std::string &sourceFile)
{
	this->sourceFile = sourceFile;
	// load kernel code from file
	FILE* kernelCodeFile = fopen(sourceFile.c_str(), "r");
	if (kernelCodeFile == NULL)
	{
		Log::Error("The program couldn't find the source file " + sourceFile);
		return false;
	}

	// look for file size
	fseek(kernelCodeFile, 0, SEEK_END);
	long size = ftell(kernelCodeFile);
	fseek(kernelCodeFile, 0, SEEK_SET);

	//alloc enough memory
	const char* kernelCode = new char[size];
	memset((char*)kernelCode, 0, size);
	fread((char*)kernelCode, sizeof(char), size, kernelCodeFile);
	// end file stuff
	fclose(kernelCodeFile);

	// now the building phase
	Log::Message("Compiling " + sourceFile);

	cl_int error;

	// send source code to OpenCL
	program = clCreateProgramWithSource(context->GetContext(), 1, &kernelCode, NULL,&error);
	if (error != CL_SUCCESS)
	{
		Log::Error("There was an error when sending the source code to the OpenCL implementation.");
		return false;
	}

	delete[] kernelCode;

	isLoaded = true;

	return true;
}

bool OCLProgram::BuildProgram(const char* options)
{
	cl_int error = clBuildProgram(program, 0, NULL, options, NULL, NULL);

	if (error != CL_SUCCESS)
	{

		if (error == CL_BUILD_PROGRAM_FAILURE)
		{
			// Determine the size of the log
			size_t log_size;
			clGetProgramBuildInfo(program, context->GetDevice()->GetID(), CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char *log = (char *)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(program, context->GetDevice()->GetID(), CL_PROGRAM_BUILD_LOG, log_size, (void*)log, NULL);

			// Print the log
			Log::Error("OpenCL compiler returned an error: " + std::string(log));
			free(log);
		}
		else
		{
			Log::Error("Error while compiling OpenCL code on file: " + sourceFile);
		}
		return false;
	}

	// ok, cool
	isCompiled = true;

	return true;

}

