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

#include "watchprocesses.h"
#include "platformspecific.h"
#include "finalmq/helpers/Utils.h"
#include "defines.h"

#include <algorithm>

#define MODULENAME	"processServer"

constexpr int TIMEOUT_WATCHLOOP_MIN = 5;	// [s]
constexpr int THREAD_POLL_INTERVAL = 100;	// [ms]



WatchProcesses::WatchProcesses(IProcessServer& processServer)
	: m_processServer(processServer)
	, m_timeWatched(Helper::getClockSecond())
{
}

WatchProcesses::~WatchProcesses()
{
	m_terminate = true;
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void WatchProcesses::start(int pollEntityTimeinterval, int pollEntityTimeReaction)
{
	m_pollEntityTimeinterval = pollEntityTimeinterval / THREAD_POLL_INTERVAL;
	m_pollEntityTimeReaction = pollEntityTimeReaction / THREAD_POLL_INTERVAL;
	m_timeoutWatchLoop = (pollEntityTimeinterval * 2) / 1000;
	if (m_timeoutWatchLoop < TIMEOUT_WATCHLOOP_MIN)
	{
		m_timeoutWatchLoop = TIMEOUT_WATCHLOOP_MIN;
	}

	m_thread = std::thread([this]() {
		run();
	});

	m_peerCache.start(1000, 0);
}


std::vector<WatchProcesses::FailedProcess> WatchProcesses::getFailedPids() const
{
	std::unique_lock<std::mutex> lock(m_mtx);
	return m_failedProcesses;
}

bool WatchProcesses::loopOk()
{
	std::unique_lock<std::mutex> lock(m_mtx);
	const auto now = Helper::getClockSecond();
	const auto ok = (now <= m_timeWatched + m_timeoutWatchLoop);
	return ok;
}


void WatchProcesses::eraseAllNotExistingFailedPids(const std::vector<ProcessStatus>& processes)
{
	for (size_t i=0; i<m_failedProcesses.size(); i++)
	{
		int pidFail = m_failedProcesses[i].m_pid;
		bool found = false;
		for (size_t n=0; n<processes.size() && !found; n++)
		{
			const ProcessStatus& process = processes[n];
			if (process.pid == pidFail &&
				process.state == ProcessState::STATE_ON)
			{
				found = true;
			}
		}
		if (!found)
		{
			m_failedProcesses.erase(m_failedProcesses.begin() + i);
			i--;
		}
	}
}


bool contains(const std::vector<WatchProcesses::FailedProcess>& failedProcesses, int pid)
{
	for (size_t i=0; i<failedProcesses.size(); i++)
	{
		if (failedProcesses[i].m_pid == pid)
		{
			return true;
		}
	}
	return false;
}


static std::string getPollId(const std::string& processConfigId, const std::string& endpoint, const std::string& entityName)
{
	return processConfigId + '!' + endpoint + '!' + entityName;
}


void WatchProcesses::checkUnansweredPolls()
{
	static const std::string errorReason;
	for (auto it = m_polledEntities.begin(); it != m_polledEntities.end(); ++it)
	{
		EntityInfo& entityInfo = it->second;
		m_peerCache.removePeer(entityInfo.endpoint, entityInfo.entityName, entityInfo.pid);
		m_failedProcesses.emplace_back(entityInfo.pid, errorReason, RecoverMode::RECOVERMODE_NONE);
	}
	m_polledEntities.clear();
}



// Thread
void WatchProcesses::run()
{
	int cnt = 0;
	while (!m_terminate.wait(THREAD_POLL_INTERVAL))
	{
		m_peerCache.getExecutor()->runAvailableActions([this]() {
			return m_terminate.getValue();
		});
		
		if ((cnt > m_cntPoll + m_pollEntityTimeReaction) && !m_terminate.getValue())
		{
			checkUnansweredPolls();
		}

		if (((++cnt % m_pollEntityTimeinterval) == 0) && !m_terminate.getValue())
		{
			checkUnansweredPolls();
			m_cntPoll = cnt;
			std::vector<ProcessStatus> processes;
			m_processServer.getAllProcesses(processes);

			std::unique_lock<std::mutex> lock(m_mtx);
			eraseAllNotExistingFailedPids(processes);
			std::vector<FailedProcess> failedProcesses = m_failedProcesses;
			m_timeWatched = Helper::getClockSecond();
			lock.unlock();

			for (size_t i = 0; i < processes.size(); i++)
			{
				const ProcessStatus& process = processes[i];
				if (process.state == ProcessState::STATE_ON &&
					process.pid != 0 &&
					Utils::existsProcess(process.pid) &&
					!contains(failedProcesses, process.pid))
				{
					std::string errorReason;
					RecoverMode recoverMode;
					for (size_t i = 0; i < process.config.entities.size() && errorReason.empty(); i++)
					{
						const EntityConfig& entity = process.config.entities[i];
						if (entity.watchMode == WatchMode::WATCHMODE_POLL)
						{
							const std::string entityName = entity.entityName;
							const std::string endpoint = entity.endpoint;
							const std::string pollId = getPollId(process.config.id, endpoint, entityName);
							m_polledEntities[pollId] = EntityInfo{endpoint, entityName, process.pid};
							PeerId peerId;
							IRemoteEntity& entity = m_peerCache.getPeer(entityName, endpoint, peerId);
							NoData nodata;
							entity.requestReply<PollEntityReply>(peerId, "pollentity", nodata, [this, pollId](PeerId peerId, Status status, const std::shared_ptr<PollEntityReply>& reply) {
								const auto it = m_polledEntities.find(pollId);
								if (it != m_polledEntities.end())
								{
									EntityInfo& entityInfo = it->second;
									std::string errorReason = status.toString();
									RecoverMode recoverMode = RecoverMode::RECOVERMODE_NONE;
									if (reply && !reply->errorReason.empty())
									{
										errorReason = reply->errorReason;
										recoverMode = reply->recoverMode;
									}
									if (!reply || !reply->errorReason.empty())
									{
										m_peerCache.removePeer(entityInfo.endpoint, entityInfo.entityName, entityInfo.pid);
										m_failedProcesses.emplace_back(entityInfo.pid, errorReason, recoverMode);
									}
									m_polledEntities.erase(it);
								}
							});
						}
					}
				}
			}
		}
	}
}


