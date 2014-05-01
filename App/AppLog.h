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


#ifndef __APPLOGHEADER__
#define __APPLOGHEADER__

#include "wx\wx.h"
#include "Log.h"

/* AppLog.h holds the classes need to perform the connection in the log devices
	between wxWidgets and RenderGirl, extra info avaiable here http://docs.wxwidgets.org/trunk/overview_log.html */


/* Custom wxLogWindow class for redirect the logs from the core, it will also capture internal erros from wxWidgets */
class AppLog : public wxLog
{
public:
	/* a wxTextCtrol is used as output inside the main frame of the application */
	AppLog(wxTextCtrl* output, wxLog* oldLog)
	{
		this->m_output = output;
		this->m_oldLog = oldLog;
	}

	virtual ~AppLog()
	{
		// put olg log back
		wxLog::SetActiveTarget(m_oldLog);
		//...apparently, we need at least one log target at all times, otherwise wxwidgets tries to access it and crashes
	}

private:
	// print messages on the TextCtrl
	virtual void DoLogTextAtLevel(wxLogLevel level, const wxString& msg)
	{
		// change color by the nature of the message
		if (level < 2)
			m_output->SetDefaultStyle(wxTextAttr(*wxRED));
		else
			m_output->SetDefaultStyle(wxTextAttr(*wxBLACK));
		/* wxLogLevel avaiable at wxLogLevelValues enum on wx/log.h */
		m_output->AppendText(msg + "\n");
	}


	// child of main panel used for log output
	wxTextCtrl *m_output;
	// old wxLog target
	wxLog* m_oldLog;
};


// custom loglistener tasked to send the RenderGirl messages to the wxwidgets interface
class LogOutputWx : public LogListener
{
public:
	void PrintLog(const char* message)
	{
		wxLogMessage(message);
	}
	void PrintError(const char* error)
	{
		wxLogError(error);
	}
};


#endif // __APPLOGHEADER__