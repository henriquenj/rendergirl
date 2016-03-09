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

#include <assert.h>

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

int AddSceneGroup(
    const char* name, const float* vertex,
    const int vertex_size,
	const int* faces, const int faces_size,
	const float position[3],
	const float rotation[3],
    const float scale[3])
{
	// static check to make sure sizes are multiple of 3
	assert((vertex_size % 3) == 0 && "Vertex size is not a product of 3");
	assert((faces_size % 3) == 0 && "Faces size is not a product of 3");

	assert(name != nullptr && "Received null string from Blender");

	SceneManager& manager = SceneManager::GetSharedManager();

	/* create group */
	SceneGroup* group = manager.CreateSceneGroup(std::string(name));

	/* set global transformations */
	cl_float3 float3 = {position[0], position[1], position[2]};
	group->SetPosition(float3);
	float3.s[0] = rotation[0]; float3.s[1] = rotation[1]; float3.s[2] = rotation[2];
	group->SetRotation(float3);
	float3.s[0] = scale[0]; float3.s[1] = scale[1]; float3.s[2] = scale[2];
	group->SetScale(float3);

	/* wihtin rendergirl, vertices and faces are grouped by 3, so we compute the buffers size now */
	int group_vertex_size = vertex_size / 3;
	int group_face_size = faces_size / 3;

	/* alloc memory for this object */
	cl_float3* group_vertex = new cl_float3[group_vertex_size];
	cl_int3* group_faces = new cl_int3[group_face_size];

	/* copy vertex data */
	for (int i = 0, a = 0; i < group_vertex_size; i++, a += 3)
	{
		group_vertex[i].s[0] = vertex[a];
		group_vertex[i].s[1] = vertex[a+1];
		group_vertex[i].s[2] = vertex[a+2];
	}

	group->SetVertices(group_vertex, group_vertex_size);

	/* copy faces data */
	for (int i = 0, a = 0; i < group_face_size; i++, a += 3)
	{
		group_faces[i].s[0] = faces[a];
		group_faces[i].s[1] = faces[a+1];
		group_faces[i].s[2] = faces[a+2];
	}

	group->SetFaces(group_faces, group_face_size);

	if (!group->CheckCorruptedFaces())
	{
		return -1;
	}

	delete group_vertex;
	delete group_faces;

	return 0;
}

void ClearScene()
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.ClearScene();
}

int Render(const int width, const int height,
	const float camera_pos[3], const float camera_up[3], const float camera_dir[3],
	const float light_pos[3], const float color[3],
	unsigned char* frame_out)
{

	Light light;

	light.pos.s[0] = light_pos[0];
	light.pos.s[1] = light_pos[1];
	light.pos.s[2] = light_pos[2];

	/* hardcoded spectular and ambient for now */
	light.Ks = 0.2f;
	light.Ka = 0.0f;

	light.color.s[0] = color[0];
	light.color.s[1] = color[1];
	light.color.s[2] = color[2];

	Camera cam;	
	// set up vector to the be just pointing up
	cam.up.s[0] = camera_up[0];
	cam.up.s[1] = -camera_up[1]; // TODO: find out why this is necessary
	cam.up.s[2] = camera_up[2];

	cam.pos.s[0] = camera_pos[0];
	cam.pos.s[1] = camera_pos[1];
	cam.pos.s[2] = camera_pos[2];

	cam.dir.s[0] = camera_dir[0];
	cam.dir.s[1] = camera_dir[1];
	cam.dir.s[2] = camera_dir[2];

	cam.from_lookAt = false;

	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	bool ret = shared.Render(width, height, cam, light);

	if (!ret)
	{ 
		return -1;
	}

	int frame_size = width * height;
	const cl_uchar4* frame = shared.GetFrame();
	/* copy frame to frame_out */
	for (int a = 0, b = 0; a < frame_size; a++, b+=4)
	{
		/* red */
		frame_out[b] = frame[a].s[0];
		/* green */
		frame_out[b + 1] = frame[a].s[1];
		/* blue */
		frame_out[b + 2] = frame[a].s[2];
		/* alpha */
		frame_out[b + 3] = frame[a].s[3];
	}

	return 0;
}

void FinishRenderGirl()
{
	SceneManager& manager = SceneManager::GetSharedManager();
	manager.ClearScene();

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
