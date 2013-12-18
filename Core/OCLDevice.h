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


#ifndef __OCLDEVICE_CLASS__
#define __OCLDEVICE_CLASS__


#include "CL\cl.h"

// types of devices accepted by OpenCL
enum DeviceType
{
	Default = 0x00000001,
	CPU = 0x00000002,
	GPU = 0x00000004,
	Accelerator = 0x00000008,
	All = 0xFFFFFFFF
};

/* OCLDevice class encapsules the OpenCL device information*/
class OCLDevice
{
public:
	// Init device and put it ready to receive a context
	void InitDevice(DeviceType type,cl_device_id id);

private:
	DeviceType type;
	cl_device_id id;
};



#endif // __OCLDEVICE_CLASS__