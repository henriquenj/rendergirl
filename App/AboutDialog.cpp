/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2014, Henrique Jung, All rights reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library.
*/

#include "AboutDialog.h"


AboutDialog::AboutDialog(wxWindow *parent, wxWindowID id,
	const wxString &title, const wxPoint &position,
	const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	// put ok button
	wxButton* okButton = new wxButton(this, wxID_OK, _("Ok"));
	
	// add hyperlink to the project
	wxHyperlinkCtrl* projectLink = new wxHyperlinkCtrl(this, wxID_ANY, _("Project hosted at GitHub"), "https://github.com/henriquenj/rendergirl");

	// build string to be displayed on about box
	std::string aboutText = "\n"
		"Core version: " RENDERGIRLCORE_VERSION "\n"
		"Compiled at: " RENDERGIRLCORE_COMPILED_DATE "\n"
		"\n"
		"OpenCL raytracer";

	// put on a static text
	wxStaticText* const Message = new wxStaticText(this, wxID_ANY,
		wxString::FromAscii(aboutText.c_str()));

	Message->Wrap(GetSize().GetWidth());

	wxStaticText* programTitle = new wxStaticText(this, wxID_ANY, "RenderGirl", wxDefaultPosition, wxDefaultSize,
													wxALIGN_CENTRE_HORIZONTAL);
	programTitle->Wrap(this->GetSize().GetWidth());

	// change font to bold
	wxFont font = programTitle->GetFont();
	font.SetPointSize(14);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	programTitle->SetFont(font);


	sizer->Add(programTitle, 0, wxCENTER,10);
	sizer->Add(Message,0,wxCENTER,10);
	sizer->Add(projectLink,0,wxCENTER,10);
	sizer->Add(okButton,0,wxCENTER | wxALL,10);

	sizer->SetMinSize(wxSize(300,100));
	SetSizer(sizer);
	SetBestFittingSize();

	SetFocus();
	Center();
}



