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

#include "finalmq/streamconnection/IMessage.h"

namespace finalmq {

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
    virtual std::uint32_t getProtocolId() const = 0;
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

}   // namespace finalmq
