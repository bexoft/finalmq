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
#include "peercache.h"

#include <string>
#include <vector>


using namespace finalmq;
using namespace finalmq::fmqprocess;

struct IProcessServer
{
	virtual ~IProcessServer() = default;
	virtual void getAllProcesses(std::vector<ProcessStatus>& processes) const = 0;
};


class WatchProcesses
{
public:
	struct FailedProcess
	{
		FailedProcess()
			: m_pid(0)
            , m_errorReason()
			, m_recoverMode(RecoverMode::RECOVERMODE_NONE)
		{
		}
        FailedProcess(int pid, const std::string& errorReason, RecoverMode recoverMode)
			: m_pid(pid)
			, m_errorReason(errorReason)
			, m_recoverMode(recoverMode)
		{
		}
        int         m_pid;
        std::string m_errorReason;
		RecoverMode m_recoverMode;
	};

	WatchProcesses(IProcessServer& processServer);
	~WatchProcesses();

	void start(int pollEntityTimeinterval, int pollEntityTimeReaction);
	std::vector<FailedProcess> getFailedPids() const;
	bool loopOk();

private:

	struct EntityInfo
	{
		std::string endpoint;
		std::string entityName;
		int pid;
	};

	// Thread
	void run();

	void eraseAllNotExistingFailedPids(const std::vector<ProcessStatus>& processes);
	void checkUnansweredPolls();

	IProcessServer&							m_processServer;
	time_t									m_timeWatched;
	std::vector<FailedProcess>				m_failedProcesses;
	std::thread								m_thread;
	mutable std::mutex						m_mtx;
	CondVar									m_terminate{};
	PeerCache								m_peerCache;
	int										m_pollEntityTimeinterval;
	int										m_pollEntityTimeReaction;
	std::unordered_map<std::string, EntityInfo> m_polledEntities;
	int										m_cntPoll;
	int										m_timeoutWatchLoop;
};


