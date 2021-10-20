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

#include "finalmq/protocolsession/ProtocolSession.h"
#include "finalmq/streamconnection/StreamConnectionContainer.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"

#include <assert.h>


namespace finalmq {

const static std::string FMQ_CONNECTION_ID = "fmq_echo_connid";



ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_executorPollerThread(executorPollerThread)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_bindProperties(bindProperties)
{
    initProtocolValues();
}

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpointStreamConnection, const ConnectProperties& connectProperties, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_executorPollerThread(executorPollerThread)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_streamConnectionContainer(streamConnectionContainer)
    , m_endpointStreamConnection(endpointStreamConnection)
    , m_connectionProperties(connectProperties)
{
    initProtocolValues();
}

//ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, int contentType)
//    : m_callback(callback)
//    , m_executor(executor)
//    , m_executorPollerThread(executorPollerThread)
//    , m_protocol(protocol)
//    , m_protocolSessionList(protocolSessionList)
//    , m_contentType(contentType)
//    , m_streamConnectionContainer(streamConnectionContainer)
//{
//    initProtocolValues();
//}
//

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer)
    : m_callback(callback)
    , m_executor(executor)
    , m_executorPollerThread(executorPollerThread)
    , m_protocolSessionList(protocolSessionList)
    , m_streamConnectionContainer(streamConnectionContainer)
{
}



ProtocolSession::~ProtocolSession()
{

}



void ProtocolSession::initProtocolValues()
{
    IProtocolPtr& protocol = m_protocol;
    assert(protocol);
    m_protocolId = protocol->getProtocolId();
    m_protocolFlagMessagesResendable = protocol->areMessagesResendable();
    m_protocolFlagSupportMetainfo = protocol->doesSupportMetainfo();
    m_protocolFlagNeedsReply = protocol->needsReply();
    m_protocolFlagIsMultiConnectionSession = protocol->isMultiConnectionSession();
    m_protocolFlagIsSendRequestByPoll = protocol->isSendRequestByPoll();
    m_protocolFlagSupportFileTransfer = protocol->doesSupportFileTransfer();
    m_messageFactory = protocol->getMessageFactory();

    m_protocolSet.store(true, std::memory_order_release);
}




// IProtocolSessionPrivate

bool ProtocolSession::connect()
{
    assert(m_streamConnectionContainer);
    assert(m_protocol);
    IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocol));
    setConnection(connection, true);
    bool res = m_streamConnectionContainer->connect(connection, m_endpointStreamConnection, m_connectionProperties);
    return res;
}


void ProtocolSession::createConnection()
{
    assert(m_streamConnectionContainer);
    IStreamConnectionPtr connection;
    if (m_protocol)
    {
        connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocol));
    }
    setConnection(connection, true);
}


int64_t ProtocolSession::setConnection(const IStreamConnectionPtr& connection, bool verified)
{
    if (m_protocol)
    {
        initProtocolValues();
        m_protocol->setCallback(shared_from_this());
        m_protocol->setConnection(connection);
    }
    if (connection)
    {
        m_connectionId = connection->getConnectionId();
        bool incomingConnection = connection->getConnectionData().incomingConnection;
//        std::unique_lock<std::mutex> lock(m_mutex);
        m_incomingConnection = incomingConnection;
        if (m_incomingConnection)
        {
            m_activityTimer.setTimeout(m_activityTimeout);
        }
    }
    addSessionToList(verified);
    return m_sessionId;
}




// IProtocolSession
IMessagePtr ProtocolSession::createMessage() const
{
    if (m_protocolSet.load(std::memory_order_acquire) && m_messageFactory)
    {
        return m_messageFactory();
    }
    return std::make_shared<ProtocolMessage>(0);
}

