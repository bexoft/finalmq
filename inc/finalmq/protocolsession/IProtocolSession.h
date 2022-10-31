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

#include "finalmq/streamconnection/StreamConnection.h"
#include "IProtocol.h"

namespace finalmq {

struct IProtocolSession
{
    virtual ~IProtocolSession() {}
    virtual IMessagePtr createMessage() const = 0;
    virtual void sendMessage(const IMessagePtr& msg, bool isReply = false) = 0;
    virtual std::int64_t getSessionId() const = 0;
    virtual ConnectionData getConnectionData() const = 0;
    virtual int getContentType() const = 0;
    virtual bool doesSupportMetainfo() const = 0;
    virtual bool needsReply() const = 0;
    virtual bool isMultiConnectionSession() const = 0;
    virtual bool isSendRequestByPoll() const = 0;
    virtual bool doesSupportFileTransfer() const = 0;
    virtual void disconnect() = 0;
//    virtual bool connect(const std::string& endpoint, const ConnectProperties& connectionProperties = {}) = 0;
    virtual bool connect(const std::string& endpoint, const ConnectProperties& connectionProperties = {}, int contentType = 0) = 0;
    virtual IExecutorPtr getExecutor() const = 0;
    virtual void subscribe(const std::vector<std::string>& subscribtions) = 0;
};

//struct IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;



struct IProtocolSessionCallback
{
    virtual ~IProtocolSessionCallback() {}
    virtual void connected(const IProtocolSessionPtr& session) = 0;
    virtual void disconnected(const IProtocolSessionPtr& session) = 0;
    virtual void disconnectedVirtualSession(const IProtocolSessionPtr& session, const std::string& virtualSessionId) = 0;
    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message) = 0;
    virtual void socketConnected(const IProtocolSessionPtr& session) = 0;
    virtual void socketDisconnected(const IProtocolSessionPtr& session) = 0;
};

}   // namespace finalmq
