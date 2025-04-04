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

#include "finalmq/protocolsession/ProtocolSessionContainer.h"
#include "finalmq/streamconnection/StreamConnectionContainer.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"


namespace finalmq {

ProtocolBind::ProtocolBind(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, IProtocolFactoryPtr protocolFactory, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_executorPollerThread(executorPollerThread)
    , m_protocolFactory(protocolFactory)
    , m_protocolSessionList(protocolSessionList)
    , m_bindProperties(bindProperties)
    , m_contentType(contentType)
{

}

// IStreamConnectionCallback
hybrid_ptr<IStreamConnectionCallback> ProtocolBind::connected(const IStreamConnectionPtr& connection)
{
    IProtocolPtr protocol;
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    if (protocolSessionList)
    {
        protocol = m_protocolFactory->createProtocol(m_bindProperties.protocolData);
        assert(protocol);
        IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(m_callback, m_executor, m_executorPollerThread, protocol, protocolSessionList, m_bindProperties, m_contentType);
        protocolSession->setConnection(connection, !protocol->doesSupportSession());
    }
    return std::weak_ptr<IStreamConnectionCallback>(protocol);
}

void ProtocolBind::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    // should never be called, because the callback will be overriden by connected
    assert(false);
}

bool ProtocolBind::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& /*socket*/, int /*bytesToRead*/)
{
    // should never be called, because the callback will be overriden by connected
    assert(false);
    return false;
}



//////////////////////////////

ProtocolSessionContainer::ProtocolSessionContainer()
    : m_protocolSessionList(std::make_shared<ProtocolSessionList>())
    , m_streamConnectionContainer(std::make_shared<StreamConnectionContainer>())
{
    m_executorPollerThread = m_streamConnectionContainer->getPollerThreadExecutor();
}

ProtocolSessionContainer::~ProtocolSessionContainer()
{
    terminatePollerLoop();
}

// IProtocolSessionContainer
void ProtocolSessionContainer::init(const IExecutorPtr& executor, int cycleTime, FuncTimer funcTimer, int checkReconnectInterval)
{
    m_executor = executor;
    std::shared_ptr<FuncTimer> pFuncTimer = funcTimer ? std::make_shared<FuncTimer>(std::move(funcTimer)) : nullptr;
    m_streamConnectionContainer->init(cycleTime, [this, pFuncTimer/*, counterInterval*/]() {
        if (pFuncTimer)
        {
            if (m_executor)
            {
                m_executor->addAction([pFuncTimer]() {
                    (*pFuncTimer)();
                });
            }
            else
            {
                (*pFuncTimer)();
            }
        }
        std::vector< IProtocolSessionPrivatePtr > sessions = m_protocolSessionList->getAllSessions();
        for (size_t i = 0; i < sessions.size(); ++i)
        {
            const IProtocolSessionPrivatePtr& session = sessions[i];
            assert(session);
            session->cycleTime();
        }
    }, checkReconnectInterval);
    if (m_executor)
    {
        m_thread = std::thread([this]() { m_streamConnectionContainer->run(); });
    }
}

static std::string endpointToStreamEndpoint(const std::string& endpoint, std::string* protocolName = nullptr)
{
    std::string endpointToUse;
    size_t ixEndOfEndpoint = endpoint.find('{');
    if (ixEndOfEndpoint == std::string::npos)
    {
        endpointToUse = endpoint;
    }
    else
    {
        endpointToUse = endpoint.substr(0, ixEndOfEndpoint);
    }

    size_t ixEndpoint = endpointToUse.find_last_of(':');
    if (ixEndpoint == std::string::npos)
    {
        return "";
    }

    if (protocolName)
    {
        *protocolName = endpointToUse.substr(ixEndpoint + 1, endpointToUse.size() - (ixEndpoint + 1));
    }

    return endpointToUse.substr(0, ixEndpoint);
}


