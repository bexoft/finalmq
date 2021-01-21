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

#pragma once


#include "ConnectionData.h"
#include "Socket.h"
#include "finalmq/poller/Poller.h"
#include "finalmq/helpers/hybrid_ptr.h"
#include "finalmq/streamconnection/IMessage.h"

#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include <assert.h>


namespace finalmq {


struct BindProperties
{
    CertificateData certificateData;
};

struct ConnectProperties
{
    CertificateData certificateData;
    int reconnectInterval = 5000;
    int totalReconnectDuration = -1;
};



struct IStreamConnection;
typedef std::shared_ptr<IStreamConnection> IStreamConnectionPtr;

struct IStreamConnectionCallback
{
    virtual ~IStreamConnectionCallback() {}
    virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) = 0;
    virtual void disconnected(const IStreamConnectionPtr& connection) = 0;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) = 0;
};




struct IStreamConnection
{
    virtual ~IStreamConnection() {}
    virtual bool sendMessage(const IMessagePtr& msg) = 0;
    virtual ConnectionData getConnectionData() const = 0;
    virtual ConnectionState getConnectionState() const = 0;
    virtual std::int64_t getConnectionId() const = 0;
    virtual SocketPtr getSocket() = 0;
    virtual void disconnect() = 0;
};


struct IStreamConnectionPrivate : public IStreamConnection
{
    virtual bool connect() = 0;
    virtual SocketPtr getSocketPrivate() = 0;
    virtual bool sendPendingMessages() = 0;
    virtual bool checkEdgeConnected() = 0;
    virtual bool doReconnect() = 0;
    virtual bool changeStateForDisconnect() = 0;
    virtual bool getDisconnectFlag() const = 0;
    virtual void updateConnectionData(const ConnectionData& connectionData) = 0;

    virtual void connected(const IStreamConnectionPtr& connection) = 0;
    virtual void disconnected(const IStreamConnectionPtr& connection) = 0;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) = 0;
};



typedef std::shared_ptr<IStreamConnectionPrivate> IStreamConnectionPrivatePtr;


class SYMBOLEXP StreamConnection : public IStreamConnectionPrivate
{
public:
    StreamConnection(const ConnectionData& connectionData, std::shared_ptr<Socket> socket, const IPollerPtr& poller, hybrid_ptr<IStreamConnectionCallback> callback);

private:
    // IStreamConnection
    virtual bool sendMessage(const IMessagePtr& msg) override;
    virtual ConnectionData getConnectionData() const override;
    virtual ConnectionState getConnectionState() const override;
    virtual std::int64_t getConnectionId() const override;
    virtual SocketPtr getSocket() override;
    virtual void disconnect() override;

    // IStreamConnectionPrivate
    virtual bool connect() override;
    virtual SocketPtr getSocketPrivate() override;
    virtual bool sendPendingMessages() override;
    virtual bool checkEdgeConnected() override;
    virtual bool doReconnect() override;
    virtual bool changeStateForDisconnect() override;
    virtual bool getDisconnectFlag() const override;
    virtual void updateConnectionData(const ConnectionData& connectionData) override;

    virtual void connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;

    struct MessageSendState
    {
        IMessagePtr msg;
        std::list<BufferRef>::const_iterator it;
        int offset = 0;
    };

    ConnectionData              m_connectionData;
    SocketPtr                   m_socketPrivate;
    SocketPtr                   m_socket;
    IPollerPtr                  m_poller;
    std::list<MessageSendState> m_pendingMessages;
    bool                        m_disconnectFlag = false;
    hybrid_ptr<IStreamConnectionCallback> m_callback;

    std::chrono::time_point<std::chrono::system_clock> m_lastReconnectTime;

    mutable std::mutex          m_mutex;
};

}   // namespace finalmq
