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
#include "finalmq/streamconnection/StreamConnection.h"

#include <functional>


namespace finalmq {

class Socket;
typedef std::shared_ptr<Socket> SocketPtr;

struct IProtocol;
typedef std::shared_ptr<IProtocol>  IProtocolPtr;


struct IProtocolCallback
{
    virtual ~IProtocolCallback() {}
    virtual void connected() = 0;
    virtual void disconnected() = 0;
    virtual void disconnectedVirtualSession(const std::string& virtualSessionId) = 0;
    virtual void received(const IMessagePtr& message, std::int64_t connectionId = 0) = 0;
    virtual void socketConnected() = 0;
    virtual void socketDisconnected() = 0;
    virtual void reconnect() = 0;
    virtual bool findSessionByName(const std::string& sessionName, const IProtocolPtr& protocol) = 0;
    virtual void setSessionName(const std::string& sessionName, const IProtocolPtr& protocol, const IStreamConnectionPtr& connection) = 0;
    virtual void pollRequest(const IProtocolPtr& protocol, int timeout, int pollCountMax) = 0;
    virtual void activity() = 0;
    virtual void setActivityTimeout(int timeout) = 0;
    virtual void setPollMaxRequests(int maxRequests) = 0;
    virtual void disconnectedMultiConnection(const IProtocolPtr& protocol) = 0;
};

struct IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


struct IProtocol : public IStreamConnectionCallback
{
    typedef std::function<IMessagePtr()> FuncCreateMessage;
    virtual ~IProtocol() {}
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) = 0;
    virtual void setConnection(const IStreamConnectionPtr& connection) = 0;
    virtual IStreamConnectionPtr getConnection() const = 0;
    virtual void disconnect() = 0;
    virtual std::uint32_t getProtocolId() const = 0;
    virtual bool areMessagesResendable() const = 0;
    virtual bool doesSupportMetainfo() const = 0;
    virtual bool doesSupportSession() const = 0;
    virtual bool needsReply() const = 0;
    virtual bool isMultiConnectionSession() const = 0;
    virtual bool isSendRequestByPoll() const = 0;
    virtual bool doesSupportFileTransfer() const = 0;
    virtual FuncCreateMessage getMessageFactory() const = 0;
    virtual void sendMessage(IMessagePtr message) = 0;
    virtual void moveOldProtocolState(IProtocol& protocolOld) = 0;
    virtual IMessagePtr pollReply(std::deque<IMessagePtr>&& messages) = 0;
    virtual void subscribe(const std::vector<std::string>& subscribtions) = 0;
    virtual void cycleTime() = 0;
};


struct IProtocolFactory
{
    virtual ~IProtocolFactory() {}
    virtual IProtocolPtr createProtocol(const Variant& data) = 0;
};

typedef std::shared_ptr<IProtocolFactory>  IProtocolFactoryPtr;

}   // namespace finalmq