IMessagePtr ProtocolSession::convertMessageToProtocol(const IMessagePtr& msg)
{
    IMessagePtr message = msg;
    if (message->getProtocolId() != m_protocolId ||
        (!m_protocolFlagMessagesResendable && message->wasSent()) ||
        (message->getTotalSendPayloadSize() == 0 && msg->getReceivePayload().second > 0))
    {
        message = nullptr;
        if (m_protocolFlagMessagesResendable)
        {
            message = msg->getMessage(m_protocolId);
        }
        if (message == nullptr)
        {
            message = createMessage();
            message->getAllMetainfo() = msg->getAllMetainfo();
            if (msg->getTotalSendPayloadSize() > 0)
            {
                ssize_t sizePayload = msg->getTotalSendPayloadSize();
                char* payload = message->addSendPayload(sizePayload);
                const std::list<BufferRef>& payloads = msg->getAllSendPayloads();
                ssize_t offset = 0;
                for (auto it = payloads.begin(); it != payloads.end(); ++it)
                {
                    const BufferRef& p = *it;
                    assert(offset + p.second == sizePayload);
                    memcpy(payload + offset, p.first, p.second);
                    offset += p.second;
                }
            }
            else
            {
                BufferRef receivePayload = msg->getReceivePayload();
                char* payload = message->addSendPayload(receivePayload.second);
                memcpy(payload, receivePayload.first, receivePayload.second);
            }
            if (m_protocolFlagMessagesResendable)
            {
                msg->addMessage(message);
            }
        }
    }

    return message;
}



void ProtocolSession::getProtocolFromConnectionId(IProtocolPtr& protocol, std::int64_t connectionId)
{
    assert(protocol->getConnection());
    if (connectionId != 0 && connectionId != protocol->getConnection()->getConnectionId())
    {
        protocol = nullptr;
        auto it = m_multiProtocols.find(connectionId);
        if (it != m_multiProtocols.end())
        {
            protocol = it->second;
        }
    }
}


bool ProtocolSession::sendMessage(const IMessagePtr& msg, bool isReply)
{
    bool ok = false;
    // lock whole function, because the protocol can be changed by setProtocol (lock also over sendMessage, because the protocol could increment a sequential message counter and the order of the counter shall be like the messages that are sent.)
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!isReply && m_protocolFlagIsSendRequestByPoll)
    {
        if (m_pollWaiting)
        {
            assert(m_pollReply == nullptr);
            IProtocolPtr protocol = m_protocol;
            if (m_protocolFlagIsMultiConnectionSession)
            {
                getProtocolFromConnectionId(protocol, m_pollConnectionId);
            }
            if (protocol)
            {
                m_pollCounter++;
                m_pollReply = protocol->pollReply({ msg });
                ok = sendMessage(m_pollReply, protocol);
                if (ok)
                {
                    m_pollReply = nullptr;
                }
                if (m_pollCountMax >= 0 && m_pollCounter >= m_pollCountMax)
                {
                    pollRelease();
                }
            }
            else
            {
                m_pollWaiting = false;
                m_pollConnectionId = 0;
                m_pollTimer.stop();
                m_pollMessages.push_back(msg);
            }
            ok = true;
        }
        else
        {
            m_pollMessages.push_back(msg);
            ok = true;
        }
        if (m_pollMaxRequests != -1 && static_cast<ssize_t>(m_pollMessages.size()) >= m_pollMaxRequests)
        {
            lock.unlock();
            disconnect();
        }
        return ok;
    }
    else
    {
        if (!m_protocol || (m_endpointStreamConnection.empty() && !m_incomingConnection))
        {
            m_messagesBuffered.push_back(msg);
            return true;
        }

        IProtocolPtr protocol = m_protocol;
        if (m_protocolFlagIsMultiConnectionSession)
        {
            Variant& echoData = msg->getEchoData();
            std::int64_t connectionId = echoData.getDataValue<std::int64_t>(FMQ_CONNECTION_ID);
            getProtocolFromConnectionId(protocol, connectionId);
        }

        ok = sendMessage(msg, protocol);
    }
    return ok;
}





std::int64_t ProtocolSession::getSessionId() const
{
    return m_sessionId;
}

ConnectionData ProtocolSession::getConnectionData() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection;
    if (m_protocol)
    {
        connection = m_protocol->getConnection();
    }
    if (m_protocolFlagIsMultiConnectionSession && 
        ((connection == nullptr) || connection->getConnectionData().connectionState != ConnectionState::CONNECTIONSTATE_CONNECTED))
    {
        for (auto it = m_multiProtocols.begin(); it != m_multiProtocols.end(); ++it)
        {
            IStreamConnectionPtr connIter = it->second->getConnection();
            if (connection == nullptr)
            {
                connection = connIter;
            }
            if (connIter && connIter->getConnectionData().connectionState == ConnectionState::CONNECTIONSTATE_CONNECTED)
            {
                connection = connIter;
                break;
            }
        }
    }
    lock.unlock();
    if (connection)
    {
        return connection->getConnectionData();
    }
    static ConnectionData defaultConnectionData;
    defaultConnectionData.connectionState = ConnectionState::CONNECTIONSTATE_DISCONNECTED;
    return defaultConnectionData;
}



