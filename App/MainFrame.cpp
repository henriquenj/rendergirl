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
#include "wx/colordlg.h"


MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(NULL, wxID_ANY, title, pos, size, style)
{	

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
				- scene sizer
					- light sizer
			- Render Sizer
				- resolution sizer
				- camera sizer
					- cam position sizer
					- cam direction sizer
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
	wxBoxSizer* sceneSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Scene"), wxVERTICAL);
	topSizer->Add(sceneSizer, 1, wxEXPAND);

	// load model button
	m_loadModelButton = new wxButton(panel, LoadModelButton, "Load OBJ file");
	sceneSizer->Add(m_loadModelButton, 0, wxCENTER, 10);

	/* light fields interface */
	wxBoxSizer* lightSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Light"), wxVERTICAL);
	sceneSizer->Add(lightSizer, 0, wxCENTER);
	// validador allowing only floating point values
	wxFloatingPointValidator<double> valDouble;
	wxStaticText* lightPosText = new wxStaticText(panel, wxID_ANY, "Position");
	lightSizer->Add(lightPosText);
	m_lightPosXField = new wxTextCtrl(panel, wxID_ANY, "1.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	lightSizer->Add(m_lightPosXField);
	m_lightPosYField = new wxTextCtrl(panel, wxID_ANY, "1.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	lightSizer->Add(m_lightPosYField);
	m_lightPosZField = new wxTextCtrl(panel, wxID_ANY, "-10.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	lightSizer->Add(m_lightPosZField);
	// set color button
	wxButton* setColorButton = new wxButton(panel, wxID_SELECT_COLOR, "Set Color");
	lightSizer->Add(setColorButton);
	// put default color on light
	m_lightColor.Set(255,255,255);

	// render area sizer
	wxBoxSizer* renderAreaSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Render"), wxHORIZONTAL);
	m_renderButton = new wxButton(panel, RenderButton, "Render");
	m_renderButton->Disable();
	renderAreaSizer->Add(m_renderButton,0,wxCENTER,20);
	
	wxBoxSizer* resoSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Resolution"), wxVERTICAL);
	renderAreaSizer->Add(resoSizer, 0, wxALL);
	// create validador to prevent the user of typing letters
	wxIntegerValidator<int> val;
	m_widthField = new wxTextCtrl(panel, wxID_ANY, "512", wxDefaultPosition, wxDefaultSize, 0L, val);
	resoSizer->Add(m_widthField);
	m_heightField = new wxTextCtrl(panel, wxID_ANY, "512", wxDefaultPosition, wxDefaultSize, 0L, val);
	resoSizer->Add(m_heightField);

	//camera options
	wxBoxSizer* cameraSizer = new wxStaticBoxSizer(new wxStaticBox(panel, wxID_ANY, "Camera"), wxVERTICAL);
	renderAreaSizer->Add(cameraSizer, 0, wxALL);
	// camera position sizer
	wxBoxSizer* cameraPositionSizer = new wxBoxSizer(wxHORIZONTAL);
	cameraSizer->Add(cameraPositionSizer,0,wxALL);
	wxStaticText* camPositionText = new wxStaticText(panel, wxID_ANY, "Position");
	cameraPositionSizer->Add(camPositionText);

	m_cameraPosXField = new wxTextCtrl(panel, wxID_ANY, "0.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	cameraPositionSizer->Add(m_cameraPosXField,0,wxLEFT,5);
	m_cameraPosYField = new wxTextCtrl(panel, wxID_ANY, "0.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	cameraPositionSizer->Add(m_cameraPosYField);
	m_cameraPosZField = new wxTextCtrl(panel, wxID_ANY, "-10.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	cameraPositionSizer->Add(m_cameraPosZField);
	// camera direction sizer
	wxBoxSizer* cameraLookAtSizer = new wxBoxSizer(wxHORIZONTAL);
	cameraSizer->Add(cameraLookAtSizer);
	wxStaticText* camLookText = new wxStaticText(panel, wxID_ANY, "Look At");
	cameraLookAtSizer->Add(camLookText);
	// fields to type the direction
	m_cameraLookXField = new wxTextCtrl(panel, wxID_ANY, "0.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	cameraLookAtSizer->Add(m_cameraLookXField, 0, wxLEFT, 7);
	m_cameraLookYField = new wxTextCtrl(panel, wxID_ANY, "0.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	cameraLookAtSizer->Add(m_cameraLookYField);
	m_cameraLookZField = new wxTextCtrl(panel, wxID_ANY, "0.000000", wxDefaultPosition, wxDefaultSize, 0L, valDouble);
	cameraLookAtSizer->Add(m_cameraLookZField);

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
	this->Connect(wxID_SELECT_COLOR, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnSetColorButton));
	this->Connect(ShowRenderViewMenu, wxEVT_MENU, wxCommandEventHandler(MainFrame::OnShowRenderFrame));
	this->Connect(wxID_ABOUT, wxEVT_MENU, wxCommandEventHandler(MainFrame::OnAbout));

	panel->SetBestFittingSize();
	this->SetBestFittingSize();
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


	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	const std::vector<OCLPlatform*>& platforms = shared.ReturnPlatforms();

	// update wxChoice
	for (int a = 0; a < platforms.size(); a++)
	{
		m_platformChoice->Append(platforms[a]->GetName());
	}
	m_platformChoice->SetBestFittingSize();
	m_deviceChoice->SetBestFittingSize();
}

void MainFrame::OnPlatformSelect(wxCommandEvent& event)
{
	int selected = event.GetSelection();
	/* search for all devices on this platform */
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	const std::vector<OCLPlatform*>& platforms = shared.ReturnPlatforms();

	const std::vector<OCLDevice*>& devices = platforms[selected]->GetDevices();
	// update second wxChocie
	// reset it 
	m_deviceChoice->Clear();
	m_deviceChoice->Enable();
	m_selectButton->Disable();
	for (int a = 0; a < devices.size(); a++)
	{
		m_deviceChoice->Append(devices[a]->GetName());
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
	SceneManager& manager = SceneManager::GetSharedManager();

	// prepare the selected device and turn off the interface for selecting
	m_deviceChoice->Disable();
	m_platformChoice->Disable();
	m_selectButton->Disable();

	// get index from interface 
	int currentDevice = m_deviceChoice->GetCurrentSelection();
	int currentPlatform = m_platformChoice->GetCurrentSelection();

	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	const std::vector<OCLPlatform*>& platforms = shared.ReturnPlatforms();
	const std::vector<OCLDevice*>& devices = platforms[currentPlatform]->GetDevices();
	shared.SelectDevice(devices[currentDevice]);
	if (!shared.PrepareRaytracer())
	{
		// error, get out
		shared.ReleaseDevice();
		this->UpdateDevicesInterface();
		return;
	}

	// got here, no errors
	m_releaseButton->Enable();
	if (manager.GetGroupsCount() >= 1) /* we must have at least one group loaded */
	{
		this->SetStatusText("RenderGirl ready!");
		m_renderButton->Enable();
	}
	else
	{
		this->SetStatusText("No 3D scene");
	}

}

void MainFrame::OnLoadModel(wxCommandEvent& WXUNUSED(event))
{
	/* Show file dialog */

	wxFileDialog openFileDialog(this, _("Open OBJ file"), "", "","OBJ files (*.OBJ)|*.OBJ", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user has pressed cancel

	SceneManager& manager = SceneManager::GetSharedManager();
	manager.ClearScene();
	manager.LoadSceneFromOBJ(openFileDialog.GetPath().ToStdString());

	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	if (shared.GetSelectedDevice())
	{
		m_renderButton->Enable();
		this->SetStatusText("RenderGirl ready!");
	}


}

void MainFrame::OnRenderButton(wxCommandEvent& WXUNUSED(event))
{
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();

	long width = 32;
	m_widthField->GetValue().ToLong(&width);
	long height = 32;
	m_heightField->GetValue().ToLong(&height);
	/* Send data to OpenCL implementation */

	/* grab camera information from interface */
	Camera camera;

	double position = 0.0;
	m_cameraPosXField->GetValue().ToCDouble(&position);
	camera.pos.s[0] = position;
	m_cameraPosYField->GetValue().ToCDouble(&position);
	camera.pos.s[1] = position;
	m_cameraPosZField->GetValue().ToCDouble(&position);
	camera.pos.s[2] = position;

	double l_double = 0.0;
	m_cameraLookXField->GetValue().ToCDouble(&l_double);
	camera.lookAt.s[0] = l_double;
	m_cameraLookYField->GetValue().ToCDouble(&l_double);
	camera.lookAt.s[1] = l_double;
	m_cameraLookZField->GetValue().ToCDouble(&l_double);
	camera.lookAt.s[2] = l_double;

	// set up vector to the be just pointing up
	camera.up.s[0] = 0.0;
	camera.up.s[1] = 1.0;
	camera.up.s[2] = 0.0;

	/* set light*/
	Light light;
	m_lightPosXField->GetValue().ToCDouble(&l_double);
	light.pos.s[0] = l_double;
	m_lightPosYField->GetValue().ToCDouble(&l_double);
	light.pos.s[1] = l_double;
	m_lightPosZField->GetValue().ToCDouble(&l_double);
	light.pos.s[2] = l_double;

	// set color
	light.color.s[0] = m_lightColor.Red() / 255.0;
	light.color.s[1] = m_lightColor.Green() / 255.0;
	light.color.s[2] = m_lightColor.Blue() / 255.0;
	light.Ks = 0.2;
	light.Ka = 0.0;

	// render
	if (!shared.Render(width, height, camera, light))
		return;

	// get data back
	const cl_uchar4* frame = shared.GetFrame();

	m_renderFrame->SetImage(frame, wxSize(width, height));
	m_renderFrame->Show();
	m_renderFrame->Raise();
	m_windowMenu->Check(ShowRenderViewMenu, true);
}

void MainFrame::OnReleaseButton(wxCommandEvent& WXUNUSED(event))
{
	RenderGirlShared& shared = RenderGirlShared::GetRenderGirlShared();
	//release this device
	shared.ReleaseDevice();
	
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
	AboutDialog a_dialog(this, wxID_ANY, "RenderGirl", wxDefaultPosition);

	a_dialog.ShowModal();
}

void MainFrame::OnSetColorButton(wxCommandEvent& WXUNUSED(event))
{
	wxColourData data;

	// show color picker
	wxColourDialog colorPicker(this, &data);

	if (colorPicker.ShowModal() == wxID_OK)
	{
		data = colorPicker.GetColourData();
		m_lightColor = data.GetColour();
	}
}