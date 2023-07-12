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

constexpr int64_t INSTANCEID_PREFIX = 0x0100000000000000ll;
constexpr int DEFAULT_MAX_SYNC_REQREP_CONNECTIONS = 6;
static const std::string PROPERTY_MAX_SYNC_REQREP_CONNECTIONS = "max_sync_reqrep_connections";


const static std::string FMQ_CONNECTION_ID = "fmq_echo_connid";



ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const IProtocolPtr& protocol, const std::shared_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_executorPollerThread(executorPollerThread)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_sessionId(protocolSessionList->getNextSessionId())
    , m_instanceId(m_sessionId | INSTANCEID_PREFIX)
    , m_contentType(contentType)
    , m_bindProperties(bindProperties)
    , m_protocolData(m_bindProperties.protocolData)
    , m_formatData(m_bindProperties.formatData)
{
}

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const IProtocolFactoryPtr& protocolFactory, const std::shared_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpointStreamConnection, const ConnectProperties& connectProperties, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_executorPollerThread(executorPollerThread)
    , m_protocolSessionList(protocolSessionList)
    , m_sessionId(protocolSessionList->getNextSessionId())
    , m_instanceId(m_sessionId | INSTANCEID_PREFIX)
    , m_contentType(contentType)
    , m_protocolFactory(protocolFactory)
    , m_streamConnectionContainer(streamConnectionContainer)
    , m_endpointStreamConnection(endpointStreamConnection)
    , m_connectionProperties(connectProperties)
    , m_protocolData(m_connectionProperties.protocolData)
    , m_formatData(m_connectionProperties.formatData)
{
    m_protocol = protocolFactory->createProtocol(m_connectionProperties.protocolData);
    assert(m_protocol);
}

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const std::shared_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer)
    : m_callback(callback)
    , m_executor(executor)
    , m_executorPollerThread(executorPollerThread)
    , m_protocolSessionList(protocolSessionList)
    , m_sessionId(protocolSessionList->getNextSessionId())
    , m_instanceId(m_sessionId | INSTANCEID_PREFIX)
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
    m_protocolFlagSynchronousRequestReply = protocol->isSynchronousRequestReply();
    m_messageFactory = protocol->getMessageFactory();

    if (m_protocolFlagSynchronousRequestReply)
    {
        m_maxSynchReqRepConnections = m_protocolData.getDataValue<int>(PROPERTY_MAX_SYNC_REQREP_CONNECTIONS);
        if (m_maxSynchReqRepConnections == 0)
        {
            m_maxSynchReqRepConnections = DEFAULT_MAX_SYNC_REQREP_CONNECTIONS;   // default connections
        }
    }

    m_protocolSet.store(true, std::memory_order_release);
}




// IProtocolSessionPrivate

bool ProtocolSession::connect()
{
    assert(m_protocol);
    bool res = false;
    if (m_protocol->isSynchronousRequestReply())
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        initProtocolValues();
        m_protocol = nullptr;
        IProtocolPtr protocol = createRequestConnection();
        IStreamConnectionPtr connection = protocol->getConnection();
        if (connection)
        {
            m_unallocatedConnections.insert(connection->getConnectionId());
        }
        lock.unlock();
        addSessionToList(true);
        res = true;
    }
    else
    {
        assert(m_streamConnectionContainer);
        IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocol));
        setConnection(connection, true);
        m_callConnect = true;
        res = m_streamConnectionContainer->connect(m_endpointStreamConnection, connection, m_connectionProperties);
    }
    return res;
}


//void ProtocolSession::createConnection()
//{
//    assert(m_streamConnectionContainer);
//    IStreamConnectionPtr connection;
//    if (m_protocol)
//    {
//        connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocol));
//    }
//    setConnection(connection, true);
//}


