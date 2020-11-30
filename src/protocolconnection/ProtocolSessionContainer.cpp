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

#include "protocolconnection/ProtocolSessionContainer.h"
#include "streamconnection/StreamConnectionContainer.h"


namespace finalmq {

ProtocolBind::ProtocolBind(hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType)
    : m_callback(callback)
    , m_protocolFactory(protocolFactory)
    , m_protocolSessionList(protocolSessionList)
    , m_bindProperties(bindProperties)
    , m_contentType(contentType)
{

}

// IStreamConnectionCallback
hybrid_ptr<IStreamConnectionCallback> ProtocolBind::connected(const IStreamConnectionPtr& connection)
{
    IProtocolPtr protocol = m_protocolFactory->createProtocol();
    assert(protocol);
    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(m_callback, protocol, m_protocolSessionList, m_bindProperties, m_contentType);
    protocolSession->setConnection(connection);
    return std::weak_ptr<IStreamConnectionCallback>(protocolSession);
}

void ProtocolBind::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    // should never be called, because the callback will be overriden by connected
    assert(false);
}

void ProtocolBind::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& /*socket*/, int /*bytesToRead*/)
{
    // should never be called, because the callback will be overriden by connected
    assert(false);
}



//////////////////////////////

ProtocolSessionContainer::ProtocolSessionContainer()
    : m_protocolSessionList(std::make_shared<ProtocolSessionList>())
    , m_streamConnectionContainer(std::make_shared<StreamConnectionContainer>())
{

}

ProtocolSessionContainer::~ProtocolSessionContainer()
{

}

// IProtocolSessionContainer
void ProtocolSessionContainer::init(int cycleTime, int checkReconnectInterval)
{
    m_streamConnectionContainer->init(cycleTime, checkReconnectInterval);
}

int ProtocolSessionContainer::bind(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const BindProperties& bindProperties, int contentType)
{
    int err = 0;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_endpoint2Bind.find(endpoint);
    if (it == m_endpoint2Bind.end())
    {
        ProtocolBindPtr bind = std::make_shared<ProtocolBind>(callback, protocolFactory, m_protocolSessionList, bindProperties, contentType);
        m_endpoint2Bind[endpoint] = bind;
        lock.unlock();

        err = m_streamConnectionContainer->bind(endpoint, bind, bindProperties);
    }
    return err;
}

void ProtocolSessionContainer::unbind(const std::string& endpoint)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_endpoint2Bind.find(endpoint);
    if (it != m_endpoint2Bind.end())
    {
        m_endpoint2Bind.erase(it);
        m_streamConnectionContainer->unbind(endpoint);
    }
}

IProtocolSessionPtr ProtocolSessionContainer::connect(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const ConnectProperties& connectProperties, int contentType)
{
    assert(protocol);
    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(callback, protocol, m_protocolSessionList, m_streamConnectionContainer, endpoint, connectProperties, contentType);
    protocolSession->connect();
    return protocolSession;
}

std::vector< IProtocolSessionPtr > ProtocolSessionContainer::getAllSessions() const
{
    std::vector< IProtocolSessionPtr > protocolSessions = m_protocolSessionList->getAllSessions();
    return protocolSessions;
}

IProtocolSessionPtr ProtocolSessionContainer::getSession(std::int64_t sessionId) const
{
    IProtocolSessionPtr protocolSession = m_protocolSessionList->getSession(sessionId);
    return protocolSession;
}


void ProtocolSessionContainer::threadEntry()
{
    m_streamConnectionContainer->threadEntry();
}


bool ProtocolSessionContainer::terminatePollerLoop(int timeout)
{
    return m_streamConnectionContainer->terminatePollerLoop(timeout);
}



}   // namespace finalmq
