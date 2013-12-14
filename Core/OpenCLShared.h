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


#include "CL\cl.h"

#define DEVICE_GPU 1
#define DEVICE_CPU 2


// Static class encapsules all the OpenCL status and interacts with the OpenCL device
class OpenCLShared
{
public:
	/*init OpenCL on a given device
		DEVICE_GPU for GPU device
		DEVICE_CPU for CPU device
	*/
	static void InitOpenCL(int DeviceType = DEVICE_CPU);

	~OpenCLShared();

private:
	OpenCLShared(){;}

	// OpenCL stuff for internal control
	static cl_platform_id platformId;
	static cl_device_id deviceId;
	static cl_context context;
};