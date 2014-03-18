/*
	RenderGirl - OpenCL raytracer renderer
	Copyright(C) Henrique Jung

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Log.h"


std::vector<LogListener*> Log::logListeners;

Log::~Log()
{
}

void Log::RemoveAllListeners()
{
	// delete all listeners
	for (int a = 0; a < logListeners.size(); a++)
	{
		LogListener* listener = logListeners[a];
		delete listener;
	}
	logListeners.clear();
}

void Log::RemoveListener(LogListener* listener)
{
	assert((std::find(logListeners.begin(), logListeners.end(), listener) != logListeners.end())
		&& "This listener is not registred with the log class");
	//search for the listener inside the vector
	int size = logListeners.size();
	for (int a = 0; a < size; a++)
	{
		if (logListeners[a] == listener)
		{
			delete listener;
			logListeners.erase(logListeners.begin() + a);
			break;
		}
	}
}

void Log::AddListener(LogListener* listener)
{
	logListeners.push_back(listener);
}

void Log::Message(const std::string &message)
{
	// send to all listeners
	int size = logListeners.size();
	for (int a = 0; a < size; a++)
	{
		logListeners[a]->PrintLog(message.c_str());
	}
}

void Log::Error(const std::string &error)
{
	// send to all listeners
	int size = logListeners.size();
	for (int a = 0; a < size; a++)
	{
		logListeners[a]->PrintError(error.c_str());
	}
}