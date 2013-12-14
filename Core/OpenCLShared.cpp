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


#include "OpenCLShared.h"

cl_context		OpenCLShared::context;
cl_device_id	OpenCLShared::deviceId;
cl_platform_id	OpenCLShared::platformId;

void OpenCLShared::InitOpenCL(int DeviceType)
{
	// create and init OpenCL

	// error handling
	cl_uint error = CL_SUCCESS;

	// create plataforms
	error = clGetPlatformIDs(1, &platformId, NULL);

	//TODO: make debug class

}

OpenCLShared::~OpenCLShared()
{
}