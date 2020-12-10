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


#include "streamconnection/StreamConnectionContainer.h"
#include "streamconnection/Socket.h"
#include "streamconnection/AddressHelpers.h"

#if defined(WIN32) || defined(__MINGW32__)
#include "poller/PollerImplSelect.h"
#else
#include "poller/PollerImplEpoll.h"
#endif

#if !defined(WIN32) && !defined(__MINGW32__)
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/unistd.h>
#include <sys/un.h>
#endif

#include <thread>
#include <assert.h>


namespace finalmq {


StreamConnectionContainer::StreamConnectionContainer()
#if defined(WIN32) || defined(__MINGW32__)
    : m_poller(std::make_shared<PollerImplSelect>())
#else
    : m_poller(std::make_shared<PollerImplEpoll>())
#endif
    , m_pollerLoopTerminated(CondVar::CondVarMode::CONDVAR_MANUAL)
{
}

StreamConnectionContainer::~StreamConnectionContainer()
{
    terminatePollerLoop(10);
}



std::unordered_map<SOCKET, StreamConnectionContainer::BindData>::iterator StreamConnectionContainer::findBindByEndpoint(const std::string& endpoint)
{
    for (auto it = m_sd2binds.begin(); it != m_sd2binds.end(); ++it)
    {
        if (it->second.connectionData.endpoint == endpoint)
        {
            return it;
        }
    }
    return m_sd2binds.end();
}


IStreamConnectionPrivatePtr StreamConnectionContainer::findConnectionBySd(SOCKET sd)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPrivatePtr connection;
    auto it = m_sd2Connection.find(sd);
    if (it != m_sd2Connection.end())
    {
        connection = it->second;
    }
    lock.unlock();
    return connection;
}



// IStreamConnectionContainer

void StreamConnectionContainer::init(int cycleTime, int checkReconnectInterval)
{
    // no mutex lock, because it init is called before the thread will be active.
    m_cycleTime = cycleTime;
    m_checkReconnectInterval = checkReconnectInterval;
    m_poller->init();
}


int StreamConnectionContainer::bind(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callbackDefault, const BindProperties& bindProperties)
{
    ConnectionData connectionData = AddressHelpers::endpoint2ConnectionData(endpoint);
    connectionData.ssl = bindProperties.certificateData.ssl;
    std::shared_ptr<Socket> socket = std::make_shared<Socket>();

    bool ok = false;
#ifdef USE_OPENSSL
    if (connectionData.ssl)
    {
        ok = socket->createSslServer(connectionData.af, connectionData.type, connectionData.protocol, bindProperties.certificateData);
    }
    else
#endif
    {
        ok = socket->create(connectionData.af, connectionData.type, connectionData.protocol);
    }

    int err = -1;
    if (ok)
    {
        std::string addr = AddressHelpers::makeSocketAddress(connectionData.hostname, connectionData.port, connectionData.af);
        err = socket->bind((const sockaddr*)addr.c_str(), (int)addr.size());
    }
    if (err == 0)
    {
        // listen for incoming connections
        err = socket->listen(SOMAXCONN);
    }
    if (err == 0)
    {
        SocketDescriptorPtr sd = socket->getSocketDescriptor();
        assert(sd);

        std::unique_lock<std::mutex> locker(m_mutex);

        auto it = findBindByEndpoint(endpoint);
        if (it == m_sd2binds.end())
        {
            assert(m_poller);
            m_sd2binds.emplace(sd->getDescriptor(), BindData{connectionData, socket, callbackDefault});
            m_poller->addSocketEnableRead(sd);
        }
        locker.unlock();
    }
    return err;
}



void StreamConnectionContainer::unbind(const std::string& endpoint)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = findBindByEndpoint(endpoint);
    if (it != m_sd2binds.end())
    {
        SocketPtr socket = it->second.socket;
        assert(socket);
        m_poller->removeSocket(socket->getSocketDescriptor());
        m_sd2binds.erase(it);
    }
    locker.unlock();
}

