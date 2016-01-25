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
	Log::Message("Starting RenderGirlBlender");

	// calls for the singleton RenderGirlShared for the first time, creating it
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	SceneManager& scene_m = SceneManager::GetSharedManager();

	Log::Message("RenderGirlBlender started");
	Log::Error("This is an error example");

	return 0;
}



