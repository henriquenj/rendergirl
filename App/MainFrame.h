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


#ifndef __RENDERGIRLAPP_MAINFRAME__
#define __RENDERGIRLAPP_MAINFRAME__


#include "OBJLoader.h"
#include "wx\wx.h"
#include "MainPanel.h"
#include "AppLog.h"
#include "wx\valnum.h"

#include "RenderGirlCore.h"

enum MainFrameEvents /* Event processing for this frame */
{
	ZeroEvent = wxID_HIGHEST,
	ChoiceSelectPlatform,
	ChoiceSelectDevice,
	SelectDevicePress,
	LoadModelPress,
	RenderPress
};


/* Main frame of the application */
class MainFrame : public wxFrame
{
public:

	MainFrame(const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);

	void OnQuit(wxCommandEvent& event);

	virtual ~MainFrame();


	// update menus listing devices and platforms, reseting the interface
	void UpdateDevicesInterface();

private:
	// log target for this application
	AppLog *m_logTarget;

	/* wxChoice with the list for platforms and devices */
	wxChoice* m_platformChoice;
	wxChoice* m_deviceChoice;
	/* buttons */
	wxButton* m_selectButton;
	wxButton* m_changeDeviceButton;
	wxButton* m_loadModelButton;
	wxButton* m_renderButton;

	wxTextCtrl* resolutionField;

	// loaded scene
	Scene3D* scene;

	/*callback functions*/
	void OnPlatformSelect(wxCommandEvent& event);
	void OnDeviceSelect(wxCommandEvent& event);
	void OnSelectButtonPressed(wxCommandEvent& event);
	void OnLoadModel(wxCommandEvent& event);
	void OnRenderButton(wxCommandEvent& event);
};


#endif // __RENDERGIRLAPP_MAINFRAME__