int ProtocolSession::getContentType() const
{
    return m_contentType;
}


bool ProtocolSession::doesSupportMetainfo() const
{
    return m_protocolFlagSupportMetainfo;
}

bool ProtocolSession::needsReply() const
{
    return m_protocolFlagNeedsReply;
}

bool ProtocolSession::isMultiConnectionSession() const
{
    return m_protocolFlagIsMultiConnectionSession;
}

bool ProtocolSession::isSendRequestByPoll() const
{
    return m_protocolFlagIsSendRequestByPoll;
}

bool ProtocolSession::doesSupportFileTransfer() const
{
    return m_protocolFlagSupportFileTransfer;
}

void ProtocolSession::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    std::vector<IProtocolPtr> protocols;
    protocols.reserve(m_multiProtocols.size() + 1);
    if (m_protocol)
    {
        protocols.push_back(m_protocol);
    }
    for (auto it = m_multiProtocols.begin(); it != m_multiProtocols.end(); ++it)
    {
        if (it->second)
        {
            protocols.push_back(it->second);
        }
    }
    lock.unlock();

    for (size_t i = 0; i < protocols.size(); ++i)
    {
        protocols[i]->disconnect();
    }

    if (protocolSessionList)
    {
        protocolSessionList->removeProtocolSession(m_sessionId);
    }

    assert(m_executorPollerThread);

    std::weak_ptr<ProtocolSession> pThisWeak = shared_from_this();
    m_executorPollerThread->addAction([pThisWeak]() {
        std::shared_ptr<ProtocolSession> pThis = pThisWeak.lock();
        if (pThis)
        {
            pThis->disconnected();
        }
    });
}

void ProtocolSession::sendBufferedMessages()
{
    for (size_t i = 0; i < m_messagesBuffered.size(); ++i)
    {
        sendMessage(m_messagesBuffered[i], m_protocol);
    }
    m_messagesBuffered.clear();
}


//bool ProtocolSession::connect(const std::string& endpoint, const ConnectProperties& connectionProperties)
//{
//    assert(m_protocolConnection.protocol);
//
//    std::unique_lock<std::mutex> lock(m_mutex);
//    m_endpoint = endpoint;
//    m_connectionProperties = connectionProperties;
//    sendBufferedMessages();
//    lock.unlock();
//
//    bool res = m_streamConnectionContainer->connect(m_protocolConnection.connection, m_endpoint, m_connectionProperties);
//    return res;
//}

void ProtocolSession::addSessionToList(bool verified)
{
    if (m_sessionId == 0)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_verified = verified;
        IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
        lock.unlock();
        if (protocolSessionList)
        {
            m_sessionId = protocolSessionList->addProtocolSession(shared_from_this(), verified);
        }
    }
}


bool ProtocolSession::connect(const std::string& endpoint, const ConnectProperties& connectionProperties, int contentType)
{
    size_t ixEndpoint = endpoint.find_last_of(':');
    if (ixEndpoint == std::string::npos)
    {
        return false;
    }
    std::string protocolName = endpoint.substr(ixEndpoint+1, endpoint.size() - (ixEndpoint + 1));
    IProtocolFactoryPtr protocolFactory = ProtocolRegistry::instance().getProtocolFactory(protocolName);
    if (!protocolFactory)
    {
        return false;
    }

    IProtocolPtr protocol = protocolFactory->createProtocol(connectionProperties.protocolData);
    assert(protocol);


    assert(m_protocol == nullptr);
    assert(m_streamConnectionContainer);

    IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(protocol));

    std::unique_lock<std::mutex> lock(m_mutex);
    m_endpointStreamConnection = endpoint.substr(0, ixEndpoint);
    m_connectionProperties = connectionProperties;
    m_contentType = contentType;
    m_protocol = protocol;
    m_connectionId = connection->getConnectionId();
    initProtocolValues();
    m_protocol->setCallback(shared_from_this());
    m_protocol->setConnection(connection);
    sendBufferedMessages();
    lock.unlock();

    bool res = m_streamConnectionContainer->connect(connection, m_endpointStreamConnection, m_connectionProperties);
    return res;
}