int ProtocolSessionContainer::bind(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, const BindProperties& bindProperties, int contentType)
{
    std::string protocolName;
    const std::string endpointStreamConnection = endpointToStreamEndpoint(endpoint, &protocolName);
    if (endpointStreamConnection.empty())
    {
        return -1;
    }

    IProtocolFactoryPtr protocolFactory = ProtocolRegistry::instance().getProtocolFactory(protocolName);
    if (!protocolFactory)
    {
        return -1;
    }

    BindProperties bindPropertiesToUse = bindProperties;
    StreamConnectionContainer::getBindPropertiesFromEndpoint(endpoint, bindPropertiesToUse);

    int err = -1;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_endpoint2Bind.find(endpoint);
    if (it == m_endpoint2Bind.end())
    {
        ProtocolBindPtr bind = std::make_shared<ProtocolBind>(callback, m_executor, m_executorPollerThread, protocolFactory, m_protocolSessionList, bindPropertiesToUse, contentType);
        m_endpoint2Bind[endpoint] = bind;
        lock.unlock();

        err = m_streamConnectionContainer->bind(endpointStreamConnection, bind, bindPropertiesToUse);
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
        const std::string endpointStreamConnection = endpointToStreamEndpoint(endpoint);
        m_streamConnectionContainer->unbind(endpointStreamConnection);
    }
}

IProtocolSessionPtr ProtocolSessionContainer::connect(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, const ConnectProperties& connectProperties, int contentType)
{
    std::string endpointToUse;
    size_t ixEndOfEndpoint = endpoint.find('{');
    if (ixEndOfEndpoint == std::string::npos)
    {
        endpointToUse = endpoint;
    }
    else
    {
        endpointToUse = endpoint.substr(0, ixEndOfEndpoint);
    }
    size_t ixEndpoint = endpointToUse.find_last_of(':');
    if (ixEndpoint == std::string::npos)
    {
        return nullptr;
    }
    std::string protocolName = endpointToUse.substr(ixEndpoint + 1, endpointToUse.size() - (ixEndpoint + 1));
    IProtocolFactoryPtr protocolFactory = ProtocolRegistry::instance().getProtocolFactory(protocolName);
    if (!protocolFactory)
    {
        return nullptr;
    }

    std::string endpointStreamConnection = endpointToUse.substr(0, ixEndpoint);

    ConnectProperties connectPropertiesToUse = connectProperties;
    StreamConnectionContainer::getConnectPropertiesFromEndpoint(endpoint, connectPropertiesToUse);

    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(callback, m_executor, m_executorPollerThread, protocolFactory, m_protocolSessionList, m_streamConnectionContainer, endpointStreamConnection, connectPropertiesToUse, contentType);
    protocolSession->connect();
    return protocolSession;
}


//IProtocolSessionPtr ProtocolSessionContainer::createSession(hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, int contentType)
//{
//    assert(protocol);
//    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(callback, m_executor, m_executorPollerThread, protocol, m_protocolSessionList, m_streamConnectionContainer, contentType);
//    protocolSession->createConnection();
//    return protocolSession;
//}
//

IProtocolSessionPtr ProtocolSessionContainer::createSession(hybrid_ptr<IProtocolSessionCallback> callback)
{
    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(callback, m_executor, m_executorPollerThread, m_protocolSessionList, m_streamConnectionContainer);
    protocolSession->setConnection(nullptr, true);
    return protocolSession;
}



std::vector< IProtocolSessionPtr > ProtocolSessionContainer::getAllSessions() const
{
    std::vector< IProtocolSessionPrivatePtr > protocolSessions = m_protocolSessionList->getAllSessions();
    return { std::make_move_iterator(protocolSessions.begin()), std::make_move_iterator(protocolSessions.end()) };
}

IProtocolSessionPtr ProtocolSessionContainer::getSession(std::int64_t sessionId) const
{
    IProtocolSessionPtr protocolSession = m_protocolSessionList->getSession(sessionId);
    return protocolSession;
}


void ProtocolSessionContainer::run()
{
    if (m_executor == nullptr)
    {
        m_streamConnectionContainer->run();
    }
}


void ProtocolSessionContainer::terminatePollerLoop()
{
    std::vector< IProtocolSessionPtr > sessions = getAllSessions();
    for (size_t i = 0; i < sessions.size(); ++i)
    {
        sessions[i]->disconnect();
    }

    m_streamConnectionContainer->terminatePollerLoop();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

IExecutorPtr ProtocolSessionContainer::getExecutor() const
{
    return m_executor ? m_executor : m_executorPollerThread;
}


}   // namespace finalmq
