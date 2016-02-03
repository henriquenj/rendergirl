/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2015, Henrique Jung, All rights reserved.

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

#include "RenderGirlBlender.h"
#include "RenderGirlShared.h"
#include "Log.h"
#include "OCLProgram.h"


/* 
	BlenderLogOutput class will call a C callback accesible from Python 
*/
class BlenderLogOutput : public LogListener
{
public:
	BlenderLogOutput()
	{
		this->log_callback = nullptr;
	}

	void SetCallback(c_log_callback callback)
	{
		this->log_callback = callback;
	}

private:

	void PrintLog(const char * message)
	{
		if (log_callback != nullptr)
		{
			(*log_callback)(message, false);
		}
	}

	void PrintError(const char * error)
	{
		if (log_callback != nullptr)
		{
			(*log_callback)(error, true);
		}
	}

	c_log_callback log_callback;

};

void StartLogSystem(const c_log_callback log_callback)
{
	/* Register log stuff */
	BlenderLogOutput* logOutput = new BlenderLogOutput();
	Log::AddListener(logOutput);

	logOutput->SetCallback(log_callback);
}

int StartRendergirl()
{
	// calls for the singleton RenderGirlShared for the first time, creating it
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	SceneManager& scene_m = SceneManager::GetSharedManager();

	shared.InitPlatforms();
	shared.InitDevices();

	// TODO: this is hardcoded init preserved until we have GUI on Blender side

	// select list of platforms
	std::vector<OCLPlatform*> platforms = shared.ReturnPlatforms();
	if (platforms.empty())
	{
		Log::Error("No platform found. Maybe you should install some OpenCL drivers.");
		return -1;
	}

	// get first device on first platform
	std::vector<OCLDevice*> devices = platforms[0]->GetDevices();
	if (devices.empty())
	{
		Log::Error("No device found on platform " + platforms[0]->GetName());
		return -1;
	}

	if (!shared.SelectDevice(devices[0]))
	{
		return -1;
	}

	if (!shared.PrepareRaytracer())
	{
		return -1;
	}

	return 0;
}


void FinishRenderGirl()
{
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	shared.ReleaseDevice();
}

void FinishLogSystem()
{
	Log::RemoveAllListeners();
}

void SetSourcePath(const char* path)
{
	// set to global path of RenderGirl
	OCLProgram::SetDirectoryToPath(std::string(path));
}