void ProtocolSession::setConnection(const IStreamConnectionPtr& connection, bool verified)
{
    if (connection)
    {
        if (m_protocol)
        {
            initProtocolValues();
            m_protocol->setCallback(shared_from_this());
            m_protocol->setConnection(connection);
        }

        m_connectionId = connection->getConnectionId();
        activity();
    }
    addSessionToList(verified);
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
    // mutext is already locked
    if (msg == nullptr)
    {
        return nullptr;
    }
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
            ssize_t sizePayload = msg->getTotalSendPayloadSize();
            if (msg->getTotalSendPayloadSize() > 0)
            {
                char* payload = message->addSendPayload(sizePayload);
                const std::list<BufferRef>& payloads = msg->getAllSendPayloads();
                ssize_t offset = 0;
                for (auto it = payloads.begin(); it != payloads.end(); ++it)
                {
                    const BufferRef& p = *it;
                    assert(offset + p.second <= sizePayload);
                    memcpy(payload + offset, p.first, p.second);
                    offset += p.second;
                }
                assert(offset == sizePayload);
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
    // mutext is already locked

    IStreamConnectionPtr connection = protocol->getConnection();
    if (protocol == nullptr || (connectionId != 0 && (connection == nullptr || connectionId != connection->getConnectionId())))
    {
        protocol = nullptr;
        auto it = m_multiProtocols.find(connectionId);
        if (it != m_multiProtocols.end())
        {
            protocol = it->second;
        }
    }
}


void ProtocolSession::sendMessage(const IMessagePtr& msg, bool isReply)
{
    // lock whole function, because the protocol can be changed by setProtocol (lock also over sendMessage, because the protocol could increment a sequential message counter and the order of the counter shall be like the messages that are sent.)
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!m_protocolFlagSynchronousRequestReply)
    {
        if (!isReply && m_protocolFlagIsSendRequestByPoll)
        {
            if (m_pollProtocol)
            {
                m_pollCounter++;
                IMessagePtr pollReply = m_pollProtocol->pollReply({ msg });
                sendMessage(pollReply, m_pollProtocol);
                if (m_pollCountMax >= 0 && m_pollCounter >= m_pollCountMax)
                {
                    pollRelease();
                }
            }
            else
            {
                m_pollMessages.push_back(msg);
            }
            if (m_pollMaxRequests != -1 && static_cast<ssize_t>(m_pollMessages.size()) >= m_pollMaxRequests)
            {
                lock.unlock();
                disconnect();
            }
        }
        else
        {
            if (!m_protocolSet.load(std::memory_order_relaxed))  // relaxed, because already locked
            {
                m_messagesBuffered.push_back(msg);
                return;
            }

            IProtocolPtr protocol = m_protocol;
            if (m_protocolFlagIsMultiConnectionSession)
            {
                Variant& echoData = msg->getEchoData();
                std::int64_t connectionId = echoData.getDataValue<std::int64_t>(FMQ_CONNECTION_ID);
                getProtocolFromConnectionId(protocol, connectionId);
            }

            sendMessage(msg, protocol);
        }
    }
    else
    {
        m_messagesBuffered.push_back(msg);
        sendNextRequests();
    }
}


bool ProtocolSession::hasPendingRequests() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_messagesBuffered.empty())
    {
        return true;
    }
    return false;
}

IProtocolPtr ProtocolSession::allocateRequestConnection()
{
    // m_mutex is already locked
    if (!m_unallocatedConnections.empty())
    {
        auto itUnallocated = m_unallocatedConnections.begin();
        assert(itUnallocated != m_unallocatedConnections.end());
        std::int64_t connectionId = *itUnallocated;
        m_unallocatedConnections.erase(itUnallocated);
        auto itConnection = m_multiProtocols.find(connectionId);
        if (itConnection != m_multiProtocols.end())
        {
            return itConnection->second;
        }
    }
    return createRequestConnection();
}

IProtocolPtr ProtocolSession::createRequestConnection()
{
    // m_mutex is already locked
    if (static_cast<int>(m_multiProtocols.size()) < m_maxSynchReqRepConnections)
    {
        assert(m_streamConnectionContainer);

        assert(m_protocolFactory);
        IProtocolPtr protocol = m_protocolFactory->createProtocol(m_protocolData);
        assert(protocol);

        IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(protocol));

        assert(m_protocol == nullptr);
        m_connectionId = connection->getConnectionId();
        m_multiProtocols[m_connectionId] = protocol;
        protocol->setCallback(shared_from_this());
        protocol->setConnection(connection);

        m_connectionProperties.config.totalReconnectDuration = 0;   // only try once, do not make retries to connect (in case of SyncReqRep)

        bool res = m_streamConnectionContainer->connect(m_endpointStreamConnection, connection, m_connectionProperties);
        if (res)
        {
            return protocol;
        }
    }

    return nullptr;
}

void ProtocolSession::sendNextRequests()
{
    // m_mutex is already locked
    while (true)
    {
        if (m_messagesBuffered.empty())
        {
            return;
        }

        IProtocolPtr protocol = allocateRequestConnection();
        if (protocol == nullptr)
        {
            return;
        }

        IStreamConnectionPtr connection = protocol->getConnection();
        if (connection == nullptr)
        {
            return;
        }

        std::int64_t connectionId = connection->getConnectionId();
        IMessagePtr message = m_messagesBuffered.front();
        m_messagesBuffered.pop_front();
        assert(message);
        m_runningRequests[connectionId] = std::move(message->getEchoData());
        sendMessage(message, protocol);
    }
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

bool ProtocolSession::isSynchronousRequestReply() const
{
    return m_protocolFlagSynchronousRequestReply;
}

void ProtocolSession::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_messagesBuffered.clear();
    m_pollMessages.clear();
    m_pollProtocol = nullptr;
    m_pollTimer.stop();
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
    }, m_instanceId);
}

