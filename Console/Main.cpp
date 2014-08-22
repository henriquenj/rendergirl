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
	
	It's also useful to capture printf from the kernel on Intel platforms 
	(outputed to stdout)
*/

#include <vector>
#include <iostream>

#include "RenderGirlCore.h"
#include "OBJLoader.h"


class LogOutput : public LogListener
{
public:
	void PrintLog(const char * message)
	{
		std::cout << message << std::endl;
	}
	void PrintError(const char * error)
	{
		std::cout << error << std::endl;
	}
};

int main()
{
	// register log class
	LogOutput* listenerOutput = new LogOutput();
	Log::AddListener(listenerOutput);


	// calls for the singleton RenderGirlShared for the first time, creating it
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	SceneManager& scene_m = SceneManager::GetSharedManager();

	/* Just search for OpenCL capable devices on all platforms */
	shared.InitPlatforms();
	shared.InitDevices();

	/* Fill up camera information */
	Camera camera;
	camera.pos.s[0] = camera.lookAt.s[1] = 0.0;
	camera.pos.s[2] = -10.0;
	camera.lookAt.s[0] = camera.lookAt.s[1] = camera.lookAt.s[2] = 0.0;

	// set up vector to the be just pointing up
	camera.up.s[0] = 0.0;
	camera.up.s[1] = 1.0;
	camera.up.s[2] = 0.0;

	Light light;
	light.pos.s[0] = light.pos.s[1] = 1.0;
	light.pos.s[2] = -10.0;

	light.color.s[0] = light.color.s[1] = light.color.s[2] = 1.0;

	light.Ks = 0.2;
	light.Ka = 0.0;

	// select list of platforms
	std::vector<OCLPlatform*> platforms = shared.ReturnPlatforms();
	// get first device on first platform
	std::vector<OCLDevice*> devices = platforms[0]->GetDevices();
	shared.SelectDevice(devices[0]);
	// load kernel code and compile raytracer
	shared.PrepareRaytracer();

	std::string path;

	std::cout << "Please type the path of an OBJ file: ";
	std::cin >> path;
	

	if (!path.empty())
	{
		// using the provided OBJ loader
		if (scene_m.LoadSceneFromOBJ(path))
		{
			// call the render function
			shared.Render(256, 256, camera, light);
		}
		else
		{
			std::cout << "The program can't find the file located at " << path << std::endl;
		}
	}

	// dealloc the OpenCL driver and all memory used in the process.
	shared.ReleaseDevice();

	Log::RemoveAllListeners();

	return 0;
}