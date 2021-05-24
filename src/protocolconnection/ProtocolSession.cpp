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

#include "finalmq/protocolconnection/ProtocolSession.h"
#include "finalmq/streamconnection/StreamConnectionContainer.h"
#include "finalmq/protocolconnection/ProtocolMessage.h"

#include <assert.h>


namespace finalmq {

const static std::string FMQ_CONNECTION_ID = "_fmq_echo_connid";



ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_protocolConnection{ protocol, nullptr }
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_bindProperties(bindProperties)
{
    initProtocolValues();
}

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpoint, const ConnectProperties& connectProperties, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_protocolConnection{ protocol, nullptr }
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_streamConnectionContainer(streamConnectionContainer)
    , m_endpoint(endpoint)
    , m_connectionProperties(connectProperties)
{
    initProtocolValues();
}

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, int contentType)
    : m_callback(callback)
    , m_executor(executor)
    , m_protocolConnection{ protocol, nullptr }
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_streamConnectionContainer(streamConnectionContainer)
{
    initProtocolValues();
}


ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer)
    : m_callback(callback)
    , m_executor(executor)
    , m_protocolSessionList(protocolSessionList)
    , m_streamConnectionContainer(streamConnectionContainer)
{
    m_protocolSet.store(false, std::memory_order_release);
}



ProtocolSession::~ProtocolSession()
{

}



void ProtocolSession::initProtocolValues()
{
    IProtocolPtr& protocol = m_protocolConnection.protocol;
    assert(protocol);
    m_protocolId = protocol->getProtocolId();
    m_protocolFlagMessagesResendable = protocol->areMessagesResendable();
    m_protocolFlagSupportMetainfo = protocol->doesSupportMetainfo();
    m_protocolFlagNeedsReply = protocol->needsReply();
    m_protocolFlagIsMultiConnectionSession = protocol->isMultiConnectionSession();
    m_protocolFlagIsSendRequestByPoll = protocol->isSendRequestByPoll();
    m_messageFactory = protocol->getMessageFactory();

    m_protocolSet.store(true, std::memory_order_release);
}




// IProtocolSessionPrivate

bool ProtocolSession::connect()
{
    assert(m_streamConnectionContainer);
    assert(m_protocolConnection.protocol);
    IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocolConnection.protocol));
    setConnection(connection, true);
    bool res = m_streamConnectionContainer->connect(connection, m_endpoint, m_connectionProperties);
    return res;
}


void ProtocolSession::createConnection()
{
    assert(m_streamConnectionContainer);
    IStreamConnectionPtr connection;
    if (m_protocolConnection.protocol)
    {
        connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocolConnection.protocol));
    }
    setConnection(connection, true);
}


