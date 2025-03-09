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

#include "platformspecific.h"
#include "finalmq/logger/LogStream.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <Windows.h>
#include <winnt.h>
#else
#include <sys/wait.h>
#endif

#define MODULENAME	"ProcessServer"


time_t Helper::getClockSecond()
{
#ifndef WIN32
	timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return tp.tv_sec;
#else
	return ::GetTickCount64() / 1000;
#endif
}




#ifdef WIN32

int PlatformProcess::startProcess(const std::string& command, const std::vector<std::string>& args, const std::string& workingdir)
{
	std::string cmdLine = command;
	std::vector<std::string>::const_iterator it;
	for (it=args.begin(); it != args.end(); ++it)
	{
		cmdLine.append(" ");
		cmdLine.append(*it);
	}		

	STARTUPINFOA startupInfo;
	GetStartupInfoA(&startupInfo); // take defaults from current process
	startupInfo.cb          = sizeof(STARTUPINFOA);
	startupInfo.lpReserved  = nullptr;
	startupInfo.lpDesktop   = nullptr;
	startupInfo.lpTitle     = nullptr;
	startupInfo.dwFlags     = STARTF_FORCEOFFFEEDBACK;
	startupInfo.cbReserved2 = 0;
	startupInfo.lpReserved2 = nullptr;

	HANDLE hProc = GetCurrentProcess();
	
	const char* workingDirectory = workingdir.empty() ? 0 : workingdir.c_str();
	
	PROCESS_INFORMATION processInfo;
	DWORD creationFlags = 0;
	BOOL bStarted = CreateProcessA(
		nullptr, 
		const_cast<char*>(cmdLine.c_str()), 
		nullptr, // processAttributes
		nullptr, // threadAttributes
		false,
		creationFlags,
		nullptr, 
		workingDirectory, 
		&startupInfo, 
		&processInfo
	);
	
	if (bStarted)
	{
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
		return processInfo.dwProcessId;
	}
	return 0;
}

int PlatformProcess::termProcess(int pid)
{
	bool bRet = true;
	if (pid != 0)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hProcess)
		{
			if (TerminateProcess(hProcess, 0) != 0)
			{
				bRet = true;
			}
			CloseHandle(hProcess);
		}
	}
	return bRet;
}

int PlatformProcess::killProcess(int pid)
{
	bool bRet = true;
	if (pid != 0)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hProcess)
		{
			if (TerminateProcess(hProcess, 0) != 0)
			{
				bRet = true;
			}
			CloseHandle(hProcess);
		}
	}
	return bRet;
}

WatchStartedProcessesForTermination::WatchStartedProcessesForTermination()
{
	HANDLE control = ::CreateEvent(nullptr, FALSE, 0, nullptr);
	m_processes.push_back(control);
	m_pids.push_back(-1);
}
WatchStartedProcessesForTermination::~WatchStartedProcessesForTermination()
{
	terminate();
	if (m_thread.joinable())
	{
		m_thread.join();
	}
	clear();
}

void WatchStartedProcessesForTermination::addStartedProcess(int pid)
{
	std::unique_lock<std::mutex> lock(m_mtx);
	HANDLE handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	m_processes.push_back(handle);
	m_pids.push_back(pid);
	if (!m_processes.empty())
	{
		::SetEvent(m_processes[0]);
	}
	lock.unlock();
}

std::vector<int> WatchStartedProcessesForTermination::getEndedPids()
{
	std::unique_lock<std::mutex> lock(m_mtx);
	std::vector<int> ended = m_pidsEnded;
	m_pidsEnded.clear();
	lock.unlock();
	return ended;
}

void WatchStartedProcessesForTermination::clear()
{
	std::unique_lock<std::mutex> lock(m_mtx);
	for (size_t i=0; i<m_pids.size(); i++)
	{
		::CloseHandle(m_processes[i]);
	}
	m_processes.clear();
	m_pids.clear();
	m_pidsEnded.clear();
	lock.unlock();
}

void WatchStartedProcessesForTermination::removeEndedProcess(int ix)
{
	std::unique_lock<std::mutex> lock(m_mtx);
	int pid = m_pids[ix];
	m_pidsEnded.push_back(pid);
	::CloseHandle(m_processes[ix]);
	m_processes.erase(m_processes.begin() + ix);
	m_pids.erase(m_pids.begin() + ix);
	lock.unlock();
}


void WatchStartedProcessesForTermination::start()
{
	if (!m_thread.joinable())
	{
		m_thread = std::thread([this] {
			run();
		});
	}
}