void ProtocolSession::sendBufferedMessages()
{
    // mutext is already locked
    for (size_t i = 0; i < m_messagesBuffered.size(); ++i)
    {
        sendMessage(m_messagesBuffered[i], m_protocol);
    }
    m_messagesBuffered.clear();
}


void ProtocolSession::addSessionToList(bool verified)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_verified = verified;
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    lock.unlock();
    if (protocolSessionList)
    {
        protocolSessionList->addProtocolSession(shared_from_this(), m_sessionId, verified);
    }
}


bool ProtocolSession::connect(const std::string& endpoint, const ConnectProperties& connectionProperties, int contentType)
{
    if (m_protocolSet.load(std::memory_order_acquire))
    {
        return false;
    }

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

    m_protocolFactory = protocolFactory;

    IProtocolPtr protocol = protocolFactory->createProtocol(connectionProperties.protocolData);
    assert(protocol);

    bool res = false;
    if (protocol->isSynchronousRequestReply())
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        assert(m_protocol == nullptr);
        m_endpointStreamConnection = endpoint.substr(0, ixEndpoint);
        m_connectionProperties = connectionProperties;
        m_protocolData = m_connectionProperties.protocolData;
        m_formatData = m_connectionProperties.formatData;
        m_contentType = contentType;
        initProtocolValues();
        IProtocolPtr connection = createRequestConnection();
        if (connection)
        {
            m_unallocatedConnections.insert(connection->getProtocolId());
            sendNextRequests();
        }
        lock.unlock();
        res = true;
    }
    else
    {
        assert(m_streamConnectionContainer);
        IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(protocol));

        std::unique_lock<std::mutex> lock(m_mutex);
        assert(m_protocol == nullptr);
        m_endpointStreamConnection = endpoint.substr(0, ixEndpoint);
        m_connectionProperties = connectionProperties;
        m_protocolData = m_connectionProperties.protocolData;
        m_formatData = m_connectionProperties.formatData;
        m_contentType = contentType;
        m_protocol = protocol;
        m_connectionId = connection->getConnectionId();
        initProtocolValues();
        m_callConnect = true;
        m_protocol->setCallback(shared_from_this());
        m_protocol->setConnection(connection);
        sendBufferedMessages();
        lock.unlock();

        res = m_streamConnectionContainer->connect(m_endpointStreamConnection, connection, m_connectionProperties);
    }

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

const Variant& ProtocolSession::getFormatData() const
{
    return m_formatData;
}

// IProtocolCallback
void ProtocolSession::connected()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_triggeredConnected)
    {
        m_triggeredConnected = true;
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
            }, m_instanceId);
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
    m_messagesBuffered.clear();
    m_pollMessages.clear();
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    bool doDisconnected = (!m_triggeredDisconnected) && (m_triggeredConnected || m_callConnect);
    m_triggeredDisconnected = true;
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
            }, m_instanceId);
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
        }, m_instanceId);
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

const std::string FMQ_DISCONNECTED = "fmq_disconnected";