IStreamConnectionPtr StreamConnectionContainer::createConnection(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const ConnectProperties& connectionProperties)
{
    ConnectionData connectionData = AddressHelpers::endpoint2ConnectionData(endpoint);
    connectionData.incomingConnection = false;
    connectionData.reconnectInterval = connectionProperties.reconnectInterval;
    connectionData.totalReconnectDuration = connectionProperties.totalReconnectDuration;
    connectionData.startTime = std::chrono::system_clock::now();
    connectionData.ssl = connectionProperties.certificateData.ssl;
    connectionData.connectionState = ConnectionState::CONNECTIONSTATE_CREATED;
    std::string addr = AddressHelpers::makeSocketAddress(connectionData.hostname, connectionData.port, connectionData.af);
    connectionData.sockaddr = addr;

    SocketPtr socket = std::make_shared<Socket>();
    int ret = -1;
#ifdef USE_OPENSSL
    if (connectionData.ssl)
    {
        ret = socket->createSslClient(connectionData.af, connectionData.type, connectionData.protocol, connectionProperties.certificateData);
    }
    else
#endif
    {
        ret = socket->create(connectionData.af, connectionData.type, connectionData.protocol);
    }

    IStreamConnectionPrivatePtr connection;

    if (ret >= 0)
    {
        connection = addConnection(socket, connectionData, callback);
        assert(connection);
    }

    return connection;
}



std::vector< IStreamConnectionPtr > StreamConnectionContainer::getAllConnections() const
{
    std::vector< IStreamConnectionPtr > connections;

    std::unique_lock<std::mutex> lock(m_mutex);
    connections.reserve(m_connectionId2Connection.size());
    for (auto it = m_connectionId2Connection.begin(); it != m_connectionId2Connection.end(); ++it)
    {
        connections.push_back(it->second);
    }
    lock.unlock();

    return connections;
}

IStreamConnectionPtr StreamConnectionContainer::getConnection(std::int64_t connectionId) const
{
    IStreamConnectionPtr connection;

    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_connectionId2Connection.find(connectionId);
    if (it != m_connectionId2Connection.end())
    {
        connection = it->second;
    }
    lock.unlock();

    return connection;
}




//////////////

void StreamConnectionContainer::removeConnection(const SocketDescriptorPtr& sd, std::int64_t connectionId)
{
    assert(sd);

    std::unique_lock<std::mutex> lock(m_mutex);
    m_sd2Connection.erase(sd->getDescriptor());
    m_connectionId2Connection.erase(connectionId);
    lock.unlock();
}


void StreamConnectionContainer::disconnectIntern(const IStreamConnectionPrivatePtr& connectionDisconnect, const SocketDescriptorPtr& sd)
{
    bool removeConn = connectionDisconnect->changeStateForDisconnect();
    if (removeConn)
    {
        removeConnection(sd, connectionDisconnect->getConnectionData().connectionId);
        connectionDisconnect->disconnected(connectionDisconnect);
    }
}

//////////////




void StreamConnectionContainer::run()
{
    pollerLoop();
}


bool StreamConnectionContainer::terminatePollerLoop(int timeout)
{
    terminatePollerLoop();
    return m_pollerLoopTerminated.wait(timeout);
}



void StreamConnectionContainer::terminatePollerLoop()
{
    m_terminatePollerLoop = true;
    m_poller->releaseWait();
}



IStreamConnectionPrivatePtr StreamConnectionContainer::addConnection(const SocketPtr& socket, ConnectionData& connectionData, hybrid_ptr<IStreamConnectionCallback> callback)
{
    SocketDescriptorPtr sd = socket->getSocketDescriptor();
    assert(sd);
    connectionData.sd = sd->getDescriptor();
    AddressHelpers::addr2peer((sockaddr*)connectionData.sockaddr.c_str(), connectionData);

    std::unique_lock<std::mutex> lock(m_mutex);
    std::int64_t connectionId = m_nextConnectionId++;
    connectionData.connectionId = connectionId;
    IStreamConnectionPrivatePtr connection = std::make_shared<StreamConnection>(connectionData, socket, m_poller, callback);
    m_connectionId2Connection[connectionId] = connection;
    m_sd2Connection[connectionData.sd] = connection;
    lock.unlock();

    return connection;
}


void StreamConnectionContainer::handleReceive(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, int bytesToRead)
{
#ifdef USE_OPENSSL
    if (socket->isSsl())
    {
        bytesToRead = socket->sslPending();
    }
#endif

    int maxloop = 5;
    while (bytesToRead > 0)
    {
        connection->received(connection, socket, bytesToRead);
        maxloop--;
        if (maxloop > 0)
        {
            bytesToRead = socket->pendingRead();
#ifdef USE_OPENSSL
            if (bytesToRead > 0 && socket->isSsl())
            {
                bytesToRead = socket->sslPending();
            }
#endif
        }
        else
        {
            break;
        }
    }

#ifdef USE_OPENSSL
    if (socket->isReadWhenWritable())
    {
        SocketDescriptorPtr sd = socket->getSocketDescriptor();
        assert(sd);
        m_poller->enableWrite(sd);
    }
#endif
}



