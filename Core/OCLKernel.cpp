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

#include "OCLKernel.h"


OCLKernel::OCLKernel(OCLProgram* program, std::string &name)
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
	}

	kernelOk = true;
}

OCLKernel::~OCLKernel()
{
	clReleaseKernel(kernel);
}