void ProtocolSession::received(const IMessagePtr& message, std::int64_t connectionId)
{
    if (m_protocolFlagSynchronousRequestReply)
    {
        bool foundRunningRequest = false;
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_runningRequests.find(connectionId);
        if (it != m_runningRequests.end())
        {
            message->getEchoData() = std::move(it->second);
            m_runningRequests.erase(it);
            foundRunningRequest = true;
        }
        const bool disconnected = (message->getControlData().getVariant(FMQ_DISCONNECTED) != nullptr);

        // in case of disconnected -> keep connection allocated, so that no one will use it till the disconnectedMultiConnection is called
        if (!disconnected)
        {
            // if not disconnected -> mark connection as unallocated
            m_unallocatedConnections.insert(connectionId);
            sendNextRequests();
        }

        if (!foundRunningRequest)
        {
            return;
        }
    }

    activity();

    bool writeChannelIdIntoEchoData = false;
    if (!m_protocolFlagSynchronousRequestReply && m_protocolFlagIsMultiConnectionSession && connectionId)
    {
        writeChannelIdIntoEchoData = (connectionId != m_connectionId);
    }

    if (writeChannelIdIntoEchoData)
    {
        Variant& echoData = message->getEchoData();
        echoData.add(FMQ_CONNECTION_ID, connectionId);
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
        }, m_instanceId);
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
        }, m_instanceId);
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
        }, m_instanceId);
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
    std::unique_lock<std::mutex> lock(m_mutex);
    IProtocolPtr protocol = m_protocol;
    std::string endpointStreamConnection = m_endpointStreamConnection;
    lock.unlock();

    assert(m_streamConnectionContainer);
    assert(protocol);
    IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(protocol));

    m_connectionId = connection->getConnectionId();
    protocol->setConnection(connection);
    m_streamConnectionContainer->connect(endpointStreamConnection, connection, m_connectionProperties);
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
    
    if (m_pollProtocol && m_pollTimer.isExpired())
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
            if (session->getContentType() == getContentType() && session->getFormatData() == getFormatData())
            {
                if (session.get() != this)
                {
                    assert(protocol);
                    if (m_verified)
                    {
                        session->setProtocol(protocol);
                    }
                    else
                    {
                        disconnected(); // we are in the poller loop thread

                        lock.lock();
                        assert(m_protocol == protocol);
                        m_protocol = nullptr;
                        m_connectionId = 0;
                        lock.unlock();

                        session->setProtocol(protocol);
                    }
                }
                return true;
            }
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
            connected();    // we are in the poller loop thread
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
            connected();    // we are in the poller loop thread
        }
        else
        {
            IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(m_callback, m_executor, m_executorPollerThread, protocol, list, m_bindProperties, m_contentType);
            protocolSession->setConnection(connection, false);
            protocolSession->setSessionNameInternal(sessionName);
        }
    }
}



void ProtocolSession::pollRelease()
{
    // mutext is already locked
    if (m_pollProtocol)
    {
        IMessagePtr reply = m_pollProtocol->pollReply({});
        if (reply)
        {
            Variant& controlData = reply->getControlData();
            controlData.add("fmq_poll_stop", true);
            sendMessage(reply, m_pollProtocol);
        }
        m_pollProtocol = nullptr;
        m_pollTimer.stop();
    }
}



void ProtocolSession::pollRequest(const IProtocolPtr& protocol, int timeout, int pollCountMax)
{
    assert(protocol);
    activity();
    std::unique_lock<std::mutex> lock(m_mutex);
    pollRelease();
    m_pollCounter = 0;
    m_pollCountMax = pollCountMax;
    m_pollProtocol = protocol;
    m_pollTimer.stop();
    if (timeout > 0)
    {
        m_pollTimer.setTimeout(timeout);
    }

    if (!m_pollMessages.empty())
    {
        m_pollCounter++;
        IMessagePtr pollReply = protocol->pollReply(std::move(m_pollMessages));
        m_pollMessages.clear();
        sendMessage(pollReply, protocol);
    }

    if (timeout == 0 || (m_pollCountMax >= 0 && m_pollCounter >= m_pollCountMax))
    {
        pollRelease();
    }
}




void ProtocolSession::activity()
{
    m_activityTimer.setTimeout(m_activityTimeout);
}


void ProtocolSession::setActivityTimeout(int timeout)
{
    m_activityTimeout = timeout;
}

void ProtocolSession::setPollMaxRequests(int maxRequests)
{
    m_pollMaxRequests = maxRequests;
}




void ProtocolSession::disconnectedMultiConnection(const IProtocolPtr& protocol)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_protocolFlagSynchronousRequestReply)
    {
        if (m_pollProtocol == protocol)
        {
            m_pollProtocol = nullptr;
            m_pollTimer.stop();
        }
    }

    assert(protocol);
    IStreamConnectionPtr connection = protocol->getConnection();
    if (connection)
    {
        std::int64_t connectionId = connection->getConnectionId();
        m_unallocatedConnections.erase(connectionId);
        m_multiProtocols.erase(connectionId);
        if (m_connectionId == connectionId)
        {
            m_protocol = nullptr;
            m_connectionId = 0;
        }
    }

    lock.unlock();
}



void ProtocolSession::sendMessage(const IMessagePtr& message, const IProtocolPtr& protocol)
{
    // the mutex must be locked before calling sendMessage, lock also over sendMessage, because the protocol could increment a sequential message counter and the order of the counter shall be like the messages that are sent.
    if (protocol && message)
    {
        IMessagePtr messageProtocol = convertMessageToProtocol(message);
        protocol->sendMessage(messageProtocol);
    }
}



}
