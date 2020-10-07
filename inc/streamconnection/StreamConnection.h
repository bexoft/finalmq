#pragma once


#include "ConnectionData.h"
#include "Socket.h"
#include "poller/Poller.h"
#include "helpers/hybrid_ptr.h"
#include "streamconnection/IMessage.h"

#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include <assert.h>




struct IStreamConnection;
typedef std::shared_ptr<IStreamConnection> IStreamConnectionPtr;

struct IStreamConnectionCallback
{
    virtual ~IStreamConnectionCallback() {}
    virtual bex::hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) = 0;
    virtual void disconnected(const IStreamConnectionPtr& connection) = 0;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) = 0;
};




struct IStreamConnection
{
    virtual ~IStreamConnection() {}
    virtual bool connect() = 0;
    virtual bool sendMessage(const IMessagePtr& msg) = 0;
    virtual const ConnectionData& getConnectionData() const = 0;
    virtual SocketPtr getSocket() = 0;
    virtual void disconnect() = 0;
};


struct IStreamConnectionPrivate : public IStreamConnection
{
    virtual SocketPtr getSocketPrivate() = 0;
    virtual bool sendPendingMessages() = 0;
    virtual bool checkEdgeConnected() = 0;
    virtual bool doReconnect() = 0;
    virtual bool changeStateForDisconnect() = 0;
    virtual bool getDisconnectFlag() const = 0;

    virtual void connected(const IStreamConnectionPtr& connection) = 0;
    virtual void disconnected(const IStreamConnectionPtr& connection) = 0;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) = 0;
};



typedef std::shared_ptr<IStreamConnectionPrivate> IStreamConnectionPrivatePtr;


class StreamConnection : public IStreamConnectionPrivate
{
public:
    StreamConnection(const ConnectionData& connectionData, std::shared_ptr<Socket> socket, const IPollerPtr& poller, bex::hybrid_ptr<IStreamConnectionCallback> callback);

private:
    // IStreamConnection
    virtual bool connect() override;
    virtual bool sendMessage(const IMessagePtr& msg) override;
    virtual const ConnectionData& getConnectionData() const override;
    virtual SocketPtr getSocket() override;
    virtual void disconnect() override;

    // IStreamConnectionPrivate
    virtual SocketPtr getSocketPrivate() override;
    virtual bool sendPendingMessages() override;
    virtual bool checkEdgeConnected() override;
    virtual bool doReconnect() override;
    virtual bool changeStateForDisconnect() override;
    virtual bool getDisconnectFlag() const override;

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
    bex::hybrid_ptr<IStreamConnectionCallback> m_callback;

    std::chrono::time_point<std::chrono::system_clock> m_lastReconnectTime;

    mutable std::mutex          m_mutex;
};


