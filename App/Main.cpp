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


/* App project implements an interface with the Core using the wxWidgets toolkit */

#include <iostream>

#include "wx\wx.h"

#include "MainFrame.h"

class RenderGirlApp : public wxApp
{

public:

	virtual bool OnInit()
	{
		if (!wxApp::OnInit())
			return false;

		wxInitAllImageHandlers();

		MainFrame* frame = new MainFrame("RenderGirl " + _(RENDERGIRL_ARCH), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);
		
		// create listener for log details
		LogOutputWx* listener = new LogOutputWx();
		Log::AddListener(listener);
		
		RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
		/* search for OpenCL capable devices on all platforms */
		shared.InitPlatforms();
		shared.InitDevices();

		frame->UpdateDevicesInterface();

		frame->Show(true);

		return true;
	}

	virtual ~RenderGirlApp()
	{
		RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
		// dealloc the OpenCL driver and all memory used in the process.
		if (shared.GetSelectedDevice())
			shared.ReleaseDevice();

		Log::RemoveAllListeners();
	}
};

IMPLEMENT_APP(RenderGirlApp)