IExecutorPtr ProtocolSession::getExecutor() const
{
    return m_executor ? m_executor : m_executorPollerThread;
}

void ProtocolSession::subscribe(const std::vector<std::string>& subscribtions)
{
    IProtocolPtr protocol;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_protocol)
    {
        protocol = m_protocol;
    }
    lock.unlock();
    if (protocol)
    {
        protocol->subscribe(subscribtions);
    }
}


// IProtocolCallback
void ProtocolSession::connected()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_triggerConnected)
    {
        m_triggerConnected = true;
        lock.unlock();

        if (m_executor)
        {
            std::weak_ptr<ProtocolSession> pThisWeak = shared_from_this();
            m_executor->addAction([pThisWeak]() {
                std::shared_ptr<ProtocolSession> pThis = pThisWeak.lock();
                if (pThis)
                {
                    auto callback = pThis->m_callback.lock();
                    if (callback)
                    {
                        callback->connected(pThis);
                    }
                }
            });
        }
        else
        {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->connected(shared_from_this());
            }
        }
    }
}

void ProtocolSession::disconnected()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    pollRelease();

    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    bool doDisconnected = (!m_triggerDisconnected) && (m_triggerConnected || !m_endpointStreamConnection.empty());
    m_triggerDisconnected = true;
    lock.unlock();

    if (doDisconnected)
    {
        if (m_executor)
        {
            std::weak_ptr<ProtocolSession> pThisWeak = shared_from_this();
            m_executor->addAction([pThisWeak]() {
                std::shared_ptr<ProtocolSession> pThis = pThisWeak.lock();
                if (pThis)
                {
                    auto callback = pThis->m_callback.lock();
                    if (callback)
                    {
                        callback->disconnected(pThis);
                    }
                }
            });
        }
        else
        {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->disconnected(shared_from_this());
            }
        }
    }

    if (protocolSessionList)
    {
        protocolSessionList->removeProtocolSession(m_sessionId);
    }
}

void ProtocolSession::disconnectedVirtualSession(const std::string& virtualSessionId)
{
    if (m_executor)
    {
        std::weak_ptr<ProtocolSession> pThisWeak = shared_from_this();
        m_executor->addAction([pThisWeak, virtualSessionId]() {
            std::shared_ptr<ProtocolSession> pThis = pThisWeak.lock();
            if (pThis)
            {
                auto callback = pThis->m_callback.lock();
                if (callback)
                {
                    callback->disconnectedVirtualSession(pThis, virtualSessionId);
                }
            }
        });
    }
    else
    {
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->disconnectedVirtualSession(shared_from_this(), virtualSessionId);
        }
    }
}



void ProtocolSession::received(const IMessagePtr& message, std::int64_t connectionId)
{
    if (m_incomingConnection)
    {
        m_activityTimer.setTimeout(m_activityTimeout);
    }
    bool writeChannelIdIntoEchoData = false;
    if (m_protocolFlagIsMultiConnectionSession && connectionId)
    {
        writeChannelIdIntoEchoData = (connectionId != m_connectionId);
    }

    if (writeChannelIdIntoEchoData)
    {
        Variant& echoData = message->getEchoData();
        if (echoData.getType() == VARTYPE_NONE)
        {
            echoData = VariantStruct{ {FMQ_CONNECTION_ID, connectionId} };
        }
        else
        {
            echoData.add(FMQ_CONNECTION_ID, connectionId);
        }
    }

    if (m_executor)
    {
        std::weak_ptr<ProtocolSession> pThisWeak = shared_from_this();
        m_executor->addAction([pThisWeak, message]() {
            std::shared_ptr<ProtocolSession> pThis = pThisWeak.lock();
            if (pThis)
            {
                auto callback = pThis->m_callback.lock();
                if (callback)
                {
                    callback->received(pThis, message);
                }
            }
        });
    }
    else
    {
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->received(shared_from_this(), message);
        }
    }
}

void ProtocolSession::socketConnected()
{
    if (m_executor)
    {
        std::weak_ptr<ProtocolSession> pThisWeak = shared_from_this();
        m_executor->addAction([pThisWeak]() {
            std::shared_ptr<ProtocolSession> pThis = pThisWeak.lock();
            if (pThis)
            {
                auto callback = pThis->m_callback.lock();
                if (callback)
                {
                    callback->socketConnected(pThis);
                }
            }
        });
    }
    else
    {
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->socketConnected(shared_from_this());
        }
    }
}