int WatchStartedProcessesForTermination::run()
{
	int timWait = 0;
	while (!m_terminate.wait(timWait))
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		std::vector<HANDLE>	processesCpy = m_processes;
		lock.unlock();
		DWORD res = WaitForMultipleObjects(static_cast<DWORD>(processesCpy.size()), processesCpy.data(), FALSE, INFINITE);
		HRESULT hr = ::GetLastError();
		if (res == WAIT_OBJECT_0)
		{
			// control event
			timWait = 0;
		}
		else if (res > WAIT_OBJECT_0 && res < WAIT_OBJECT_0+processesCpy.size())
		{
			int ix = res - WAIT_OBJECT_0;
			removeEndedProcess(ix);
			timWait = 0;
		}
		else if (res >= WAIT_ABANDONED_0 && res < WAIT_ABANDONED_0+processesCpy.size())
		{
			int ix = res - WAIT_ABANDONED_0;
			removeEndedProcess(ix);
			timWait = 0;
		}
		else if (res == WAIT_TIMEOUT)
		{
			timWait = 0;
		}
		else	// failed
		{
			timWait = 100;
		}
	}
	return 0;
}
void WatchStartedProcessesForTermination::terminate()
{
	std::unique_lock<std::mutex> lock(m_mtx);
	if (!m_processes.empty())
	{
		::SetEvent(m_processes[0]);
	}
	lock.unlock();
	m_terminate = true;
}


#else

int PlatformProcess::startProcess(const std::string& command, const std::vector<std::string>& args, const std::string& workingdir)
{
	int id = fork();
	streamInfo << "fork " << command;
	if (id == 0)
	{
		const char** argv = new const char*[args.size()+2];
		argv[0] = command.c_str();
		for (size_t i=0; i<args.size(); i++)
		{
			argv[i+1] = args[i].c_str();
		}
		argv[args.size()+1] = nullptr;
		if (!workingdir.empty())
		{
			int res = chdir(workingdir.c_str());
			if (res != 0)
			{
				streamError << "chdir(" << workingdir << ") error: " << res;
			}
		}
		streamInfo << "execv " << argv[0];
		execv(argv[0], (char**)argv);
		streamError << "execv failed for " << argv[0] << ", errno:" << errno; fflush(stdout);
		// should not be executed on success.
		_exit(-1);
	}
	return (id != -1) ? id : 0;
}
int PlatformProcess::termProcess(int pid)
{
	streamInfo << "term pid:" << pid;
	if (pid != 0)
	{
		kill(pid, SIGTERM);
	}
	return 0;
}
int PlatformProcess::killProcess(int pid)
{
	streamInfo << "kill pid:" << pid;
	if (pid != 0)
	{
		kill(pid, SIGKILL);
	}
	return 0;
}

WatchStartedProcessesForTermination::WatchStartedProcessesForTermination()
{
}
WatchStartedProcessesForTermination::~WatchStartedProcessesForTermination()
{
	m_terminate = true;
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void WatchStartedProcessesForTermination::addStartedProcess(int pid)
{
}

std::vector<int> WatchStartedProcessesForTermination::getEndedPids()
{
	std::unique_lock<std::mutex> lock(m_mtx);
	std::vector<int> ended = m_pidsEnded;
	m_pidsEnded.clear();
	lock.unlock();
	return ended;
}

void WatchStartedProcessesForTermination::start()
{
	if (!m_thread.joinable())
	{
		m_thread = std::thread([this] {
			run();
		});
	}
}

int WatchStartedProcessesForTermination::run()
{
	int timWait = 0;
	while (!m_terminate.wait(timWait))
	{
		int status = 0;
		printf("=== wait...\n");
		pid_t pid = wait(&status);
		printf("=== wait pid:%d, status:%d\n", pid, status);
		if (pid != -1)
		{
			if (WIFEXITED(status) || WEXITSTATUS(status))
			{
				printf("=== pid:%d TERMINATED WEXITSTATUS(status):%d\n", pid, WEXITSTATUS(status));
				timWait = 0;
				std::unique_lock<std::mutex> lock(m_mtx);
				m_pidsEnded.push_back(pid);
				lock.unlock();
			}
			else if (WTERMSIG(status))
			{
				printf("=== pid:%d TERMINATED WTERMSIG(status):%d\n", pid, WTERMSIG(status));
				timWait = 0;
				std::unique_lock<std::mutex> lock(m_mtx);
				m_pidsEnded.push_back(pid);
				lock.unlock();
			}
			else
			{
				printf("=== pid:%d NOT TERMINATED\n", pid);
				timWait = 100;
			}
			fflush(stdout);
		}
		else
		{
			timWait = 500;
		}
	}
	return 0;
}

void WatchStartedProcessesForTermination::terminate()
{
	m_terminate = true;
}

#endif
