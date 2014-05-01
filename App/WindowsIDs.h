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


#ifndef __RENDERGIRLEVENTS__
#define __RENDERGIRLEVENTS__


#include "wx\wx.h"

/* Header for all the custom window identifiers used by the wx app */

enum MainFrameEvents /* Window identifiers for MainFrame */
{
	ChoiceSelectPlatform = wxID_HIGHEST,
	ChoiceSelectDevice,
	SelectDeviceButton,
	LoadModelButton,
	ReleaseButton,
	RenderButton,
	ShowRenderViewMenu
};


enum RenderFrameEvents /* Window identifiers for render frame */
{
	StartID = RenderButton + 1 // id of render frame window starts where the main frame ids ends
};


#endif // __RENDERGIRLEVENTS__