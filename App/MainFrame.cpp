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


#include "MainFrame.h"


MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(NULL, wxID_ANY, title, pos, size, style)
{	

	scene = NULL;

	/* create top menu bar*/
	wxMenuBar* topBar = new wxMenuBar();
	this->SetMenuBar(topBar);
	/* window menu */
	m_windowMenu = new wxMenu;
	m_windowMenu->Append(ShowRenderViewMenu, "Render View", "Show render view frame", true);
	m_windowMenu->Check(ShowRenderViewMenu, false);
	topBar->Append(m_windowMenu, "Show");

	wxMenu* helpMenu = new wxMenu;
	helpMenu->Append(wxID_ABOUT, "About", "About RenderGirl", false);
	topBar->Append(helpMenu, "Help");

	/* put render frame at the right of the main frame*/
	wxPoint posRender(this->GetPosition().x + this->GetSize().x,this->GetPosition().y);
	/* create render window */
	m_renderFrame = new RenderFrame(this, "Render View", posRender, wxDefaultSize, wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);

	/* Create botton bar */
	this->CreateStatusBar();
	this->SetStatusText("RenderGirl not ready, select a device to perform the rendering");

	MainPanel* panel = new MainPanel(this);

	/* hierarchy of the sizers inside the main frame
		- Main sizer
			- Top sizer
				- device sizer
				- load file sizer
			- Render Sizer
				- resolution sizer
			- Log sizer
	*/

	// sizer that holds the others sizers
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);


	/* selection of devices and files */
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* deviceSizer = new wxStaticBoxSizer(new wxStaticBox(panel,wxID_ANY,"Device selection"),wxVERTICAL);
	topSizer->Add(deviceSizer, 1, wxEXPAND);

	// choice of platforms
	wxStaticText* platformsChoiceText = new wxStaticText(panel, wxID_ANY, "Platforms");
	wxStaticText* devicesChoiceText = new wxStaticText(panel, wxID_ANY, "Devices");

	m_platformChoice = new wxChoice(panel, ChoiceSelectPlatform);
	m_deviceChoice = new wxChoice(panel, ChoiceSelectDevice);
	m_deviceChoice->Disable();
	m_selectButton = new wxButton(panel, SelectDeviceButton, "Select");
	m_selectButton->Disable();// disabled until the user picks a device
	m_releaseButton = new wxButton(panel, ReleaseButton, "Release");
	m_releaseButton->Disable();

	deviceSizer->Add(platformsChoiceText);
	deviceSizer->Add(m_platformChoice);
	deviceSizer->Add(devicesChoiceText);
	deviceSizer->Add(m_deviceChoice);
	deviceSizer->Add(m_selectButton,0,wxALL,10);
	deviceSizer->Add(m_releaseButton, 0, wxALL, 10);


	// sizer for loading model button
	wxBoxSizer* loadFileSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Model"), wxVERTICAL);
	topSizer->Add(loadFileSizer, 1, wxEXPAND);

	// load model button
	m_loadModelButton = new wxButton(panel, LoadModelButton, "Load OBJ file", wxDefaultPosition, wxSize(100, 30));
	loadFileSizer->Add(m_loadModelButton,0,wxCENTER,10);

	// render area sizer
	wxBoxSizer* renderAreaSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Render"), wxHORIZONTAL);
	m_renderButton = new wxButton(panel, RenderButton, "Render", wxDefaultPosition, wxSize(300, 100));
	m_renderButton->Disable();
	renderAreaSizer->Add(m_renderButton,0,wxCENTER);
	
	wxBoxSizer* resoSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Resolution"), wxVERTICAL);
	renderAreaSizer->Add(resoSizer, 0, wxALL,30);
	// create validador to prevent the user of typing letters
	wxIntegerValidator<int> val;
	resolutionField = new wxTextCtrl(panel, wxID_ANY, "512", wxDefaultPosition, wxSize(50,30), 0L,val);
	resoSizer->Add(resolutionField);

	// sizer for log
	wxSizer *sizerLog = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, wxT("&Log window")),wxVERTICAL);
	/* Create lot output */
	wxTextCtrl* logOutput = new wxTextCtrl(panel, wxID_ANY, wxEmptyString,wxDefaultPosition,wxDefaultSize,
		wxTE_MULTILINE | wxHSCROLL | wxTE_RICH | wxTE_READONLY);
	sizerLog->Add(logOutput, 1, wxEXPAND);

	// create log target for this application
	m_logTarget = new AppLog(logOutput,wxLog::GetActiveTarget());
	wxLog::SetActiveTarget(m_logTarget);

	mainSizer->Add(topSizer, 1, wxEXPAND);
	mainSizer->Add(renderAreaSizer, 1, wxEXPAND);
	mainSizer->Add(sizerLog, 1, wxEXPAND);
	panel->SetSizer(mainSizer);

	// connect events
	this->Connect(ChoiceSelectPlatform, wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(MainFrame::OnPlatformSelect));
	this->Connect(ChoiceSelectDevice, wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(MainFrame::OnDeviceSelect));
	this->Connect(SelectDeviceButton, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnSelectButtonPressed));
	this->Connect(LoadModelButton, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnLoadModel));
	this->Connect(RenderButton, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnRenderButton));
	this->Connect(ReleaseButton, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnReleaseButton));
	this->Connect(ShowRenderViewMenu, wxEVT_MENU, wxCommandEventHandler(MainFrame::OnShowRenderFrame));
	this->Connect(wxID_ABOUT, wxEVT_MENU, wxCommandEventHandler(MainFrame::OnAbout));

	panel->SetBestFittingSize();
}

