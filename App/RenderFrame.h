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



#ifndef __RENDERGIRLAPP_RENDERFRAME__
#define __RENDERGIRLAPP_RENDERFRAME__

#include "wx\wx.h"
#include "WindowsIDs.h"
#include "CL\cl.h"

/* RenderFrame creates another window to show the final rendered image to the user */
class RenderFrame : public wxFrame
{
public:

	RenderFrame(wxWindow* parent,const wxString& title, const wxPoint& pos, const wxSize& size, long style);

	virtual ~RenderFrame();

	void OnClose(wxCloseEvent& event);
	void OnPaint(wxPaintEvent& event);

	/* Set a image to render on the frame and update the UI, this function will make a copy of the frame */
	void SetImage(const cl_uchar4 *frame, wxSize& resolution);

private:

	void OnSaveImage(wxCommandEvent& event);

	wxMenu* m_imageMenu;
	// the generated image
	wxImage m_render;
	// sizer to calculate the best fitting size
	wxBoxSizer* m_sizer;
	/* boolean control when there's a image to render */
	bool isThereImage;
};



#endif // __RENDERGIRLAPP_RENDERFRAME__