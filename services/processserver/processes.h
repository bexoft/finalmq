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

#include "finalmq/interfaces/fmqprocess.fmq.h"

#include <string>
#include <vector>

using namespace finalmq;
using namespace finalmq::fmqprocess;


class Processes
{
public:
	void addProcess(const ProcessConfig& config);
	void removeProcess(const std::string& id);
	ProcessState getProcessState(const std::string& id) const;
	std::vector<ProcessStatus>& getProcesses();
	const std::vector<ProcessStatus>& getProcesses() const;
	ProcessStatus* getProcess(const std::string& id);
	ProcessStatus* getProcess(int pid);
	int getProcessStatusIx(const std::string& id) const;
	int getProcessStatusIx(int pid) const;
	bool getDependentProcessNamesRecursive(const std::string& id, std::vector<std::string>& processNames);
	void getDependantProcessNamesRecursive(const std::string& id, std::vector<std::string>& processNames);
	bool getDependantProcessNames(const std::string& id, std::vector<std::string>& processNames);
	void getEntityNames(const std::string& id, std::vector<std::string>& entityNames);
	void getStartupProcessNames(std::vector<std::string>& processNames);
	void getStartedProcessNames(std::vector<std::string>& processNames);
    void getStartedDependantProcessNames(const std::string& processId, std::vector<std::string>& processNames);
    bool isStable(const std::string& id);
	bool areStable(const std::vector<std::string>& ids);
	bool isSystemOk();
	ProcessStatus* isStartTimeout();

	static bool isStable(ProcessState state);
    static bool isStarted(const ProcessStatus* process);

private:
	std::vector<ProcessStatus>		m_processes;
};


