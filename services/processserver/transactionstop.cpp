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

#include "transactionstop.h"

#include <time.h>
#include <algorithm>

#define MODULENAME	"ProcessServer"


TransactionStop::TransactionStop(const std::string& owner, const RequestContextPtr& requestContext, Processes& processes, TransactionList& transactions)
	: TransactionBase(TRANSACTIONMODE_STOP, owner, requestContext, processes, transactions, nullptr)
{
	ProcessStatus* process = m_processes.getProcess(m_owner);
	if (process)
	{
		m_processes.getDependantProcessNamesRecursive(m_owner, m_processNamesForTransaction);
	}
}

void TransactionStop::init(std::vector<ReportProcessStatus>& report)
{
	for (size_t i=0; i<m_processNamesForTransaction.size(); i++)
	{
		ProcessStatus* processDep = m_processes.getProcess(m_processNamesForTransaction[i]);
		if (processDep)
		{
			if (processDep->pid != 0)
			{
                processDep->errorReason.clear();
				processDep->state = ProcessState::STATE_STOPPING;
                processDep->timeRequest = time(nullptr);
                processDep->timeRunning = 0;
                putReport(report, *processDep, ReportId::REPORTID_STATECHANGE);
                PlatformProcess::termProcess(processDep->pid);
			}
		}
	}
}


bool TransactionStop::executeOneProcess(const std::string& name, std::vector<ReportProcessStatus>& report)
{
	bool ok = true;
	ProcessStatus* process = m_processes.getProcess(name);
	if (process)
	{
		if (process->state == ProcessState::STATE_STOPPING)
		{
			if (process->pid == 0)
			{
				process->state = ProcessState::STATE_OFF;
                process->timeDone = time(nullptr);
                putReport(report, *process, ReportId::REPORTID_STATECHANGE);
			}
			else
			{
				// just in case the system time is set into the past
				if (time(nullptr) < process->timeRequest)
				{
					process->timeRequest = time(nullptr);
					process->timeRunning = 0;
				}
				if (process->timeRunning == 0)
				{
                    if (time(nullptr) > process->timeRequest + process->config.termTimeout)
					{
                        process->timeRunning = time(nullptr);
                        putReport(report, *process, ReportId::REPORTID_KILLPROCESS);
                        PlatformProcess::killProcess(process->pid);
                    }
				}
				else
				{
					if (time(nullptr) > process->timeRunning + process->config.termTimeout)
					{
						process->state = ProcessState::STATE_OFF;
                        process->timeDone = time(nullptr);
                        putReport(report, *process, ReportId::REPORTID_STATECHANGE);
                        process->pid = 0;
                    }
				}
			}
		}
	}
	else
	{
		// process not found
		ok = false;
		streamError << "process not found " << name;
	}

	return ok;
}


bool TransactionStop::execute(std::vector<ReportProcessStatus>& report, bool& startupError)
{
	startupError = false;
	for (size_t i=0; i<m_processNamesForTransaction.size(); i++)
	{
		if (!m_processes.isStable(m_processNamesForTransaction[i]))
		{
			executeOneProcess(m_processNamesForTransaction[i], report);
		}
	}

	bool stable = m_processes.areStable(m_processNamesForTransaction);

	if (stable && m_requestContext)
	{
		ProcessState state = m_processes.getProcessState(m_owner);
		if (state == ProcessState::STATE_OFF)
		{
			m_requestContext->reply(Status::STATUS_OK);
		}
		else
		{
			m_requestContext->reply(Status::STATUS_REQUEST_PROCESSING_ERROR);
		}
		m_requestContext = nullptr;
	}
	return !stable;	// true = continue
}

//void TransactionStop::abort()
//{
//	ProcessStatus* process = m_processes.getProcess(m_owner);
//	if (process)
//	{
//		if (process->state == ProcessState::STATE_STOPPING)
//		{
//			// kill process
//			PlatformProcess::killProcess(process->pid);
//			process->state = ProcessState::STATE_OFF;
//		}
//	}
//	if (m_replyer)
//	{
//		m_replyer->errorReply(ERemoteError::CALL_E_CONFLICT);
//		m_replyer = nullptr;
//	}
//}
