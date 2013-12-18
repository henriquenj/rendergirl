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

#ifndef __RENDERGIRL_SHARED_CLASS__
#define __RENDERGIRL_SHARED_CLASS__

#include <assert.h>
#include <string>

#include "CL\cl.h"
#include "Log.h"
#include "OCLPlatform.h"


// types of devices accepted by OpenCL
enum DeviceType
{
	Default = 0x00000001,
	CPU = 0x00000002,
	GPU = 0x00000004,
	Accelerator = 0x00000008,
	All = 0xFFFFFFFF
};


// Static class encapsules the OpenCL status and the renderer status
class RenderGirlShared
{
public:
	/*init OpenCL platforms, return TRUE for success or return FALSE for failure*/
	static bool InitPlatforms();
	/* return list of avaiable platforms */ 
	static inline const std::vector<OCLPlatform>& ReturnPlatforms()
	{
		return platforms;
	}

	/* return number of avaiable platforms */
	static inline const int GetPlatformsSize()
	{
		return platforms.size();
	}
	~RenderGirlShared();

private:
	RenderGirlShared(){ ; }

	// OpenCL stuff for internal control
	static std::vector<OCLPlatform> platforms;
};



#endif // __RENDERGIRL_SHARED_CLASS__