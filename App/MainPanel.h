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


#ifndef __RENDERGIRLAPP_MAINPANEL__
#define __RENDERGIRLAPP_MAINPANEL__

#include "wx\wx.h"


/* Main panel that organize most part of the interface inside the main frame */
class MainPanel : public wxPanel
{
public:

	MainPanel(wxWindow *parent,
				wxWindowID winid = wxID_ANY,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL | wxNO_BORDER,
				const wxString& name = wxPanelNameStr);

	virtual ~MainPanel();
};


#endif // __RENDERGIRLAPP_MAINPANEL__