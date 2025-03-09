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
#include "finalmq/remoteentity/RemoteEntity.h"
#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/helpers/Executor.h"

#include <string>
#include <vector>


using namespace finalmq;
using namespace finalmq::fmqprocess;


class PeerCache
{
public:
	PeerCache();
	~PeerCache();

	void start(int reconnectInterval, int totalReconnectDuration);
	IRemoteEntity& getPeer(const std::string& name, const std::string& endpoint, PeerId& peerId);
	void removePeer(const std::string& endpoint, const std::string& name, int pid);
	const IExecutorPtr& getExecutor() const;

private:

	int												m_reconnectInterval{};
	int												m_totalReconnectDuration{};
	std::unordered_map<std::string, PeerId>			m_cachePeerIds;
	std::unordered_map<std::string, SessionInfo>	m_cacheSessions;
	RemoteEntity									m_remoteEntity;
	RemoteEntityContainer							m_remoteEntityContainer;
	const IExecutorPtr								m_executor = std::make_shared<Executor>();
	mutable std::mutex								m_mtx;
};


