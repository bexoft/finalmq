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

#include "peercache.h"
#include "platformspecific.h"
#include "finalmq/helpers/Utils.h"
#include "defines.h"

#include <algorithm>

#define MODULENAME	"processServer"


PeerCache::PeerCache()
{
}

PeerCache::~PeerCache()
{
}

void PeerCache::start(int reconnectInterval, int totalReconnectDuration)
{
	m_reconnectInterval = reconnectInterval;
	m_totalReconnectDuration = totalReconnectDuration;
	m_remoteEntityContainer.init(m_executor);
	m_remoteEntityContainer.registerEntity(&m_remoteEntity);
}



IRemoteEntity& PeerCache::getPeer(const std::string& name, const std::string& endpoint, PeerId& peerId)
{
	std::unique_lock<std::mutex> lock(m_mtx);
	const std::string fullname = endpoint + "!" + name;

	const auto itPeerId = m_cachePeerIds.find(fullname);
	if (itPeerId != m_cachePeerIds.end())
	{
		peerId = itPeerId->second;
	}
	else
	{
		SessionInfo session;
		const auto itSession = m_cacheSessions.find(endpoint);
		if (itSession != m_cacheSessions.end())
		{
			session = itSession->second;
		}
		else
		{
			session = m_remoteEntityContainer.connect(endpoint, ConnectProperties{ {},{m_reconnectInterval, m_totalReconnectDuration} });
			m_cacheSessions[endpoint] = session;
		}

		peerId = m_remoteEntity.connect(session, name);
		m_cachePeerIds[fullname] = peerId;
	}

	return m_remoteEntity;
}

void PeerCache::removePeer(const std::string& endpoint, const std::string& name, int pid)
{
	std::unique_lock<std::mutex> lock(m_mtx);
	const std::string fullname = endpoint + "!" + name;

	const auto itPeers = m_cachePeerIds.find(fullname);
	if (itPeers != m_cachePeerIds.end())
	{
		m_cachePeerIds.erase(itPeers);
	}

	const auto itSessions = m_cacheSessions.find(endpoint);
	if (itSessions != m_cacheSessions.end())
	{
		m_cacheSessions.erase(itSessions);
	}
}

const IExecutorPtr& PeerCache::getExecutor() const
{
	return m_executor;
}