MainFrame::~MainFrame()
{
	delete m_logTarget;
	wxLog::DontCreateOnDemand();
}

void MainFrame::UpdateDevicesInterface()
{

	m_selectButton->Disable();
	m_releaseButton->Disable();
	m_renderButton->Disable();
	m_platformChoice->Clear();
	m_deviceChoice->Clear();
	m_platformChoice->Enable();

	const std::vector<OCLPlatform>* platforms = RenderGirlShared::ReturnPlatforms();

	// update wxChoice
	for (int a = 0; a < platforms->size(); a++)
	{
		m_platformChoice->Append((*platforms)[a].GetName());
	}

	m_deviceChoice->SetBestFittingSize();
}

void MainFrame::OnPlatformSelect(wxCommandEvent& event)
{
	int selected = event.GetSelection();
	/* search for all devices on this platform */
	const std::vector<OCLPlatform>* platforms = RenderGirlShared::ReturnPlatforms();

	const std::vector<OCLDevice>* devices = (*platforms)[selected].GetDevices();
	// update second wxChocie
	// reset it 
	m_deviceChoice->Clear();
	m_deviceChoice->Enable();
	m_selectButton->Disable();
	for (int a = 0; a < devices->size(); a++)
	{
		m_deviceChoice->Append((*devices)[a].GetName());
	}
	m_deviceChoice->SetBestFittingSize();
}

void MainFrame::OnDeviceSelect(wxCommandEvent& event)
{
	//enable select button
	m_selectButton->Enable();
}

void MainFrame::OnSelectButtonPressed(wxCommandEvent& WXUNUSED(event))
{
	// prepare the selected device and turn off the interface for selecting
	m_deviceChoice->Disable();
	m_platformChoice->Disable();
	m_selectButton->Disable();

	// get index from interface 
	int currentDevice = m_deviceChoice->GetCurrentSelection();
	int currentPlatform = m_platformChoice->GetCurrentSelection();

	const std::vector<OCLPlatform>* platforms = RenderGirlShared::ReturnPlatforms();
	const std::vector<OCLDevice>* devices = (*platforms)[currentPlatform].GetDevices();
	RenderGirlShared::SelectDevice(&(*devices)[currentDevice]);
	if (!RenderGirlShared::PrepareRaytracer())
	{
		// error, get out
		RenderGirlShared::ReleaseDevice();
		this->UpdateDevicesInterface();
		return;
	}

	// got here, no errors
	m_releaseButton->Enable();
	if (scene != NULL)
	{
		this->SetStatusText("RenderGirl ready!");
		m_renderButton->Enable();
	}

}

void MainFrame::OnLoadModel(wxCommandEvent& WXUNUSED(event))
{
	/* Show file dialog */

	wxFileDialog openFileDialog(this, _("Open OBJ file"), "", "","OBJ files (*.OBJ)|*.OBJ", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user has pressed cancel

	if (scene != NULL)
		delete scene;
	
	scene = LoadOBJ(openFileDialog.GetPath());

	if (RenderGirlShared::GetSelectedDevice())
		m_renderButton->Enable();


}

void MainFrame::OnRenderButton(wxCommandEvent& WXUNUSED(event))
{
	long resolution = 32;
	resolutionField->GetValue().ToLong(&resolution);
	/* Send data to OpenCL implementation */

	if (!RenderGirlShared::Set3DScene(scene))
		return;

	// render
	if (!RenderGirlShared::Render(resolution))
		return;

	// get data back
	const cl_uchar4* frame = RenderGirlShared::GetFrame();

	m_renderFrame->SetImage(frame, wxSize(resolution, resolution));
	m_renderFrame->Show();
	m_renderFrame->Raise();
	m_windowMenu->Check(ShowRenderViewMenu, true);
}

void MainFrame::OnReleaseButton(wxCommandEvent& WXUNUSED(event))
{
	//release this device
	RenderGirlShared::ReleaseDevice();
	
	this->SetStatusText("RenderGirl not ready, select a device to perform the rendering");
	this->UpdateDevicesInterface();
}

void MainFrame::OnShowRenderFrame(wxCommandEvent& WXUNUSED(event))
{
	/* Show or hide the render view frame bases on the current state*/
	if (!m_renderFrame->IsShown())
		m_renderFrame->Show();
	else
		m_renderFrame->Hide();
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	// create about dialog
	AboutDialog a_dialog(this, wxID_ANY, "RenderGirl", wxDefaultPosition, wxSize(340, 200));

	a_dialog.ShowModal();
}