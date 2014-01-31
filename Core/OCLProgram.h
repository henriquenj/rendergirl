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


#ifndef __OCLPROGRAM_HEADER__
#define __OCLPROGRAM_HEADER__

#include "OCLContext.h"

/* OCLProgram class provide methods for loading OpenCL source codes and build them.
	This class MUST be instantiate with a valid OCLContext. */
class OCLProgram
{
public:
	OCLProgram(OCLContext* context);
	~OCLProgram();

	/* Load an OpenCL program in a given context, sourceFile argument is a path to a .cl file.
		Return FALSE if there was a problem with the source code */
	bool LoadProgramWithSource(const std::string &sourceFile);

	/* Build the loaded OpenCL program in a given context, options argument is a set 
		of options to the OpenCL compiler (can be NULL), return FALSE if there was a problem.
		For the list of avaiable options, please refer to
		http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clBuildProgram.html */
	bool BuildProgram(const char* options);

	/* Return TRUE if this program is ready to be executed */
	inline bool IsCompiled() const
	{
		return isCompiled;
	}

	/* Return OpenCL program pointer */
	inline cl_program GetCLProgram()
	{
		return program;
	}

	/* Get the OCLContext associated with this program */
	inline OCLContext*  GetContext()
	{
		return context;
	}

private:
	// the contex on where this program is running
	OCLContext* context;
	// the OpenCL program
	cl_program program;

	// is the program compiled and ready to be executed?
	bool isCompiled;
	// is this program source code loaded?
	bool isLoaded;
	// the path of the sourcefile
	std::string sourceFile; 
};



#endif // __OCLPROGRAM_HEADER__