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

bool RenderGirlShared::SelectDevice(OCLDevice* select)
{
	assert(select != NULL);

	bool error = true;
	if (selectedDevice != NULL)
	{
		selectedDevice->ReleaseContext();
	}

	selectedDevice = select;
	if (!selectedDevice->IsReady())
	{
		// prepare this device
		error = selectedDevice->CreateContext();
		Log::Message("Selected device: " + selectedDevice->GetName());
	}

	/* TEMP SHIT*/
	//int numbers[10];
	//for (int a = 0; a < 10; a++)
	//{
	//	numbers[a] = 10 - a;
	//}
	//OCLContext* context = selectedDevice->GetContext();
	//OCLMemoryObject<int>* numberHost = context->CreateMemoryObject<int>(10);
	//OCLMemoryObject<char>* odd = context->CreateMemoryObject<char>(1);
	//OCLMemoryObject<char>* keepSorting = context->CreateMemoryObject<char>(1);
	//OCLMemoryObject<int>* size = context->CreateMemoryObject<int>(1);

	//numberHost->SetData(numbers);

	//const int size_l = 10;
	//size->SetData(&size_l);

	//const char odd_l = 0;
	//odd->SetData(&odd_l);
	//keepSorting->SetData(&odd_l);

	//context->SyncAllMemoryHostToDevice();

	//OCLProgram program(context);
	//if (!program.LoadProgramWithSource("BubbleSort.cl"))
	//	return true;
	//if (!program.BuildProgram(NULL))
	//	return true;

	//OCLKernel kernel(&program,std::string("BubbleSort"));
	//if (kernel.GetOk())
	//{
	//	// call kernel
	//	kernel.SetGlobalWorkSize(4);
	//	kernel.SetArgument(0, numberHost);
	//	kernel.SetArgument(1, size);
	//	kernel.SetArgument(2, odd);
	//	kernel.SetArgument(3, keepSorting);
	//	kernel.EnqueueExecution();
	//}

	//context->ExecuteCommands();

	//context->SyncAllMemoryDeviceToHost();

	//const int * result = numberHost->GetData();

	//context->DeleteMemoryObject(numberHost);

	/*HERE FINISH TEMP SHIT*/

	return error;
}

void RenderGirlShared::ReleaseDevice()
{
	assert(selectedDevice != NULL);
	selectedDevice->ReleaseContext();
}

RenderGirlShared::~RenderGirlShared()
{
}