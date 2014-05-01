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
	wxMenu* imageMenu = new wxMenu();
	menuBar->Append(imageMenu, "Image");
	this->SetMenuBar(menuBar);

	/* set best size based on image resolution */
	this->SetBestFittingSize();

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
	if (isThereImage)
	{
		wxPaintDC dc(this);
		dc.DrawBitmap(m_render, 0, 0, false);
	}
}