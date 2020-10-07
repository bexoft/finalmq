#pragma once

#include "streamconnection/StreamConnection.h"


struct IProtocolSession
{
    virtual ~IProtocolSession() {}
    virtual IMessagePtr createMessage() const = 0;
    virtual bool sendMessage(const IMessagePtr& msg) = 0;
    virtual std::int64_t getSessionId() const = 0;
    virtual const ConnectionData& getConnectionData() const = 0;
    virtual SocketPtr getSocket() = 0;
    virtual void disconnect() = 0;
};

struct IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;



struct IProtocolSessionCallback
{
    virtual ~IProtocolSessionCallback() {}
    virtual void connected(const IProtocolSessionPtr& session) = 0;
    virtual void disconnected(const IProtocolSessionPtr& session) = 0;
    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message) = 0;
    virtual void socketConnected(const IProtocolSessionPtr& session) = 0;
    virtual void socketDisconnected(const IProtocolSessionPtr& session) = 0;
};