void StreamConnectionContainer::handleConnectionEvents(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, const DescriptorInfo& info)
{
    bool disconnected = (info.disconnected || (info.readable && info.bytesToRead == 0));
    if (disconnected)
    {
        SocketDescriptorPtr sd = socket->getSocketDescriptor();
        assert(sd);
        disconnectIntern(connection, sd);
    }
    else
    {
        std::int32_t bytesToRead = info.bytesToRead;
        bool writable = info.writable;
        bool readable = info.readable;
        bool isSsl = socket->isSsl();
#ifdef USE_OPENSSL
        if (isSsl)
        {
            if (connection->getConnectionData().connectionState == ConnectionState::CONNECTIONSTATE_CONNECTING)
            {
                SocketDescriptorPtr sd = socket->getSocketDescriptor();
                assert(sd);
                SslSocket::IoState state = socket->sslConnecting();
                if (state == SslSocket::IoState::WANT_WRITE)
                {
                    m_poller->enableWrite(sd);
                }
                else if (state == SslSocket::IoState::WANT_READ)
                {
                    m_poller->disableWrite(sd);
                }
                else if (state == SslSocket::IoState::SSL_ERROR)
                {
                    disconnectIntern(connection, sd);
                }
                else if (state == SslSocket::IoState::SUCCESS)
                {
                    bool edgeConnection = connection->checkEdgeConnected();
                    if (edgeConnection)
                    {
                        connection->connected(connection);
                    }
                    m_poller->enableWrite(sd);
                }
                return;
            }
        }
#endif

        if (isSsl && writable && socket->isReadWhenWritable())
        {
            handleReceive(connection, socket, bytesToRead);
        }
        else if (isSsl && readable && socket->isWriteWhenReadable())
        {
            connection->sendPendingMessages();
        }
        else
        {
            if (writable)
            {
                bool edgeConnection = connection->checkEdgeConnected();
                if (edgeConnection)
                {
                    connection->connected(connection);
                }
                connection->sendPendingMessages();
                if (socket->isWriteWhenReadable())
                {
                    readable = false;
                }
            }
            if (readable)
            {
                handleReceive(connection, socket, bytesToRead);
            }
        }
    }
}

void StreamConnectionContainer::handleBindEvents(const DescriptorInfo& info)
{
    if (info.readable)
    {
        BindData bindData;
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_sd2binds.find(info.sd);
        if (it != m_sd2binds.end())
        {
            bindData = it->second;
        }
        lock.unlock();

        if (bindData.socket)
        {
            std::string addr;
            addr.resize(400);
            socklen_t addrlen = static_cast<socklen_t>(addr.size());
            SocketPtr socketAccept;
            bindData.socket->accept((sockaddr*)addr.c_str(), &addrlen, socketAccept);
            if (socketAccept)
            {
                ConnectionData connectionData = bindData.connectionData;
                connectionData.incomingConnection = true;
                connectionData.startTime = std::chrono::system_clock::now();
                connectionData.sockaddr = addr;
                connectionData.connectionState = ConnectionState::CONNECTIONSTATE_CONNECTED;

#ifdef USE_OPENSSL
                if (connectionData.ssl)
                {
                    SocketDescriptorPtr sd = socketAccept->getSocketDescriptor();
                    assert(sd);
                    SslAcceptingData sslAcceptingData{socketAccept, connectionData, bindData.callback};
                    m_sslAcceptings.emplace(sd->getDescriptor(), sslAcceptingData);
                    sslAccepting(sslAcceptingData);
                }
                else
#endif
                {
                    IStreamConnectionPrivatePtr connection = addConnection(socketAccept, connectionData, bindData.callback);
                    connection->connected(connection);
                }
                SocketDescriptorPtr sd = socketAccept->getSocketDescriptor();
                assert(sd);
                m_poller->addSocketEnableRead(sd);
            }
        }
    }
    if (info.disconnected)
    {
        bool found = false;
        std::string endpoint;
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_sd2binds.find(info.sd);
        if (it != m_sd2binds.end())
        {
            endpoint = it->second.connectionData.endpoint;
            found = true;
        }
        lock.unlock();
        if (found)
        {
            unbind(endpoint);
        }
    }
}


