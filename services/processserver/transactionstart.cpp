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

#include "transactionstart.h"

#include "finalmq/helpers/File.h"

#include <time.h>
#include <algorithm>

#define MODULENAME	"ProcessServer"


TransactionStart::TransactionStart(const std::string& owner, const RequestContextPtr& requestContext, Processes& processes, TransactionList& transactions, WatchStartedProcessesForTermination& watchStartedProcesses, bool startup)
	: TransactionBase(TRANSACTIONMODE_START, owner, requestContext, processes, transactions, &watchStartedProcesses)
	, m_initOk(false)
	, m_peerCache()
	, m_entitiesTotal(0)
	, m_entitiesStarted(0)
{
	ProcessStatus* process = m_processes.getProcess(m_owner);
	if (process)
	{
		process->startup = startup;
		m_initOk = m_processes.getDependentProcessNamesRecursive(m_owner, m_processNamesForTransaction);
	}
}

void TransactionStart::init(std::vector<ReportProcessStatus>& report)
{
	m_peerCache.start(100, -1);
	ProcessStatus* process = m_processes.getProcess(m_owner);
	if (process)
	{
		if (m_initOk)
		{
			bool ok = true;
			for (size_t i=0; i<m_processNamesForTransaction.size() && ok; i++)
			{
				ProcessStatus* processDep = m_processes.getProcess(m_processNamesForTransaction[i]);
				if (processDep)
				{
					if (processDep->pid == 0)
					{
                        processDep->timeRequest = time(nullptr);
                        if (processDep->state != ProcessState::STATE_STARTWAITING)
						{
                            processDep->errorReason.clear();
							processDep->state = ProcessState::STATE_STARTWAITING;
                            putReport(report, *processDep, ReportId::REPORTID_STATECHANGE);
						}
                    }
				}
				else
				{
					ok = false;
				}
			}
			if (!ok)
			{
				// set all dependent startwaiting processes to off
				for (size_t i=0; i<m_processNamesForTransaction.size() && ok; i++)
				{
					ProcessStatus* processDep = m_processes.getProcess(m_processNamesForTransaction[i]);
					if (processDep && processDep->state == ProcessState::STATE_STARTWAITING)
					{
						processDep->state = ProcessState::STATE_OFF;
						putReport(report, *processDep, ReportId::REPORTID_STATECHANGE);
                    }
				}
				process->state = ProcessState::STATE_ERROR_DEPENDENCY;
                putReport(report, *process, ReportId::REPORTID_STATECHANGE);
            }
		}
		else
		{
			process->state = ProcessState::STATE_ERROR_DEPENDENCY;
			putReport(report, *process, ReportId::REPORTID_STATECHANGE);
        }
	}
    else
    {
        // process not found
        streamError << "process not found " << m_owner;
    }
}