void ProtocolSession::socketDisconnected()
{
    if (m_executor)
    {
        std::weak_ptr<ProtocolSession> pThisWeak = shared_from_this();
        m_executor->addAction([pThisWeak]() {
            std::shared_ptr<ProtocolSession> pThis = pThisWeak.lock();
            if (pThis)
            {
                auto callback = pThis->m_callback.lock();
                if (callback)
                {
                    callback->socketDisconnected(pThis);
                }
            }
        });
    }
    else
    {
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->socketDisconnected(shared_from_this());
        }
    }
}

void ProtocolSession::reconnect()
{
    assert(m_streamConnectionContainer);
    assert(m_protocol);
    IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocol));
    std::unique_lock<std::mutex> lock(m_mutex);
    IProtocolPtr protocol = m_protocol;
    lock.unlock();
    m_connectionId = connection->getConnectionId();
    protocol->setConnection(connection);
    m_streamConnectionContainer->connect(connection, m_endpointStreamConnection, m_connectionProperties);
}



void ProtocolSession::cycleTime()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<IProtocolPtr> protocols;
    protocols.reserve(m_multiProtocols.size() + 1);
    if (m_protocol)
    {
        protocols.push_back(m_protocol);
    }
    for (auto it = m_multiProtocols.begin(); it != m_multiProtocols.end(); ++it)
    {
        if (it->second)
        {
            protocols.push_back(it->second);
        }
    }
    
    if (m_pollWaiting && m_pollTimer.isExpired())
    {
        pollRelease();
    }
    lock.unlock();

    bool activityTimerExpired = m_activityTimer.isExpired();
    if (activityTimerExpired)
    {
        disconnect();
    }

    for (size_t i = 0; i < protocols.size(); ++i)
    {
        protocols[i]->cycleTime();
    }
}


void ProtocolSession::cleanupMultiConnection()
{
    for (auto it = m_multiProtocols.begin(); it != m_multiProtocols.end(); )
    {
        IStreamConnectionPtr connection = it->second->getConnection();
        if (connection == nullptr || !connection->getSocket())
        {
            it = m_multiProtocols.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


void ProtocolSession::setProtocol(const IProtocolPtr& protocol)
{
    assert(protocol);

    std::int64_t connectionId = 0;
    IStreamConnectionPtr connection = protocol->getConnection();
    if (connection)
    {
        connectionId = connection->getConnectionId();
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_protocolFlagIsMultiConnectionSession)
    {
        if (connectionId != 0)
        {
            auto it = m_multiProtocols.find(connectionId);
            if (it == m_multiProtocols.end())
            {
                cleanupMultiConnection();
                m_multiProtocols[connectionId] = protocol;
            }
        }
    }
    else
    {
        protocol->moveOldProtocolState(*m_protocol);
        m_connectionId = connectionId;
        m_protocol = protocol;
    }
    protocol->setCallback(shared_from_this());
}



bool ProtocolSession::findSessionByName(const std::string& sessionName, const IProtocolPtr& protocol)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<IProtocolSessionList> list = m_protocolSessionList.lock();
    lock.unlock();
    if (list)
    {
        IProtocolSessionPrivatePtr session = list->findSessionByName(sessionName);
        if (session)
        {
            if (session.get() != this)
            {
                if (m_verified)
                {
                    session->setProtocol(protocol);
                }
                else
                {
                    assert(protocol == protocol);
                    disconnected(); // we are in the poller loop  thread
                    assert(m_protocol);
                    session->setProtocol(m_protocol);
                    m_connectionId = 0;
                    m_protocol = nullptr;
                    return true;
                }
            }
            return true;
        }
    }
    return false;
}

void ProtocolSession::setSessionNameInternal(const std::string& sessionName)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<IProtocolSessionList> list = m_protocolSessionList.lock();
    lock.unlock();

    if (list)
    {
        bool setSuccessful = list->setSessionName(m_sessionId, sessionName);
        if (setSuccessful)
        {
            lock.lock();
            m_verified = true;
            m_sessionName = sessionName;
            lock.unlock();
            connected();    // we are in the poller loop  thread
        }
    }
}


void ProtocolSession::setSessionName(const std::string& sessionName, const IProtocolPtr& protocol, const IStreamConnectionPtr& connection)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<IProtocolSessionList> list = m_protocolSessionList.lock();
    lock.unlock();

    if (list)
    {
        bool setSuccessful = list->setSessionName(m_sessionId, sessionName);
        if (setSuccessful)
        {
            lock.lock();
            m_verified = true;
            m_sessionName = sessionName;
            lock.unlock();
            connected();    // we are in the poller loop  thread
        }
        else
        {
            IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(m_callback, m_executor, m_executorPollerThread, protocol, m_protocolSessionList, m_bindProperties, m_contentType);
            protocolSession->setConnection(connection, false);
            protocolSession->setSessionNameInternal(sessionName);
        }
    }
}





