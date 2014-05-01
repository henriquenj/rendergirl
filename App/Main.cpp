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

		MainFrame* frame = new MainFrame("RenderGirl " + _(RENDERGIRL_ARCH), wxDefaultPosition, wxSize(600, 600), wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);
		
		// create listener for log details
		LogOutputWx* listener = new LogOutputWx();
		Log::AddListener(listener);

		/* search for OpenCL capable devices on all platforms */
		RenderGirlShared::InitPlatforms();
		RenderGirlShared::InitDevices();

		frame->UpdateDevicesInterface();

		frame->Show(true);

		return true;
	}

	virtual ~RenderGirlApp()
	{
		// dealloc the OpenCL driver and all memory used in the process.
		if (RenderGirlShared::GetSelectedDevice())
		RenderGirlShared::ReleaseDevice();

		Log::RemoveAllListeners();
	}
};

IMPLEMENT_APP(RenderGirlApp)
