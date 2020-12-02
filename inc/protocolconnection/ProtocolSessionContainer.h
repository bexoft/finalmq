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

#include "ProtocolSession.h"
#include "IProtocol.h"
#include "ProtocolSessionList.h"

#include <unordered_map>

namespace finalmq {

struct IProtocolSessionContainer
{
    virtual ~IProtocolSessionContainer() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const BindProperties& bindProperties = {}, int contentType = 0) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const ConnectProperties& connectProperties = {}, int contentType = 0) = 0;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const = 0;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const = 0;
    virtual void threadEntry() = 0;
    virtual bool terminatePollerLoop(int timeout) = 0;
};

typedef std::shared_ptr<IProtocolSessionContainer> IProtocolSessionContainerPtr;



struct IStreamConnectionContainer;


class ProtocolBind : public IStreamConnectionCallback
{
public:
    ProtocolBind(hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties = {}, int contentType = 0);

private:
    // IStreamConnectionCallback
    virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;

    hybrid_ptr<IProtocolSessionCallback>    m_callback;
    IProtocolFactoryPtr                     m_protocolFactory;
    std::weak_ptr<IProtocolSessionList>     m_protocolSessionList;
    BindProperties                          m_bindProperties;
    int                                     m_contentType = 0;
};

typedef std::shared_ptr<ProtocolBind> ProtocolBindPtr;





class ProtocolSessionContainer : public IProtocolSessionContainer
{
public:
    ProtocolSessionContainer();
    virtual ~ProtocolSessionContainer();

private:
    // IProtocolSessionContainer
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const BindProperties& bindProperties = {}, int contentType = 0) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const ConnectProperties& connectProperties = {}, int contentType = 0) override;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const override;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const override;
    virtual void threadEntry() override;
    virtual bool terminatePollerLoop(int timeout) override;

private:
    IProtocolSessionListPtr                                     m_protocolSessionList;
    std::unordered_map<std::string,  ProtocolBindPtr>           m_endpoint2Bind;
    std::shared_ptr<IStreamConnectionContainer>                 m_streamConnectionContainer;
    mutable std::mutex                                          m_mutex;
};

}   // namespace finalmq