#ifdef USE_OPENSSL
bool StreamConnectionContainer::sslAccepting(SslAcceptingData& sslAcceptingData)
{
    assert(sslAcceptingData.socket);

    SslSocket::IoState state = sslAcceptingData.socket->sslAccepting();
    SocketDescriptorPtr sd = sslAcceptingData.socket->getSocketDescriptor();
    assert(sd);

    if (state == SslSocket::IoState::WANT_WRITE)
    {
        m_poller->enableWrite(sd);
    }
    else
    {
        m_poller->disableWrite(sd);
    }

    if (state == SslSocket::IoState::SUCCESS)
    {
        IStreamConnectionPrivatePtr connection = addConnection(sslAcceptingData.socket, sslAcceptingData.connectionData, sslAcceptingData.callback);
        connection->connected(connection);
    }
    if (state == SslSocket::IoState::SUCCESS || state == SslSocket::IoState::SSL_ERROR)
    {
        m_sslAcceptings.erase(sd->getDescriptor());
    }
    return (state == SslSocket::IoState::SUCCESS);
}
#endif

void StreamConnectionContainer::doReconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_sd2Connection.begin(); it != m_sd2Connection.end(); ++it)
    {
        const IStreamConnectionPrivatePtr& connection = it->second;
        connection->doReconnect();
    }
    lock.unlock();
}



bool StreamConnectionContainer::isReconnectTimerExpired()
{
    bool expired = false;
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    // reconnect timer
    std::chrono::duration<double> dur = now - m_lastReconnectTime;
    int delta = static_cast<int>(dur.count() * 1000);
    if (delta < 0 || delta >= m_checkReconnectInterval)
    {
        m_lastReconnectTime = now;
        expired = true;
    }

    return expired;
}




void StreamConnectionContainer::pollerLoop()
{
    m_lastReconnectTime = std::chrono::system_clock::now();
    while (!m_terminatePollerLoop)
    {
        const PollerResult& result = m_poller->wait(m_cycleTime);

        if (result.releaseWait)
        {
            std::vector<IStreamConnectionPrivatePtr> connectionsDisconnect;
            connectionsDisconnect.reserve(m_connectionId2Connection.size());
            std::unique_lock<std::mutex> lock(m_mutex);
            for (auto it = m_connectionId2Connection.begin(); it != m_connectionId2Connection.end(); ++it)
            {
                const IStreamConnectionPrivatePtr& connection = it->second;
                assert(connection);
                if (connection->getDisconnectFlag())
                {
                    connectionsDisconnect.push_back(it->second);
                }
            }
            lock.unlock();

            for (size_t i = 0; i < connectionsDisconnect.size(); ++i)
            {
                const IStreamConnectionPrivatePtr& connectionDisconnect = connectionsDisconnect[i];
                SocketPtr socket = connectionDisconnect->getSocketPrivate();
                {
                    SocketDescriptorPtr sd = socket->getSocketDescriptor();
                    disconnectIntern(connectionDisconnect, sd);
                }
            }
        }


        if (result.error)
        {
            // error of the poller
            terminatePollerLoop();
        }
        else if (result.timeout)
        {

        }
        else
        {
            for (size_t i = 0; i < result.descriptorInfos.size(); ++i)
            {
                const DescriptorInfo& info = result.descriptorInfos[i];
                IStreamConnectionPrivatePtr connection = findConnectionBySd(info.sd);
                if (connection)
                {
                    SocketPtr socket = connection->getSocketPrivate();
                    if (socket)
                    {
                        handleConnectionEvents(connection, socket, info);
                    }
                }
                else
                {
#ifdef USE_OPENSSL
                    auto itSslAccepting = m_sslAcceptings.find(info.sd);
                    if (itSslAccepting != m_sslAcceptings.end())
                    {
                        bool success = sslAccepting(itSslAccepting->second);
                        if (success)
                        {
                            IStreamConnectionPrivatePtr connection = findConnectionBySd(info.sd);
                            if (connection)
                            {
                                SocketPtr socket = connection->getSocketPrivate();
                                if (socket)
                                {
                                    handleConnectionEvents(connection, socket, info);
                                }
                            }
                        }
                    }
                    else
#endif
                    {
                        handleBindEvents(info);
                    }
                }
            }
        }

        if (isReconnectTimerExpired())
        {
            doReconnect();
        }
    }
    m_pollerLoopTerminated = true;
}


}   // namespace finalmq
