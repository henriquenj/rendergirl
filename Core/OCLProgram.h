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
	bool LoadSource(const std::string &sourceFile);

	/* Build the loaded OpenCL program in a given context, options argument is a set 
		of options to the OpenCL compiler (can be NULL), return FALSE if there was a problem.
		For the list of avaiable options, please refer to
		http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clBuildProgram.html */
	bool BuildProgram(const char* options =  NULL);

	/* Return TRUE if this program is ready to be executed */
	inline bool IsCompiled() const
	{
		return m_isCompiled;
	}

	/* Return OpenCL program pointer */
	inline cl_program GetCLProgram()const
	{
		return m_program;
	}

	/* Get the OCLContext associated with this program */
	inline const OCLContext* GetContext()const
	{
		return m_context;
	}

private:
	// the contex on where this program is running
	const OCLContext* m_context;
	// the OpenCL program
	cl_program m_program;

	// is the program compiled and ready to be executed?
	bool m_isCompiled;
	// vector with a list of the source file list that composes this program
	std::vector<std::string> m_sourceFilePathList;
	/* vector with a list of pointers to the source code of each file,
		m_sourceCodeList[x] matches m_sourceFileList[x] */
	std::vector<char*> m_sourceCodeList;
	// the sizes of the source code files
	std::vector<int> m_sourceSizes;
};



#endif // __OCLPROGRAM_HEADER__