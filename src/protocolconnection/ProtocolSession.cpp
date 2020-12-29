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

#include "protocolconnection/ProtocolSession.h"
#include "streamconnection/StreamConnectionContainer.h"
#include "protocolconnection/ProtocolMessage.h"


namespace finalmq {


ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType)
    : m_callback(callback)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_bindProperties(bindProperties)
{
}

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpoint, const ConnectProperties& connectProperties, int contentType)
    : m_callback(callback)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_streamConnectionContainer(streamConnectionContainer)
    , m_endpoint(endpoint)
    , m_connectionProperties(connectProperties)
{
}

ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, int contentType)
    : m_callback(callback)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_contentType(contentType)
    , m_streamConnectionContainer(streamConnectionContainer)
{
}


ProtocolSession::ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer)
    : m_callback(callback)
    , m_protocolSessionList(protocolSessionList)
    , m_streamConnectionContainer(streamConnectionContainer)
{

}



ProtocolSession::~ProtocolSession()
{

}


// IProtocolSessionPrivate

void ProtocolSession::connect()
{
    assert(m_streamConnectionContainer);
    IStreamConnectionPtr connection;
    connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(shared_from_this()));
    setConnection(connection);
    m_streamConnectionContainer->connect(connection, m_endpoint, m_connectionProperties);
}


void ProtocolSession::createConnection()
{
    assert(m_streamConnectionContainer);
    IStreamConnectionPtr connection;
    connection = m_streamConnectionContainer->createConnection(std::weak_ptr<IStreamConnectionCallback>(shared_from_this()));
    setConnection(connection);
}


int64_t ProtocolSession::setConnection(const IStreamConnectionPtr& connection)
{
    if (m_protocol)
    {
        m_protocol->setCallback(shared_from_this());
    }
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    if (protocolSessionList)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_connection = connection;
        lock.unlock();
        if (m_sessionId == 0)
        {
            m_sessionId = protocolSessionList->addProtocolSession(shared_from_this());
        }
    }
    return m_sessionId;
}




// IProtocolSession
IMessagePtr ProtocolSession::createMessage() const
{
    if (m_protocol)
    {
        return m_protocol->createMessage();
    }
    else
    {
        return std::make_shared<ProtocolMessage>(0);
    }
}

IMessagePtr ProtocolSession::convertMessageToProtocol(const IMessagePtr& msg)
{
    IMessagePtr message = msg;
    if (message->getProtocolId() != m_protocol->getProtocolId() ||
        (!m_protocol->areMessagesResendable() && message->wasSent()) ||
        (message->getTotalSendPayloadSize() == 0))
    {
        message = nullptr;
        if (m_protocol->areMessagesResendable())
        {
            message = msg->getMessage(m_protocol->getProtocolId());
        }
        if (message == nullptr)
        {
            message = m_protocol->createMessage();
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
            if (m_protocol->areMessagesResendable())
            {
                msg->addMessage(message);
            }
        }
    }

    return message;
}

bool ProtocolSession::sendMessage(const IMessagePtr& msg)
{
    if (!m_protocol)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_protocol)
        {
            m_messages.push_back(msg);
            return true;
        }
        lock.unlock();
    }

    IMessagePtr message = convertMessageToProtocol(msg);

    assert(message->getTotalSendPayloadSize() > 0);

    // lock also over sendMessage, because the prepareMessageToSend could increment a sequential message counter and the order of the counter shall be like the messages that are sent.
    std::unique_lock<std::mutex> lock(m_mutex);
    m_protocol->prepareMessageToSend(message);
    IStreamConnectionPtr connection = m_connection;
    if (connection)
    {
        return connection->sendMessage(message);
    }
    return false;
}

std::int64_t ProtocolSession::getSessionId() const
{
    return m_sessionId;
}

ConnectionData ProtocolSession::getConnectionData() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    lock.unlock();
    if (connection)
    {
        return connection->getConnectionData();
    }
    static ConnectionData defaultConnectionData;
    return defaultConnectionData;
}


SocketPtr ProtocolSession::getSocket()
{
    assert(m_connection);
    return m_connection->getSocket();
}


int ProtocolSession::getContentType() const
{
    return m_contentType;
}


void ProtocolSession::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    lock.unlock();
    if (connection)
    {
        return connection->disconnect();
    }
}

bool ProtocolSession::connect(const std::string& endpoint, const ConnectProperties& connectionProperties)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    m_endpoint = endpoint;
    m_connectionProperties = connectionProperties;
    lock.unlock();

    assert(connection);

    bool res = m_streamConnectionContainer->connect(connection, m_endpoint, m_connectionProperties);
    return res;
}


bool ProtocolSession::connectProtocol(const std::string& endpoint, const IProtocolPtr& protocol, const ConnectProperties& connectionProperties, int contentType)
{
    assert(m_streamConnectionContainer);
    std::unique_lock<std::mutex> lock(m_mutex);
    assert(m_connection);
    m_endpoint = endpoint;
    m_connectionProperties = connectionProperties;
    m_contentType = contentType;
    m_protocol = protocol;
    m_protocol->setCallback(shared_from_this());
    for (size_t i = 0; i < m_messages.size(); ++i)
    {
        IMessagePtr message = convertMessageToProtocol(m_messages[i]);
        assert(message->getTotalSendPayloadSize() > 0);
        m_protocol->prepareMessageToSend(message);
        m_connection->sendMessage(message);
    }
    m_messages.clear();
    lock.unlock();

    bool res = m_streamConnectionContainer->connect(m_connection, m_endpoint, m_connectionProperties);
    return res;
}


// IStreamConnectionCallback
hybrid_ptr<IStreamConnectionCallback> ProtocolSession::connected(const IStreamConnectionPtr& /*connection*/)
{
    if (m_protocol)
    {
        m_protocol->socketConnected();
    }
    else
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        IProtocolPtr protocol = m_protocol;
        lock.unlock();
        if (protocol)
        {
            protocol->socketConnected();
        }
    }
    return nullptr;
}

void ProtocolSession::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    if (m_protocol)
    {
        m_protocol->socketDisconnected();
    }
    else
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        IProtocolPtr protocol = m_protocol;
        lock.unlock();
        if (protocol)
        {
            protocol->socketDisconnected();
        }
        else
        {
            disconnected();
        }
    }
}

void ProtocolSession::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead)
{
    if (m_protocol)
    {
        m_protocol->receive(socket, bytesToRead);
    }
    else
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        IProtocolPtr protocol = m_protocol;
        lock.unlock();
        if (protocol)
        {
            protocol->receive(socket, bytesToRead);
        }
    }
}



// IProtocolCallback
void ProtocolSession::connected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected(shared_from_this());
    }
}

void ProtocolSession::disconnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected(shared_from_this());
    }
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    if (protocolSessionList)
    {
        protocolSessionList->removeProtocolSession(m_sessionId);
    }
}

void ProtocolSession::received(const IMessagePtr& message)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->received(shared_from_this(), message);
    }
}

void ProtocolSession::socketConnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->socketConnected(shared_from_this());
    }
}

void ProtocolSession::socketDisconnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->socketDisconnected(shared_from_this());
    }
}

void ProtocolSession::reconnect()
{
    connect();
}

}
