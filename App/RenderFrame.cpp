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



#include "RenderFrame.h"
#include "MainFrame.h"


RenderFrame::RenderFrame(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(parent, wxID_ANY, title, pos, size, style)
{
	isThereImage = false;

	this->Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, wxCloseEventHandler(RenderFrame::OnClose));
	this->Connect(wxID_ANY, wxEVT_PAINT, wxPaintEventHandler(RenderFrame::OnPaint));

	
	m_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_sizer->SetMinSize(wxSize(512,512));
	this->SetSizer(m_sizer);

	/* menus on top part */
	wxMenuBar* menuBar = new wxMenuBar();
	m_imageMenu = new wxMenu();
	m_imageMenu->Append(wxID_SAVE, "Save image", "Save image on hard drive", false);
	m_imageMenu->Enable(wxID_SAVE, false);
	
	menuBar->Append(m_imageMenu, "Image");
	this->SetMenuBar(menuBar);

	/* set best size based on image resolution */
	this->SetBestFittingSize();


	this->Connect(wxID_SAVE, wxEVT_MENU, wxCommandEventHandler(RenderFrame::OnSaveImage));

}

RenderFrame::~RenderFrame()
{
	
}

void RenderFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
	/* unchecks option in the main frame */
	MainFrame* mainFrame = (MainFrame*)this->GetParent();
	mainFrame->CheckWindowMenu(ShowRenderViewMenu, false);
	
	this->Hide();
}

void RenderFrame::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	/* draw black background of the size of render view */
	wxImage blackBackground(m_sizer->GetSize());
	dc.DrawBitmap(blackBackground, 0, 0);

	if (m_render.IsOk())
		dc.DrawBitmap(m_render, 0, 0);
}


void RenderFrame::SetImage(const cl_uchar4 *frame, wxSize& resolution)
{
	if (m_render.IsOk())
		m_render.Destroy(); /* delete previously image */


	m_imageMenu->Enable(wxID_SAVE, true);

	/* copy to local data */
	long size = resolution.x * resolution.y;
	unsigned char* frameRGBData = (unsigned char*)malloc(size * 3);
	unsigned char* frameAlphaData = (unsigned char*)malloc(size);
	
	for (unsigned int a = 0; a < size; a++)
	{
		frameRGBData[a*3] = frame[a].s[0];
		frameRGBData[a*3 +1] = frame[a].s[1];
		frameRGBData[a*3 + 2] = frame[a].s[2];
		frameAlphaData[a] = frame[a].s[3];
	}

	m_render.Create(resolution, frameRGBData, frameAlphaData, true);

	/* resize screen to best fit the image */
	m_sizer->SetMinSize(resolution);
	this->SetBestFittingSize();
	this->Refresh();
	
}

void RenderFrame::OnSaveImage(wxCommandEvent& WXUNUSED(event))
{
	/* ask for the user to select a file name and a file format to save */
	wxFileDialog saveFileDialog(this, _("Save image file"), "", "render.png", "PNG(*.png)|*.png;|JPEG(*.jpg)|*.jpg;|BMP(*.bmp)|*.bmp;|TGA(*.tga)|*.tga", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user has pressed cancel

	//..got here, save image
	m_render.SaveFile(saveFileDialog.GetPath());
}