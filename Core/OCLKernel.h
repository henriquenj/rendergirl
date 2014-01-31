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

#ifndef __OCLKERNEL_HEADER__
#define __OCLKERNEL_HEADER__

#include "OCLProgram.h"

/* OCLKernel class dispatches execution for OCLProgram objects, using data avaiable on a given OCLContext,
	must be instantiated using a valid OCLProgram*/
class OCLKernel
{
public:
	OCLKernel(OCLProgram* program,std::string &name);
	~OCLKernel();

	// return FALSE is the kernel was not ok (probrably there's no such kernel in this progrm)
	inline bool GetOk()
	{
		return kernelOk;
	}

private:
	// the program that this kernel will execute
	OCLProgram* program;
	// OpenCL kernel pointer
	cl_kernel kernel;
	// kernel name
	std::string name;
	// is this kernel Ok?
	bool kernelOk;
};





#endif // __OCLKERNEL_HEADER__