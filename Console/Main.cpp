/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2014, Henrique Jung, All rights reserved.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this program.
*/



/* 
	RenderGirlConsole is an interface for RenderGirl that does not contain any GUI elements
	and it's suppose to be clean and simple. There's no image output.
	
	Windows only for now.
	
	It's also useful to capture printf from the kernel on Intel platforms 
	(outputed to stdout)
*/

#include <vector>
#include <iostream>
#include <Windows.h>

#include "RenderGirlCore.h"
#include "OBJLoader.h"
#include "Utilities.h"


class LogOutput : public LogListener
{
public:
	void PrintLog(const char * message)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		std::cout << message << std::endl;
	}
	void PrintError(const char * error)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED);
		std::cout << error << std::endl;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	}
};

int main()
{
	// register log class
	LogOutput* listenerOutput = new LogOutput();
	Log::AddListener(listenerOutput);


	// calls for the singleton RenderGirlShared for the first time, creating it
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();

	/* Just search for OpenCL capable devices on all platforms */
	shared.InitPlatforms();
	shared.InitDevices();

	Camera camera;
	Light light;

	// select list of platforms
	std::vector<OCLPlatform*> platforms = shared.ReturnPlatforms();
	// get first device on first platform
	std::vector<OCLDevice*> devices = platforms[0]->GetDevices();
	shared.SelectDevice(devices[0]);
	// load kernel code and compile raytracer
	shared.PrepareRaytracer();

	const char* path = ShowFileDialog(0, DialogOpen, "OBJ Files (*.obj)", "*.obj");

	if (path != NULL)
	{
		Scene3D* scene = LoadOBJ(path); // using the provided OBJ loader
		// send 3D information to the renderer
		shared.Set3DScene(scene);
		// call the render function
		shared.Render(256, 256, camera, light);
		delete scene;
	}

	// dealloc the OpenCL driver and all memory used in the process.
	shared.ReleaseDevice();

	Log::RemoveAllListeners();

	return 0;
}