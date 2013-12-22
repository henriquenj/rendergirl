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
#include "OCLDevice.h"

// OCLPlatform class encapsules the OpenCL platform information
class OCLPlatform
{
public:

	// init this platform
	void Init(cl_platform_id id);
	/* query for devices on this platform and init them
		DeviceType parameter can be CPU, GPU, All, Accelerator or Default
		if at least one device failed to initialized, return FALSE
	*/
	bool InitDevices(OCLDevice::DeviceType type);


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
	// get extensions list, return empty string if there's no known extensions
	inline const std::string& GetExtensions() const
	{
		return extensions;
	}
	// Get OpenCL version of this platform
	inline const std::string& GetVersion()const
	{
		return version;
	}
	// get profile type, TRUE for FULL_PROFILE and FALSE for EMBEDDED PROFILE
	inline const bool GetProfileType() const
	{
		if (profile.compare("FULL_PROFILE") == 0)
		{
			return true;
		}
		else { return false; }
	}

	// get platform ID
	inline const cl_platform_id& GetID() const
	{
		return id;
	}

	// Get devices of this platform
	inline const std::vector<OCLDevice>& GetDevices()
	{
		return devices;
	}

	// Get the amount of devices on this platform
	inline const int GetDeviceAmount()const
	{
		return devices.size();
	}
private:

	// helper function to query information about platforms (thanks QT project for this hint)
	const std::string GetStringFromPlatform(cl_platform_info name);

	std::string name;
	std::string vendor;
	std::string extensions;
	std::string profile;
	std::string version;

	cl_platform_id id;

	std::vector<OCLDevice> devices;
};





#endif // __OCLPLATFORM_CLASS__