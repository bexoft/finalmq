#pragma once

#include "streamconnection/IMessage.h"

class Socket;
typedef std::shared_ptr<Socket> SocketPtr;


struct IProtocolCallback
{
    virtual ~IProtocolCallback() {}
    virtual void connected() = 0;
    virtual void disconnected() = 0;
    virtual void received(const IMessagePtr& message) = 0;
    virtual void socketConnected() = 0;
    virtual void socketDisconnected() = 0;
    virtual void reconnect() = 0;
};


struct IProtocol
{
    virtual ~IProtocol() {}
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) = 0;
    virtual int getProtocolId() const = 0;
    virtual bool areMessagesResendable() const = 0;
    virtual IMessagePtr createMessage() const = 0;
    virtual void receive(const SocketPtr& socket, int bytesToRead) = 0;
    virtual void prepareMessageToSend(IMessagePtr message) = 0;
    virtual void socketConnected() = 0;
    virtual void socketDisconnected() = 0;
};


typedef std::shared_ptr<IProtocol>  IProtocolPtr;


struct IProtocolFactory
{
    virtual ~IProtocolFactory() {}
    virtual IProtocolPtr createProtocol() = 0;
};

typedef std::shared_ptr<IProtocolFactory>  IProtocolFactoryPtr;
