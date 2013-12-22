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


#include "RenderGirlShared.h"

std::vector<OCLPlatform>	RenderGirlShared::platforms;
OCLDevice*					RenderGirlShared::selectedDevice;

bool RenderGirlShared::InitPlatforms()
{
	selectedDevice = NULL;

	// do not query for new platforms if it has queried before
	assert(platforms.empty());

	// create and init OpenCL
	Log::Message("Initializing OpenCL platforms...");

	// create plataforms
	cl_uint platSize = 0;	// number of loaded platforms

	// empty query to see the size of platforms
	if (clGetPlatformIDs(0, 0, &platSize) != CL_SUCCESS)
	{
		Log::Error("OpenCL platform could not be created, please check the OpenCL drivers for your device");
		return false;
	}
	
	cl_platform_id* platforms_cl = new cl_platform_id[platSize];

	// now the real query
	if (clGetPlatformIDs(platSize, platforms_cl, &platSize) != CL_SUCCESS)
	{
		Log::Error("OpenCL platform could not be created, please check the OpenCL drivers for your device");
		return false;
	}

	Log::Message(std::to_string(platSize) + " platforms were found.");

	// put all in the platforms vector
	for (int a = 0; a < platSize; a++)
	{
		Log::Message("");
		Log::Message("Platform " + std::to_string(a + 1) + " created using the specs below: ");
		OCLPlatform platform;
		platform.Init(platforms_cl[a]);
		platforms.push_back(platform);
	}

	delete[] platforms_cl;
	
	return true;
}

bool RenderGirlShared::InitDevices(OCLDevice::DeviceType type)
{
	bool allOk = true;
	int size = platforms.size();
	for (int a = 0; a < size; a++)
	{
		allOk = platforms[a].InitDevices(type);
	}

	return allOk;
}

void RenderGirlShared::SelectDevice(OCLDevice* select)
{
	assert(select != NULL);
	if (selectedDevice != NULL)
	{
		//TODO RELEASE OLD DEVICE
	}

	selectedDevice = select;
	if (!selectedDevice->IsReady())
	{
		// prepare this device
		selectedDevice->CreateContext();
	}
}

RenderGirlShared::~RenderGirlShared()
{
}