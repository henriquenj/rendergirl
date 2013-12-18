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


#ifndef __OCLPLATFORM_CLASS__
#define __OCLPLATFORM_CLASS__

#include <string>

#include "CL\cl.h"
#include "Log.h"

// OCLPlatform class encapsules the OpenCL platform information
class OCLPlatform
{
public:
	// Get name of this platform
	inline const std::string& GetName()const
	{
		return name;
	}
	// get vendor name
	inline const std::string& GetVendor() const
	{
		return vendor;
	}
	inline const cl_platform_id& GetID() const
	{
		return id;
	}
	// init this platform
	void Init(cl_platform_id id);
private:

	// helper function to query information about platforms (thanks QT project for the hint)
	const std::string GetStringFromPlatform(cl_platform_info name);

	std::string name;
	std::string vendor;
	std::string extensions;
	std::string profile;
	std::string version;

	cl_platform_id id;
};





#endif // __OCLPLATFORM_CLASS__