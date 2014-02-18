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


#include "RenderGirlCore.h"

#include <Windows.h>
#include <iostream>

#include "OBJLoader.h"


/* App project will consist on a stand alone interface for the raytracer using the wxWidgets toolkit */


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
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_RED);
		std::cout << error << std::endl;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	}
};

int main()
{

	LogOutput* listenerOutput = new LogOutput();
	Log::AddListener(listenerOutput);
	
	RenderGirlShared::InitPlatforms();
	RenderGirlShared::InitDevices();

	// select a device
	std::vector<OCLPlatform> platforms = RenderGirlShared::ReturnPlatforms();
	// get the first one
	std::vector<OCLDevice> devices = platforms[0].GetDevices();
	// select this
	RenderGirlShared::SelectDevice(&devices[0]);

	RenderGirlShared::PrepareRaytracer();


	const char * path = ShowFileDialog(0, DialogOpen, "OBJ Files (*.obj)", "*.obj");
	if (path != NULL)
	{
		Scene3D* scene = LoadOBJ(path);
		// start raytracing
		RenderGirlShared::Set3DScene(scene);
		RenderGirlShared::Render(128);
	}


	RenderGirlShared::ReleaseDevice();

	system("pause");

	return 0;
}
