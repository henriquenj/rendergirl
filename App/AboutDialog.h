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


#ifndef __ABOUT_DIALOG__
#define __ABOUT_DIALOG__

#include "wx\wx.h"
#include "wx\hyperlink.h"
#include "RenderGirlCore.h"

class AboutDialog : public wxDialog
{
public:
		AboutDialog(wxWindow *parent,
                        wxWindowID id = wxID_ABOUT,
						const wxString &title = wxString("RenderGirl"),
                        const wxPoint& pos = wxDefaultPosition,
						const wxSize& size = wxDefaultSize,
                        long style = wxDEFAULT_DIALOG_STYLE);

};

#endif // __ABOUT_DIALOG__