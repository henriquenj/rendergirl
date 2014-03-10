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

#ifdef WIN32
#include "glut.h"
bool rendered;
#endif

#include "OBJLoader.h"
#include "BMPSave.h"


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

#ifndef _WIN64

void DisplayCallback(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (rendered)
	{
		glRasterPos2i(128, 128);
		//glRasterPos2i(0,  1);

		BYTE* frame = UChar4ToBYTE(RenderGirlShared::GetFrame(), 512, 512);
		glDrawPixels(512, 512, GL_RGB, GL_UNSIGNED_BYTE, frame);
		delete frame;
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
			if (RenderGirlShared::Render(512))
			{
				rendered = true;
				glutPostRedisplay();
			}

		}
	}
}

void Terminate()
{
	RenderGirlShared::ReleaseDevice();
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
	std::vector<OCLDevice> devices = platforms[1].GetDevices();
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
		if (RenderGirlShared::Render(512))
		{
			// dump image on a file
			BYTE* frame = UChar4ToBYTE(RenderGirlShared::GetFrame(), 512, 512);
			SaveBMP("image.bmp", 512, 512, frame); 
			delete frame;
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

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1024, 1024);
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

	system("pause");

	return 0;
}
