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


#include "Log.h"


std::vector<LogListener*> Log::m_logListeners;

Log::~Log()
{
}

void Log::RemoveAllListeners()
{
	// delete all listeners
	unsigned int size = m_logListeners.size();
	for (int a = 0; a < size; a++)
	{
		LogListener* listener = m_logListeners[a];
		delete listener;
	}
	m_logListeners.clear();
}

void Log::RemoveListener(LogListener* listener)
{
	assert((std::find(m_logListeners.begin(), m_logListeners.end(), listener) != m_logListeners.end())
		&& "This listener is not registred with the log class");
	//search for the listener inside the vector
	int size = m_logListeners.size();
	for (int a = 0; a < size; a++)
	{
		if (m_logListeners[a] == listener)
		{
			delete listener;
			m_logListeners.erase(m_logListeners.begin() + a);
			break;
		}
	}
}

void Log::AddListener(LogListener* listener)
{
	m_logListeners.push_back(listener);
}

void Log::Message(const std::string &message)
{
	// send to all listeners
	int size = m_logListeners.size();
	for (int a = 0; a < size; a++)
	{
		m_logListeners[a]->PrintLog(message.c_str());
	}
}

void Log::Error(const std::string &error)
{
	// send to all listeners
	int size = m_logListeners.size();
	for (int a = 0; a < size; a++)
	{
		m_logListeners[a]->PrintError(error.c_str());
	}
}