void ProtocolSession::pollRelease()
{
    if (m_pollWaiting)
    {
        assert(m_pollReply == nullptr);
        IProtocolPtr protocol = m_protocol;
        if (m_protocolFlagIsMultiConnectionSession)
        {
            getProtocolFromConnectionId(protocol, m_pollConnectionId);
        }
        if (protocol)
        {
            IMessagePtr reply = protocol->pollReply({});
            Variant& controlData = reply->getControlData();
            controlData.add("fmq_poll_stop", true);
            sendMessage(reply, protocol);
        }
        m_pollWaiting = false;
        m_pollConnectionId = 0;
        m_pollTimer.stop();
    }
}



void ProtocolSession::pollRequest(std::int64_t connectionId, int timeout, int pollCountMax)
{
    if (m_incomingConnection)
    {
        m_activityTimer.setTimeout(m_activityTimeout);
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    pollRelease();
    m_pollCounter = 0;
    m_pollCountMax = pollCountMax;
    assert(!m_pollWaiting);
    if (m_pollReply)
    {
        IProtocolPtr protocol = m_protocol;
        if (m_protocolFlagIsMultiConnectionSession)
        {
            getProtocolFromConnectionId(protocol, connectionId);
        }
        m_pollCounter++;
        bool ok = sendMessage(m_pollReply, protocol);
        if (ok)
        {
            m_pollReply = nullptr;
        }
    }
    else if (!m_pollMessages.empty())
    {
        IProtocolPtr protocol = m_protocol;
        if (m_protocolFlagIsMultiConnectionSession)
        {
            getProtocolFromConnectionId(protocol, connectionId);
        }
        std::deque<IMessagePtr> messages;
        if (protocol)
        {
            messages = std::move(m_pollMessages);
            m_pollMessages.clear();
            m_pollCounter++;
            m_pollReply = protocol->pollReply(std::move(messages));
            int ok = sendMessage(m_pollReply, protocol);
            if (ok)
            {
                m_pollReply = nullptr;
            }
        }
    }
    m_pollWaiting = true;
    m_pollConnectionId = connectionId;
    if (timeout > 0)
    {
        m_pollTimer.setTimeout(timeout);
    }

    if (timeout == 0 || (m_pollCountMax >= 0 && m_pollCounter >= m_pollCountMax))
    {
        pollRelease();
    }
}




void ProtocolSession::activity()
{
//    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_incomingConnection)
    {
        m_activityTimer.setTimeout(m_activityTimeout);
    }
}


void ProtocolSession::setActivityTimeout(int timeout)
{
    m_activityTimeout = timeout;
}

void ProtocolSession::setPollMaxRequests(int maxRequests)
{
    m_pollMaxRequests = maxRequests;
}


void ProtocolSession::disconnectedMultiConnection(const IStreamConnectionPtr& connection)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::int64_t connectionId = connection->getConnectionId();
    if (m_pollWaiting && m_pollConnectionId == connectionId)
    {
        m_pollWaiting = false;
        m_pollConnectionId = 0;
        m_pollTimer.stop();
    }
    lock.unlock();

    if (!m_verified)
    {
        disconnected();
    }
}



bool ProtocolSession::sendMessage(const IMessagePtr& message, const IProtocolPtr& protocol)
{
    // the mutex must be locked before calling sendMessage, lock also over sendMessage, because the protocol could increment a sequential message counter and the order of the counter shall be like the messages that are sent.
    bool ok = false;
    if (protocol)
    {
        IMessagePtr messageProtocol = convertMessageToProtocol(message);
        ok = protocol->sendMessage(messageProtocol);
    }
    return ok;
}



}
