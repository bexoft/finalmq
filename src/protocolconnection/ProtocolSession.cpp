
#include "protocolconnection/ProtocolSession.h"
#include "streamconnection/StreamConnectionContainer.h"





ProtocolSession::ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList)
    : m_callback(callback)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
{
}

ProtocolSession::ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpoint, int reconnectInterval, int totalReconnectDuration)
    : m_callback(callback)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_streamConnectionContainer(streamConnectionContainer)
    , m_endpoint(endpoint)
    , m_reconnectInterval(reconnectInterval)
    , m_totalReconnectDuration(totalReconnectDuration)
{
}


#ifdef USE_OPENSSL
ProtocolSession::ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const CertificateData& certificateData)
    : m_callback(callback)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_ssl(true)
    , m_certificateData(certificateData)
{

}

ProtocolSession::ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpoint, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration)
    : m_callback(callback)
    , m_protocol(protocol)
    , m_protocolSessionList(protocolSessionList)
    , m_streamConnectionContainer(streamConnectionContainer)
    , m_endpoint(endpoint)
    , m_reconnectInterval(reconnectInterval)
    , m_totalReconnectDuration(totalReconnectDuration)
    , m_ssl(true)
    , m_certificateData(certificateData)
{

}

#endif


ProtocolSession::~ProtocolSession()
{

}


// IProtocolSessionPrivate

void ProtocolSession::connect()
{
    assert(m_streamConnectionContainer);
    IStreamConnectionPtr connection;
#ifdef USE_OPENSSL
    if (m_ssl)
    {
        connection = m_streamConnectionContainer->createConnectionSsl(m_endpoint, std::weak_ptr<IStreamConnectionCallback>(shared_from_this()), m_certificateData, m_reconnectInterval, m_totalReconnectDuration);
    }
    else
#endif
    {
        connection = m_streamConnectionContainer->createConnection(m_endpoint, std::weak_ptr<IStreamConnectionCallback>(shared_from_this()), m_reconnectInterval, m_totalReconnectDuration);
    }
    setConnection(connection);
    connection->connect();
}


int64_t ProtocolSession::setConnection(const IStreamConnectionPtr& connection)
{
    IProtocolSessionListPtr protocolSessionList = m_protocolSessionList.lock();
    if (protocolSessionList)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_connection = connection;
        lock.unlock();
        if (m_sessionId == 0)
        {
            m_protocol->setCallback(shared_from_this());
            m_sessionId = protocolSessionList->addProtocolSession(shared_from_this());
        }
    }
    return m_sessionId;
}




// IProtocolSession
IMessagePtr ProtocolSession::createMessage() const
{
    return m_protocol->createMessage();
}

bool ProtocolSession::sendMessage(const IMessagePtr& msg)
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
                int sizePayload = msg->getTotalSendPayloadSize();
                char* payload = message->addSendPayload(sizePayload);
                const std::list<BufferRef>& payloads = msg->getAllSendPayloads();
                int offset = 0;
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

    assert(message->getTotalSendPayloadSize() > 0);

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

const ConnectionData& ProtocolSession::getConnectionData() const
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


// IStreamConnectionCallback
bex::hybrid_ptr<IStreamConnectionCallback> ProtocolSession::connected(const IStreamConnectionPtr& connection)
{
    m_protocol->socketConnected();
    return nullptr;
}

void ProtocolSession::disconnected(const IStreamConnectionPtr& connection)
{
    m_protocol->socketDisconnected();
}

void ProtocolSession::received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead)
{
    m_protocol->receive(socket, bytesToRead);
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