bool TransactionStart::executeOneProcess(const std::string& name, std::vector<ReportProcessStatus>& protocol)
{
	bool ok = true;
	m_peerCache.getExecutor()->runAvailableActions();
	ProcessStatus* process = m_processes.getProcess(name);
	if (process)
	{
		if (process->state == ProcessState::STATE_STARTWAITING || 
			process->state == ProcessState::STATE_STARTING)
		{
			// just in case the system time is set into the past
			if (time(nullptr) < process->timeRequest)
			{
				process->timeRequest = time(nullptr);
			}
			if (time(nullptr) <= process->timeRequest + process->config.startTimeout)
			{
				if (process->state == ProcessState::STATE_STARTWAITING)
				{
					bool processDependenciesOk = true;
					// look for file dependencies
					const std::vector<std::string>& depsFile = process->config.fileSyncAtStart;
					for (size_t i=0; i<depsFile.size() && processDependenciesOk; i++)
					{
						if (!File::doesFileExist(depsFile[i].c_str()))
						{
							processDependenciesOk = false;
						}
					}
					//// look for process dependencies
					//const std::vector<std::string>& depsProc = process->config.processSyncAtStart;
					//for (size_t i=0; i<depsProc.size() && ok && processDependenciesOk; i++)
					//{
					//	if (depsProc[i] != name)
					//	{
					//		ProcessStatus* processDep = m_processes.getProcess(depsProc[i]);
					//		if (processDep != nullptr)
					//		{
					//			if (processDep->state == ProcessState::STATE_STARTWAITING ||
					//				processDep->state == ProcessState::STATE_STARTING ||
					//				processDep->state == ProcessState::STATE_ON)
					//			{
					//				if (processDep->state != ProcessState::STATE_ON)
					//				{
					//					processDependenciesOk = false;
					//				}
					//			}
					//			else
					//			{
					//				ok = false;
					//				streamError << "dependent process not starting/started " << processDep->config.id << processDep->state.toString();
					//				process->state = ProcessState::STATE_ERROR_DEPENDENCY;
     //                               putReport(protocol, *process, ReportId::REPORTID_STATECHANGE);
     //                           }
					//		}
					//		else
					//		{
					//			ok = false;
					//			streamError << "process not found " << process->config.id;
					//			process->state = ProcessState::STATE_ERROR_DEPENDENCY;
     //                           putReport(protocol, *process, ReportId::REPORTID_STATECHANGE);
     //                       }
					//	}
					//}
					if (ok && processDependenciesOk)
					{
						process->state = ProcessState::STATE_STARTING;
                        // start process
						int pid = PlatformProcess::startProcess(process->config.command, process->config.args, process->config.workingdir);
						if (pid != 0)
						{
							process->pid = pid;
                            process->timeRunning = time(nullptr);
                            putReport(protocol, *process, ReportId::REPORTID_STATECHANGE);
                            if (m_watchStartedProcesses)
							{
								m_watchStartedProcesses->addStartedProcess(pid);
							}

							const ProcessStatus* process = m_processes.getProcess(name);
							if (process)
							{
								m_entitiesTotal = process->config.entities.size();
								for (size_t i = 0; i < process->config.entities.size(); ++i)
								{
									const EntityConfig& entityConfig = process->config.entities[i];
									const std::string entityName = entityConfig.entityName;
									const std::string endpoint = entityConfig.endpoint;
									PeerId peerId;
									IRemoteEntity& entity = m_peerCache.getPeer(entityName, endpoint, peerId);
									NoData nodata;
									entity.requestReply<PingEntityReply>(peerId, PingEntity(), [this, entityConfig](PeerId peerId, Status status, const std::shared_ptr<PingEntityReply>& reply) {
										if (status == Status::STATUS_OK)
										{
											++m_entitiesStarted;
										}
									});
								}
							}
						}
						else
						{
                            process->timeRunning = time(nullptr);
                            putReport(protocol, *process, ReportId::REPORTID_STATECHANGE);
                            ok = false;
							streamError << "could not start process " << process->config.id;
							process->state = ProcessState::STATE_ERROR_START;
                            putReport(protocol, *process, ReportId::REPORTID_STATECHANGE);
                        }
					}
				}
				else if (process->state == ProcessState::STATE_STARTING)
				{
					bool up = (m_entitiesStarted == m_entitiesTotal);
					if (up)
					{
						process->state = ProcessState::STATE_ON;
						process->timeDone = time(nullptr);
						putReport(protocol, *process, ReportId::REPORTID_STATECHANGE);
					}
				}
			}
			else
			{
				ok = false;
				streamError << "Timeout reached at starting " << process->config.id;
				process->state = ProcessState::STATE_START_TIMEOUT;
                putReport(protocol, *process, ReportId::REPORTID_STATECHANGE);
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


bool TransactionStart::execute(std::vector<ReportProcessStatus>& report, bool& startupError)
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

	if (stable)
	{
		ProcessStatus* process = m_processes.getProcess(m_owner);
		ProcessState state;
		if (process)
		{
			state = process->state;
			if (process->startup)
			{
				if (state == ProcessState::STATE_START_TIMEOUT ||
					state == ProcessState::STATE_ERROR_START)
				{
					startupError = true;
				}
				process->startup = false;
			}
		}
		if (m_requestContext)
		{
			if (state == ProcessState::STATE_ON)
			{
				m_requestContext->reply(Status::STATUS_OK);
			}
			else
			{
				m_requestContext->reply(Status::STATUS_REQUEST_PROCESSING_ERROR);
			}
			m_requestContext = nullptr;
		}
	}
	return !stable;	// true = continue
}

//void TransactionStart::abort()
//{
//	ProcessStatus* process = m_processes.getProcess(m_owner);
//	if (process)
//	{
//		if (process->state == ProcessState::STATE_STARTING)
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