int64_t ProtocolSession::setConnection(const IStreamConnectionPtr& connection, bool verified)
{
    if (m_protocolConnection.protocol)
    {
        initProtocolValues();
        m_protocolConnection.protocol->setCallback(shared_from_this());
    }
    if (connection)
    {
        bool incomingConnection = connection->getConnectionData().incomingConnection;
        std::unique_lock<std::mutex> lock(m_mutex);
        m_incomingConnection = incomingConnection;
    }
    m_protocolConnection.connection = connection;
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



void ProtocolSession::getProtocolConnectionFromConnectionId(const ProtocolConnection*& protocolConnection, std::int64_t connectionId)
{
    if (connectionId != 0 && connectionId != protocolConnection->connection->getConnectionId())
    {
        protocolConnection = nullptr;
        auto it = m_multiConnections.find(connectionId);
        if (it != m_multiConnections.end())
        {
            protocolConnection = &it->second;
        }
    }
}


bool ProtocolSession::sendMessage(const IMessagePtr& msg, bool isReply)
{
    bool ok = false;
    // lock whole function, because the protocol can be changed by setProtocol (lock also over sendMessage, because the prepareMessageToSend could increment a sequential message counter and the order of the counter shall be like the messages that are sent.)
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!isReply && m_protocolFlagIsSendRequestByPoll)
    {
        if (m_pollWaiting)
        {
            m_pollWaiting = false;
            const ProtocolConnection* protocolConnection = &m_protocolConnection;
            if (m_protocolFlagIsMultiConnectionSession)
            {
                getProtocolConnectionFromConnectionId(protocolConnection, m_pollConnectionId);
            }
            IProtocolPtr protocol;
            if (protocolConnection)
            {
                protocol = protocolConnection->protocol;
            }
            std::int64_t connectionId = m_pollConnectionId;
            m_pollConnectionId = 0;
            lock.unlock();
            if (protocol)
            {
                IMessagePtr reply = protocol->pollReply({ msg });
                ok = sendMessageLocked(reply, connectionId);
            }
            if (!ok)
            {
                lock.lock();
                m_pollMessages.push_back(msg);
                lock.unlock();
                ok = true;
            }
        }
        else
        {
            m_pollMessages.push_back(msg);
            ok = true;
        }
        return ok;
    }
    else
    {
        if (!m_protocolConnection.protocol || (m_endpoint.empty() && !m_incomingConnection))
        {
            m_messagesBuffered.push_back(msg);
            return true;
        }

        std::int64_t connectionId = 0;
        if (m_protocolFlagIsMultiConnectionSession)
        {
            Variant& echoData = msg->getEchoData();
            connectionId = echoData.getDataValue<std::int64_t>(FMQ_CONNECTION_ID);
        }

        ok = sendMessageNoLock(msg, connectionId);
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
    IStreamConnectionPtr connection = m_protocolConnection.connection;
    if (m_protocolFlagIsMultiConnectionSession && 
        ((connection == nullptr) || connection->getConnectionData().connectionState != ConnectionState::CONNECTIONSTATE_CONNECTED))
    {
        for (auto it = m_multiConnections.begin(); it != m_multiConnections.end(); ++it)
        {
            if (connection == nullptr)
            {
                connection = it->second.connection;
            }
            if (it->second.connection->getConnectionData().connectionState == ConnectionState::CONNECTIONSTATE_CONNECTED)
            {
                connection = it->second.connection;
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

void ProtocolSession::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_protocolConnection.connection;
    lock.unlock();
    if (connection)
    {
        connection->disconnect();
    }
    else
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
        lock.unlock();
        if (protocolSessionList)
        {
            protocolSessionList->removeProtocolSession(m_sessionId);
        }
    }
}

void ProtocolSession::sendBufferedMessages()
{
    for (size_t i = 0; i < m_messagesBuffered.size(); ++i)
    {
        sendMessageNoLock(m_messagesBuffered[i], 0);
    }
    m_messagesBuffered.clear();
}


bool ProtocolSession::connect(const std::string& endpoint, const ConnectProperties& connectionProperties)
{
    assert(m_protocolConnection.protocol);

    std::unique_lock<std::mutex> lock(m_mutex);
    m_endpoint = endpoint;
    m_connectionProperties = connectionProperties;
    sendBufferedMessages();
    lock.unlock();

    bool res = m_streamConnectionContainer->connect(m_protocolConnection.connection, m_endpoint, m_connectionProperties);
    return res;
}

void ProtocolSession::addSessionToList(bool verified)
{
    if (m_sessionId == 0)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
        lock.unlock();
        if (protocolSessionList)
        {
            m_sessionId = protocolSessionList->addProtocolSession(shared_from_this(), verified);
        }
    }
}


bool ProtocolSession::connectProtocol(const std::string& endpoint, const IProtocolPtr& protocol, const ConnectProperties& connectionProperties, int contentType)
{
    assert(m_protocolConnection.protocol == nullptr);
    assert(m_streamConnectionContainer);

    IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(protocol));

    std::unique_lock<std::mutex> lock(m_mutex);
    assert(m_protocolConnection.connection == nullptr);
    m_endpoint = endpoint;
    m_connectionProperties = connectionProperties;
    m_contentType = contentType;
    m_protocolConnection.protocol = protocol;
    initProtocolValues();
    m_protocolConnection.protocol->setCallback(shared_from_this());
    m_protocolConnection.connection = connection;
    sendBufferedMessages();
    lock.unlock();

    bool res = m_streamConnectionContainer->connect(m_protocolConnection.connection, m_endpoint, m_connectionProperties);
    return res;
}




// IProtocolCallback
void ProtocolSession::connected()
{
    if (m_executor)
    {
        m_executor->addAction([this] () {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->connected(shared_from_this());
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

void ProtocolSession::disconnected()
{
    if (!m_protocolFlagIsMultiConnectionSession || m_triggerConnected)
    {
        if (m_executor)
        {
            m_executor->addAction([this]() {
                auto callback = m_callback.lock();
                if (callback)
                {
                    callback->disconnected(shared_from_this());
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
    std::unique_lock<std::mutex> lock(m_mutex);
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    lock.unlock();
    if (protocolSessionList)
    {
        protocolSessionList->removeProtocolSession(m_sessionId);
    }
}




void ProtocolSession::received(const IMessagePtr& message, std::int64_t connectionId)
{
    if (m_protocolFlagIsMultiConnectionSession && connectionId && connectionId != m_protocolConnection.connection->getConnectionId())
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
        m_executor->addAction([this, message] () {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->received(shared_from_this(), message);
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
        m_executor->addAction([this] () {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->socketConnected(shared_from_this());
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
        m_executor->addAction([this] () {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->socketDisconnected(shared_from_this());
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
    assert(m_protocolConnection.protocol);
    IStreamConnectionPtr connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(m_protocolConnection.protocol));
    std::unique_lock<std::mutex> lock(m_mutex);
    m_protocolConnection.connection = connection;
    lock.unlock();
    m_streamConnectionContainer->connect(connection, m_endpoint, m_connectionProperties);
}



void ProtocolSession::cleanupMultiConnection()
{
    for (auto it = m_multiConnections.begin(); it != m_multiConnections.end(); )
    {
        if (!it->second.connection->getSocket())
        {
            it = m_multiConnections.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


void ProtocolSession::setProtocolConnection(const IProtocolPtr& protocol, const IStreamConnectionPtr& connection)
{
    assert(protocol);
    assert(connection);
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_protocolFlagIsMultiConnectionSession)
    {
        cleanupMultiConnection();
        m_multiConnections[connection->getConnectionId()] = { protocol , connection };
    }
    else
    {
        protocol->moveOldProtocolState(*m_protocolConnection.protocol);
        m_protocolConnection.protocol = protocol;
        m_protocolConnection.connection = connection;
    }
    protocol->setCallback(shared_from_this());
}


bool ProtocolSession::findSessionByName(const std::string& sessionName)
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
                disconnected(); // we are in the poller loop  thread
                assert(m_protocolConnection.protocol);
                session->setProtocolConnection(m_protocolConnection.protocol, m_protocolConnection.connection);
                m_protocolConnection.protocol = nullptr;
                return true;
            }
            return true;
        }
    }
    return false;
}

void ProtocolSession::setSessionName(const std::string& sessionName)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<IProtocolSessionList> list = m_protocolSessionList.lock();
    lock.unlock();

    if (list)
    {
        list->setSessionName(m_sessionId, sessionName);
    }

    lock.lock();
    if (!m_triggerConnected)
    {
        m_triggerConnected = true;
        lock.unlock();
        connected();    // we are in the poller loop  thread
    }
}



void ProtocolSession::pollRequest(std::int64_t connectionId)
{
    if (m_pollWaiting)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::int64_t connectionIdOld = m_pollConnectionId;
        m_pollConnectionId = connectionId;
        const ProtocolConnection* protocolConnection = &m_protocolConnection;
        if (m_protocolFlagIsMultiConnectionSession)
        {
            getProtocolConnectionFromConnectionId(protocolConnection, connectionIdOld);
        }
        IProtocolPtr protocol;
        if (protocolConnection)
        {
            protocol = protocolConnection->protocol;
        }
        lock.unlock();
        if (protocol)
        {
            IMessagePtr reply = protocol->pollReply({});
            sendMessageLocked(reply, connectionIdOld);
        }
    }
    else
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_pollMessages.empty())
        {
            const ProtocolConnection* protocolConnection = &m_protocolConnection;
            if (m_protocolFlagIsMultiConnectionSession)
            {
                getProtocolConnectionFromConnectionId(protocolConnection, connectionId);
            }
            std::deque<IMessagePtr> messages;
            IProtocolPtr protocol;
            if (protocolConnection)
            {
                messages = std::move(m_pollMessages);
                m_pollMessages.clear();
                protocol = protocolConnection->protocol;
            }
            lock.unlock();
            if (protocol)
            {
                IMessagePtr reply = protocol->pollReply(std::move(messages));
                sendMessageLocked(reply, connectionId);
            }
        }
        else
        {
            m_pollConnectionId = connectionId;
            m_pollWaiting = true;
        }
    }
}



bool ProtocolSession::sendMessageLocked(const IMessagePtr& message, std::int64_t connectionId)
{
    // lock also over sendMessage, because the prepareMessageToSend could increment a sequential message counter and the order of the counter shall be like the messages that are sent.
    std::unique_lock<std::mutex> lock(m_mutex);
    return sendMessageNoLock(message, connectionId);
}


bool ProtocolSession::sendMessageNoLock(const IMessagePtr& message, std::int64_t connectionId)
{
    // the mutex must be locked before calling sendMessage, lock also over sendMessage, because the prepareMessageToSend could increment a sequential message counter and the order of the counter shall be like the messages that are sent.
    bool ok = false;
    const ProtocolConnection* protocolConnection = &m_protocolConnection;
    if (m_protocolFlagIsMultiConnectionSession && connectionId != 0)
    {
        getProtocolConnectionFromConnectionId(protocolConnection, connectionId);
    }
    if (protocolConnection)
    {
        IMessagePtr messageProtocol = convertMessageToProtocol(message);
        protocolConnection->protocol->prepareMessageToSend(messageProtocol);
        if (protocolConnection->connection)
        {
            ok = protocolConnection->connection->sendMessage(messageProtocol);
        }
    }
    return ok;
}



}
