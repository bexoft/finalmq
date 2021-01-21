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


#include "finalmq/protocolconnection/ProtocolSessionList.h"


namespace finalmq {


ProtocolSessionList::ProtocolSessionList()
{
}

ProtocolSessionList::~ProtocolSessionList()
{
}

std::int64_t ProtocolSessionList::addProtocolSession(IProtocolSessionPtr ProtocolSession)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::int64_t sessionId = m_nextSessionId++;
    m_connectionId2ProtocolSession[sessionId] = ProtocolSession;
    lock.unlock();
    return sessionId;
}

void ProtocolSessionList::removeProtocolSession(std::int64_t sessionId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connectionId2ProtocolSession.erase(sessionId);
    lock.unlock();
}

std::vector< IProtocolSessionPtr > ProtocolSessionList::getAllSessions() const
{
    std::vector< IProtocolSessionPtr > ProtocolSessions;
    std::unique_lock<std::mutex> lock(m_mutex);
    ProtocolSessions.reserve(m_connectionId2ProtocolSession.size());
    for (auto it = m_connectionId2ProtocolSession.begin(); it != m_connectionId2ProtocolSession.end(); ++it)
    {
        ProtocolSessions.push_back(it->second);
    }
    lock.unlock();
    return ProtocolSessions;
}

IProtocolSessionPtr ProtocolSessionList::getSession(std::int64_t sessionId) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_connectionId2ProtocolSession.find(sessionId);
    if (it != m_connectionId2ProtocolSession.end())
    {
        return it->second;
    }
    return nullptr;
}

}   // namespace finalmq
