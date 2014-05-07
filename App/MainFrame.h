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

#include "wx\wx.h"
#include "wx\valnum.h"

#include "OBJLoader.h"
#include "MainPanel.h"
#include "RenderFrame.h"
#include "AboutDialog.h"
#include "AppLog.h"

#include "RenderGirlCore.h"
#include "WindowsIDs.h"


/* Main frame of the application */
class MainFrame : public wxFrame
{
public:

	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style);

	virtual ~MainFrame();


	// update menus listing devices and platforms, reseting the interface
	void UpdateDevicesInterface();

	/* Checks or uncheck a given item in the "show" menu */
	void inline CheckWindowMenu(int itemid, bool check)
	{
		m_windowMenu->Check(itemid, check);
	}

private:

	RenderFrame* m_renderFrame;
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
	wxButton* m_releaseButton;
	/* menus */
	wxMenu* m_windowMenu;

	/* fields to type */
	wxTextCtrl* m_widthField;
	/*wxTextCtrl* m_heightField;*/
	wxTextCtrl* m_lightPosXField;
	wxTextCtrl* m_lightPosYField;
	wxTextCtrl* m_lightPosZField;
	wxTextCtrl* m_cameraPosXField;
	wxTextCtrl* m_cameraPosYField;
	wxTextCtrl* m_cameraPosZField;
	wxTextCtrl* m_cameraLookXField;
	wxTextCtrl* m_cameraLookYField;
	wxTextCtrl* m_cameraLookZField;

	// loaded scene
	Scene3D* scene;

	/*callback functions*/
	void OnPlatformSelect(wxCommandEvent& event);
	void OnDeviceSelect(wxCommandEvent& event);
	void OnSelectButtonPressed(wxCommandEvent& event);
	void OnLoadModel(wxCommandEvent& event);
	void OnRenderButton(wxCommandEvent& event);
	void OnReleaseButton(wxCommandEvent& event);
	void OnShowRenderFrame(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
};


#endif // __RENDERGIRLAPP_MAINFRAME__