

#include "streamconnection/StreamConnection.h"
#include <thread>



StreamConnection::StreamConnection(const ConnectionData& connectionData, std::shared_ptr<Socket> socket, const IPollerPtr& poller, bex::hybrid_ptr<IStreamConnectionCallback> callback)
    : m_connectionData(connectionData)
    , m_socketPrivate(socket)
    , m_socket(socket)
    , m_poller(poller)
    , m_callback(callback)
{

    m_lastReconnectTime = std::chrono::system_clock::now();
}

// IStreamConnection
bool StreamConnection::sendMessage(const IMessagePtr& msg)
{
    assert(msg);
    int ret = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_socketPrivate)
    {
        ret = true;
        int size = msg->getTotalSendBufferSize();
        if (size > 0)
        {
            const auto& payloads = msg->getAllSendBuffers();
            if (!m_pendingMessages.empty() ||
                m_connectionData.connectionState != CONNECTIONSTATE_CONNECTED)
            {
                m_pendingMessages.push_back({msg, payloads.begin(), 0});
            }
            else
            {
                bool ex = false;
                int i = 0;
                for (auto it = payloads.begin(); it != payloads.end() && !ex; ++i)
                {
                    const BufferRef& payload = *it;
                    ++it;
                    bool last = (it == payloads.end());
                    int flags = last ? 0 : MSG_MORE;    // win32: MSG_PARTIAL
                    int err = m_socketPrivate->send(payload.first, payload.second, flags);
                    if (err != payload.second)
                    {
                        assert(err < payload.second);
                        --it;
                        m_pendingMessages.push_back({msg, it, err});
                        m_poller->enableWrite(m_socketPrivate->getSocketDescriptor());
                        ex = true;
                    }
                }
            }
        }
    }
    lock.unlock();
    return ret;
}


const ConnectionData& StreamConnection::getConnectionData() const
{
    return m_connectionData;
}


SocketPtr StreamConnection::getSocketPrivate()
{
    // do not mutex lock here, because the removeSocket and getSocketPrivate will be called from same thread.
    return m_socketPrivate;
}


SocketPtr StreamConnection::getSocket()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_socket;
}


void StreamConnection::disconnect()
{
    m_disconnectFlag = true;
    m_poller->releaseWait();
}




bool StreamConnection::connect()
{
    bool connecting = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if ((m_connectionData.connectionState == CONNECTIONSTATE_CREATED || m_connectionData.connectionState == CONNECTIONSTATE_CONNECTING_FAILED) &&
        m_socketPrivate)
    {
        int ret = m_socketPrivate->connect((const sockaddr*)m_connectionData.sockaddr.c_str(), (int)m_connectionData.sockaddr.size());
        if (ret == 0)
        {
            connecting = true;
            m_connectionData.connectionState = CONNECTIONSTATE_CONNECTING;
            SocketDescriptorPtr sd = m_socketPrivate->getSocketDescriptor();
            assert(sd);
            m_poller->addSocket(sd);
            m_poller->enableWrite(sd);
        }
    }
    return connecting;
}




bool StreamConnection::sendPendingMessages()
{
    bool pending = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_socketPrivate)
    {
        if (m_connectionData.connectionState == CONNECTIONSTATE_CONNECTED)
        {
            while (!m_pendingMessages.empty() && !pending)
            {
                MessageSendState& messageSendState = m_pendingMessages.front();
                IMessagePtr& msg = messageSendState.msg;
                assert(msg);
                const auto& payloads = msg->getAllSendBuffers();
                for (auto it = messageSendState.it ; it != payloads.end() && !pending; )
                {
                    const BufferRef& payload = *it;
                    ++it;
                    bool last = ((it == payloads.end()) && (m_pendingMessages.size() == 1));
                    int flags = last ? 0 : MSG_MORE;    // win32: MSG_PARTIAL
                    int size = payload.second - messageSendState.offset;
                    assert(size > 0);
                    int err = m_socketPrivate->send(payload.first + messageSendState.offset, size, flags);
                    if (err == size)
                    {
                        ++messageSendState.it;
                    }
                    else if (err > 0)
                    {
                        messageSendState.offset += err;
                        assert(messageSendState.offset < payload.second);
                        pending = true;
                    }
                    else
                    {
                        pending = true;
                    }
                }
                if (!pending)
                {
                    m_pendingMessages.pop_front();
                }
            }
            if (!pending)
            {
                m_poller->disableWrite(m_socketPrivate->getSocketDescriptor());
            }
        }
    }
    lock.unlock();

    return pending;
}





bool StreamConnection::checkEdgeConnected()
{
    bool edgeConnected = false;
    if (m_connectionData.connectionState == CONNECTIONSTATE_CONNECTING)
    {
        m_connectionData.connectionState = CONNECTIONSTATE_CONNECTED;
        edgeConnected = true;
    }
    return edgeConnected;
}


bool StreamConnection::doReconnect()
{
    bool reconnecting = false;
    if (!m_connectionData.incomingConnection &&
        m_connectionData.connectionState == CONNECTIONSTATE_CONNECTING_FAILED &&
        m_connectionData.reconnectInterval >= 0)
    {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::chrono::duration<double> dur = now - m_lastReconnectTime;
        int delta = dur.count() * 1000;
        if (delta < 0 || delta >= m_connectionData.reconnectInterval)
        {
            m_lastReconnectTime = now;
            reconnecting = connect();
        }
    }
    return reconnecting;
}


bool StreamConnection::changeStateForDisconnect()
{
    bool removeConnection = false;
    bool reconnectExpired = false;
    if (!m_disconnectFlag && (m_connectionData.connectionState == CONNECTIONSTATE_CONNECTING))
    {
        std::chrono::duration<double> dur = std::chrono::system_clock::now() - m_connectionData.startTime;
        int delta = dur.count() * 1000;
        if (m_connectionData.totalReconnectDuration >= 0 && (delta < 0 || delta >= m_connectionData.totalReconnectDuration))
        {
            reconnectExpired = true;
        }
        else
        {
            assert(m_socketPrivate);
            m_connectionData.connectionState = CONNECTIONSTATE_CONNECTING_FAILED;
            m_poller->removeSocket(m_socketPrivate->getSocketDescriptor());
        }
    }

    if (m_disconnectFlag || (m_connectionData.connectionState == CONNECTIONSTATE_CONNECTED) || reconnectExpired)
    {
        removeConnection = true;

        assert(m_socketPrivate);
        m_connectionData.connectionState = CONNECTIONSTATE_DISCONNECTED;
        m_poller->removeSocket(m_socketPrivate->getSocketDescriptor());
        m_socketPrivate = nullptr;
        std::unique_lock<std::mutex> lock(m_mutex);
        m_socket = nullptr;
    }
    return removeConnection;
}


bool StreamConnection::getDisconnectFlag() const
{
    return m_disconnectFlag;
}



void StreamConnection::connected(const IStreamConnectionPtr& connection)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        bex::hybrid_ptr<IStreamConnectionCallback> callbackOverride = callback->connected(connection);
        if (callbackOverride.lock())
        {
            m_callback = callbackOverride;
            // call connected also for the overriden callback
            callback = m_callback.lock();
            if (callback)
            {
                callback->connected(connection);
            }
        }
    }
}

void StreamConnection::disconnected(const IStreamConnectionPtr& connection)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected(connection);
    }
}

void StreamConnection::received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->received(connection, socket, bytesToRead);
    }
}

