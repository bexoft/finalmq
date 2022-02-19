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


#include "finalmq/streamconnection/StreamConnection.h"
#include "finalmq/streamconnection/AddressHelpers.h"
#include <thread>


namespace finalmq {


StreamConnection::StreamConnection(const ConnectionData& connectionData, std::shared_ptr<Socket> socket, const IPollerPtr& poller, hybrid_ptr<IStreamConnectionCallback> callback)
    : m_connectionData(connectionData)
    , m_socketPrivate(socket)
    , m_socket(socket)
    , m_poller(poller)
    , m_callback(callback)
{

    m_lastReconnectTime = std::chrono::steady_clock::now();
}


StreamConnection::~StreamConnection()
{
}


// IStreamConnection
bool StreamConnection::sendMessage(const IMessagePtr& msg)
{
    assert(msg);
    bool ret = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_socketPrivate)
    {
        ret = true;
        ssize_t size = msg->getTotalSendBufferSize();
        if (size > 0)
        {
            const auto& payloads = msg->getAllSendBuffers();
            if (!m_pendingMessages.empty() ||
                m_connectionData.connectionState != ConnectionState::CONNECTIONSTATE_CONNECTED)
            {
                m_pendingMessages.push_back({msg, payloads.begin(), 0});
            }
            else
            {
#if 0
                m_pendingMessages.push_back({msg, payloads.begin(), 0});
                m_poller->enableWrite(m_socketPrivate->getSocketDescriptor());
#else
                bool ex = false;
                int i = 0;
                for (auto it = payloads.begin(); it != payloads.end() && !ex; ++i)
                {
                    const BufferRef& payload = *it;
                    ++it;

#if __QNX__
                    int flags = 0;
#else
                    bool last = (it == payloads.end());
                    int flags = last ? 0 : MSG_MORE;    // win32: MSG_PARTIAL
#endif

#if !defined WIN32
                    flags |= MSG_NOSIGNAL;              // no sigpipe
#endif
                    int err = m_socketPrivate->send(payload.first, static_cast<int>(payload.second), flags);
                    if (err != payload.second)
                    {
                        if (err < 0)
                        {
                            err = 0;
                        }
                        assert(err < payload.second);
                        --it;
                        m_pendingMessages.push_back({msg, it, err});
                        m_poller->enableWrite(m_socketPrivate->getSocketDescriptor());
                        ex = true;
                    }
                }
#endif
            }
        }
    }
    lock.unlock();
    return ret;
}


ConnectionData StreamConnection::getConnectionData() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connectionData;
}


ConnectionState StreamConnection::getConnectionState() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connectionData.connectionState;
}

std::int64_t StreamConnection::getConnectionId() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connectionData.connectionId;
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
    m_poller->releaseWait(RELEASE_DISCONNECT);
}


bool StreamConnection::connect()
{
    bool connecting = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if ((m_connectionData.connectionState == ConnectionState::CONNECTIONSTATE_CREATED || m_connectionData.connectionState == ConnectionState::CONNECTIONSTATE_CONNECTING_FAILED) &&
        m_socketPrivate)
    {
        int ret = m_socketPrivate->connect((const sockaddr*)m_connectionData.sockaddr.c_str(), (int)m_connectionData.sockaddr.size());
        if (ret == 0)
        {
            connecting = true;
            m_connectionData.connectionState = ConnectionState::CONNECTIONSTATE_CONNECTING;
            SocketDescriptorPtr sd = m_socketPrivate->getSocketDescriptor();
            assert(sd);
            m_poller->addSocketEnableRead(sd);
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
        if (m_connectionData.connectionState == ConnectionState::CONNECTIONSTATE_CONNECTED)
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
#if __QNX__
                    int flags = 0;
#else
                    bool last = ((it == payloads.end()) && (m_pendingMessages.size() == 1));
                    int flags = last ? 0 : MSG_MORE;    // win32: MSG_PARTIAL
#endif
#if !defined WIN32
                    flags |= MSG_NOSIGNAL;              // no sigpipe
#endif
                    ssize_t size = payload.second - messageSendState.offset;
                    assert((payload.second == 0 && size == 0) || (size > 0));
                    int err = m_socketPrivate->send(payload.first + messageSendState.offset, static_cast<int>(size), flags);
                    if (err == size)
                    {
                        messageSendState.it = it;
                        messageSendState.offset = 0;
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
    if (m_connectionData.connectionState == ConnectionState::CONNECTIONSTATE_CONNECTING)
    {
        m_connectionData.connectionState = ConnectionState::CONNECTIONSTATE_CONNECTED;
        edgeConnected = true;
    }
    return edgeConnected;
}


bool StreamConnection::doReconnect()
{
    bool reconnecting = false;
    if (!m_connectionData.incomingConnection &&
        m_connectionData.connectionState == ConnectionState::CONNECTIONSTATE_CONNECTING_FAILED &&
        m_connectionData.reconnectInterval >= 0)
    {
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        std::chrono::duration<double> dur = now - m_lastReconnectTime;
        int delta = static_cast<int>(dur.count() * 1000);
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
    if (!m_disconnectFlag && (m_connectionData.connectionState == ConnectionState::CONNECTIONSTATE_CONNECTING))
    {
        std::chrono::duration<double> dur = std::chrono::steady_clock::now() - m_connectionData.startTime;
        int delta = static_cast<int>(dur.count() * 1000);
        if (m_connectionData.totalReconnectDuration >= 0 && (delta < 0 || delta >= m_connectionData.totalReconnectDuration))
        {
            reconnectExpired = true;
        }
        else
        {
            assert(m_socketPrivate);
            m_connectionData.connectionState = ConnectionState::CONNECTIONSTATE_CONNECTING_FAILED;
            m_poller->removeSocket(m_socketPrivate->getSocketDescriptor());
        }
    }

    if (m_disconnectFlag || (m_connectionData.connectionState == ConnectionState::CONNECTIONSTATE_CONNECTED) || reconnectExpired)
    {
        removeConnection = true;

        assert(m_socketPrivate);
        m_connectionData.connectionState = ConnectionState::CONNECTIONSTATE_DISCONNECTED;
        m_poller->removeSocket(m_socketPrivate->getSocketDescriptor());
        std::unique_lock<std::mutex> lock(m_mutex);
        m_socketPrivate = nullptr;
        m_socket = nullptr;
    }
    return removeConnection;
}


bool StreamConnection::getDisconnectFlag() const
{
    return m_disconnectFlag;
}


void StreamConnection::updateConnectionData(const ConnectionData& connectionData)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connectionData = connectionData;
    lock.unlock();
}









void StreamConnection::connected(const IStreamConnectionPtr& connection)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        hybrid_ptr<IStreamConnectionCallback> callbackOverride = callback->connected(connection);
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

bool StreamConnection::received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead)
{
    bool ok = false;
    auto callback = m_callback.lock();
    if (callback && !m_disconnectFlag)
    {
        ok = callback->received(connection, socket, bytesToRead);
    }
    return ok;
}

}   // namespace finalmq
