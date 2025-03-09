//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include "defines.h"
#include "finalmq/helpers/CondVar.h"

#include <string>
#include <vector>
#include <thread>
#include <mutex>

#ifdef WIN32
typedef void* HANDLE;
#endif


class Helper
{
public:
	static time_t getClockSecond();
};

class PlatformProcess
{
public:
	static int startProcess(const std::string& command, const std::vector<std::string>& args, const std::string& workingdir);
	static int termProcess(int pid);
	static int killProcess(int pid);
};


class WatchStartedProcessesForTermination
{
public:
	WatchStartedProcessesForTermination();
	~WatchStartedProcessesForTermination();
	void addStartedProcess(int pid);
	std::vector<int> getEndedPids();
	void start();

private:
#ifdef WIN32
	void clear();
	void removeEndedProcess(int ix);
#endif
	int run();
	void terminate();

#ifdef WIN32
	std::vector<HANDLE>	m_processes;
	std::vector<int>	m_pids;
#endif
	std::vector<int>	m_pidsEnded;
	std::thread         m_thread;
	std::mutex			m_mtx;
	finalmq::CondVar	m_terminate;
};


