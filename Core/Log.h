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

#ifndef __LOG_CLASS__
#define __LOG_CLASS__

#include <vector>
#include <assert.h>

class LogListener;

/*static log class used as a bridge between the core renderer and the log device used by the user,
	programs intersted in capturing log details from the renderer should derive from LogListener
*/
class Log
{
public:
	~Log();
	// Add a listener to the global list of listeners
	static void AddListener(LogListener* listener);
	// remove all listeners inside the listeners list, the method will dealloc the memory for the listeners
	static void RemoveAllListeners();
	// remove a given listener, the method will dealloc the memory
	static void RemoveListener(LogListener* listener);
	// send a message to all listeners
	static void Message(const std::string &message);
	// send an error message to all listeners
	static void Error(const std::string &error);
private:
	Log(){ ; };

	// global list of log listeners
	static std::vector<LogListener*> m_logListeners;
};


/*	Abstratic class for programs intersted in capturing log details. 
	Just instantiate this derived class, override the PrintLog and PrintError functions and
	add to the static Log class using AddListener */
class LogListener
{
public:
	/* override this function to receive the log messages from the static Log class
		the content of the pointer will be deleted shortly aftwards, so if you want to hold it, make a copy
	*/
	virtual void PrintLog(const char* message) = 0;
	/* override this function to receive the log errors from the static Log class
		you may want to print those messages in a different color
	the content of the pointer will be deleted shortly aftwards, so if you want to hold it, make a copy
	*/
	virtual void PrintError(const char* message) = 0;
};



#endif // __LOG_CLASS__