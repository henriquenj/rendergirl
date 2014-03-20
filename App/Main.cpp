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

#define TEST_RESOLUTION 512

#ifndef _WIN64
#include "glut.h"
	#pragma comment (lib, "glut32.lib")
	#pragma comment (lib, "OpenGL32.lib")
bool rendered;
#endif

#include "OBJLoader.h"
#include "BMPSave.h"

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

#ifndef _WIN64

void DisplayCallback(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (rendered)
	{
		//BYTE* frame = UChar4ToBYTE(RenderGirlShared::GetFrame(), TEST_RESOLUTION, TEST_RESOLUTION);
		//glDrawPixels(TEST_RESOLUTION, TEST_RESOLUTION, GL_RGB, GL_UNSIGNED_BYTE, frame);
		glDrawPixels(TEST_RESOLUTION, TEST_RESOLUTION, GL_RGBA, GL_UNSIGNED_BYTE, RenderGirlShared::GetFrame());
		//delete[] frame;
	}

	glFlush();
	glutSwapBuffers();
}

void KeyboardCallback(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // esc key
		exit(0);
		break;
	}
}

void Menu(int option)
{
	if (option == 0)
	{
		const char * path = ShowFileDialog(0, DialogOpen, "OBJ Files (*.obj)", "*.obj");
		if (path != NULL)
		{
			Scene3D* scene = LoadOBJ(path);

			// start raytracing
			RenderGirlShared::Set3DScene(scene);
			delete scene;
			if (RenderGirlShared::Render(TEST_RESOLUTION))
			{
				// update screen
				rendered = true;
				glutPostRedisplay();
			}

		}
	}
}

void Terminate()
{
	RenderGirlShared::ReleaseDevice();
	Log::RemoveAllListeners();
}


#endif //_WIN64



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

	bool error = RenderGirlShared::PrepareRaytracer();

#ifdef _WIN64

	const char * path = ShowFileDialog(0, DialogOpen, "OBJ Files (*.obj)", "*.obj");
	if (path != NULL)
	{
		Scene3D* scene = LoadOBJ(path);

		// start raytracing
		RenderGirlShared::Set3DScene(scene);
		delete scene;
		if (RenderGirlShared::Render(TEST_RESOLUTION))
		{
			// dump image on a file
			BYTE* frame = UChar4ToBYTE(RenderGirlShared::GetFrame(), TEST_RESOLUTION, TEST_RESOLUTION);
			SaveBMP("image.bmp", TEST_RESOLUTION, TEST_RESOLUTION, frame); 
			delete[] frame;
		}

	}

#else // make glut init procedures here

	if (!error)
	{
		RenderGirlShared::ReleaseDevice();
		exit(0);
	}

	rendered = false;


	char *my_argv[] = { "rendergirl", NULL };
	int   my_argc = 1;
	glutInit(&my_argc, my_argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(TEST_RESOLUTION, TEST_RESOLUTION);
	glutCreateWindow("RenderGirl");
	glutDisplayFunc(DisplayCallback);
	glutKeyboardFunc(KeyboardCallback);
	atexit(Terminate);

	// menus
	int menu = glutCreateMenu(Menu);
	glutAddMenuEntry("Load OBJ and render", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();



#endif


	// in glut mode, never gets here
	RenderGirlShared::ReleaseDevice();
	Log::RemoveAllListeners();

	system("pause");

	return 0;
}
