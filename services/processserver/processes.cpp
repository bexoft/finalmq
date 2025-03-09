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

#include "processes.h"

#include <algorithm>


void Processes::addProcess(const ProcessConfig& config)
{
	int ix = getProcessStatusIx(config.id);
	if (ix == -1)
	{
		m_processes.resize(m_processes.size()+1);
		m_processes.back().config = config;
	}
	else
	{
		m_processes[ix].config = config;
	}
}
void Processes::removeProcess(const std::string& id)
{
	int ix = getProcessStatusIx(id);
	if (ix != -1)
	{
		m_processes[ix].config = ProcessConfig();
		m_processes[ix].config.id = id;
	}
}
ProcessState Processes::getProcessState(const std::string& id) const
{
	int ix = getProcessStatusIx(id);
	if (ix != -1)
	{
		return m_processes[ix].state;
	}
	return ProcessState::STATE_NONE;
}
std::vector<ProcessStatus>& Processes::getProcesses()
{
	return m_processes;
}
const std::vector<ProcessStatus>& Processes::getProcesses() const
{
	return m_processes;
}
ProcessStatus* Processes::getProcess(const std::string& id)
{
	int ix = getProcessStatusIx(id);
	if (ix != -1)
	{
		return &m_processes[ix];
	}
	return nullptr;
}
ProcessStatus* Processes::getProcess(int pid)
{
	int ix = getProcessStatusIx(pid);
	if (ix != -1)
	{
		return &m_processes[ix];
	}
	return nullptr;
}
int Processes::getProcessStatusIx(const std::string& id) const
{
	for (size_t i=0; i<m_processes.size(); i++)
	{
		if (m_processes[i].config.id == id)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}
int Processes::getProcessStatusIx(int pid) const
{
	for (size_t i=0; i<m_processes.size(); i++)
	{
		if (m_processes[i].pid == pid)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}
bool Processes::getDependentProcessNamesRecursive(const std::string& id, std::vector<std::string>& processNames)
{
	bool ok = true;
	if (std::find(processNames.begin(), processNames.end(), id) == processNames.end())
	{
		int ix = getProcessStatusIx(id);
		if (ix != -1)
		{
			processNames.push_back(id);
			const ProcessConfig& config = m_processes[ix].config;
			for (size_t i=0; i<config.processDependencies.size() && ok; i++)
			{
				const std::string& idServer = config.processDependencies[i];
				ix = getProcessStatusIx(idServer);
				if (ix != -1)
				{
					ok = getDependentProcessNamesRecursive(idServer, processNames);
				}
				else
				{
					ok = false;
				}
			}
		}
		else
		{
			ok = false;
		}
	}
	return ok;
}
void Processes::getDependantProcessNamesRecursive(const std::string& id, std::vector<std::string>& processNames)
{
	if (std::find(processNames.begin(), processNames.end(), id) == processNames.end())
	{
		processNames.push_back(id);
		for (size_t i=0; i<m_processes.size(); i++)
		{
			const ProcessStatus& process = m_processes[i];
			if (std::find(process.config.processDependencies.begin(), process.config.processDependencies.end(), id) != process.config.processDependencies.end())
			{
				getDependantProcessNamesRecursive(process.config.id, processNames);
			}
		}
	}
}

bool Processes::getDependantProcessNames(const std::string& id, std::vector<std::string>& processNames)
{
	bool ok = true;
	int ix = getProcessStatusIx(id);
	if (ix != -1)
	{
		for (size_t i=0; i<m_processes.size(); i++)
		{
			const ProcessStatus& process = m_processes[i];
			if (std::find(process.config.processDependencies.begin(), process.config.processDependencies.end(), id) != process.config.processDependencies.end())
			{
				if (std::find(processNames.begin(), processNames.end(), process.config.id) == processNames.end())
				{
					processNames.push_back(process.config.id);
				}
			}
		}
	}
	else
	{
		ok = false;
	}
	return ok;
}
void Processes::getEntityNames(const std::string& id, std::vector<std::string>& entityNames)
{
	int ix = getProcessStatusIx(id);
	if (ix != -1)
	{
		const ProcessConfig& config = m_processes[ix].config;
		for (size_t i=0; i<config.entities.size(); i++)
		{
			entityNames.push_back(config.entities[i].entityName);
		}
	}
}
void Processes::getStartupProcessNames(std::vector<std::string>& processNames)
{
	for (size_t i=0; i<m_processes.size(); i++)
	{
		const ProcessStatus& process = m_processes[i];
		if (process.config.startMode == StartMode::STARTMODE_STARTUP)
		{
			processNames.push_back(process.config.id);
		}
	}
}
bool Processes::isStarted(const ProcessStatus* process)
{
    if ((process != nullptr) &&
        (process->state == ProcessState::STATE_STARTWAITING ||
         process->state == ProcessState::STATE_STARTING ||
         process->state == ProcessState::STATE_ON ||
         process->state == ProcessState::STATE_ENDED ||
         process->state == ProcessState::STATE_START_TIMEOUT ||
         process->state == ProcessState::STATE_ERROR_START ||
         process->state == ProcessState::STATE_ERROR_DEPENDENCY))
    {
        return true;
    }
    return false;
}

void Processes::getStartedDependantProcessNames(const std::string& processId, std::vector<std::string>& processNames)
{
    std::vector<std::string> processNamesDependant;
    getDependantProcessNamesRecursive(processId, processNamesDependant);
    for (size_t i=0; i<processNamesDependant.size(); i++)
    {
        const ProcessStatus* process = getProcess(processNamesDependant[i]);
        if (isStarted(process))
        {
            processNames.push_back(processNamesDependant[i]);
        }
    }
}
void Processes::getStartedProcessNames(std::vector<std::string>& processNames)
{
    for (size_t i=0; i<m_processes.size(); i++)
    {
        const ProcessStatus& process = m_processes[i];
        if (isStarted(&process))
        {
            processNames.push_back(process.config.id);
        }
    }
}


bool Processes::isStable(const std::string& id)
{
	ProcessStatus* process = getProcess(id);
	if (process)
	{
		return isStable(process->state);
	}
	return true;
}

bool Processes::areStable(const std::vector<std::string>& ids)
{
	for (size_t i=0; i<ids.size(); i++)
	{
		if (!isStable(ids[i]))
		{
			return false;
		}
	}
	return true;
}

bool Processes::isStable(ProcessState state)
{
	return (state != ProcessState::STATE_STARTWAITING &&
			state != ProcessState::STATE_STARTING &&
			state != ProcessState::STATE_STOPPING);
}


bool Processes::isSystemOk()
{
	for (size_t i=0; i<m_processes.size(); i++)
	{
		const ProcessStatus& process = m_processes[i];
		if (!(process.state == ProcessState::STATE_ON  ||
			  process.state == ProcessState::STATE_OFF ||
			  process.state == ProcessState::STATE_NONE ||
			  (process.state == ProcessState::STATE_ENDED && process.config.recoverMode == RecoverMode::RECOVERMODE_NONE)))
		{
			return false;
		}
	}
	return true;
}

ProcessStatus* Processes::isStartTimeout()
{
	for (size_t i=0; i<m_processes.size(); i++)
	{
		ProcessStatus& process = m_processes[i];
		if (process.state == ProcessState::STATE_START_TIMEOUT)
		{
			return &process;
		}
	}
	return nullptr;
}

