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

#include "finalmq/protocolconnection/ProtocolSessionContainer.h"


namespace finalmq {


struct IConnectionHub
{
    virtual ~IConnectionHub() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, const BindProperties& bindProperties = {}) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const ConnectProperties& connectProperties = {}) = 0;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const = 0;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const = 0;
    virtual void run() = 0;
    virtual void waitForTerminationOfPollerLoop() = 0;

    virtual void startMessageForwarding() = 0;
    virtual void stopForwardingFromSession(std::int64_t sessionId) = 0;
    virtual void stopForwardingToSession(std::int64_t sessionId) = 0;
};




class SYMBOLEXP ConnectionHub : public IConnectionHub
                              , private IProtocolSessionCallback
{
public:
    ConnectionHub();

private:
    // IConnectionHub
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, const BindProperties& bindProperties = {}) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const ConnectProperties& connectProperties = {}) override;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const override;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const override;
    virtual void run() override;
    virtual void waitForTerminationOfPollerLoop() override;

    virtual void startMessageForwarding() override;
    virtual void stopForwardingFromSession(std::int64_t sessionId) override;
    virtual void stopForwardingToSession(std::int64_t sessionId) override;

    // IProtocolSessionCallback
    virtual void connected(const IProtocolSessionPtr& session) override;
    virtual void disconnected(const IProtocolSessionPtr& session) override;
    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message) override;
    virtual void socketConnected(const IProtocolSessionPtr& session) override;
    virtual void socketDisconnected(const IProtocolSessionPtr& session) override;

    std::unique_ptr<IProtocolSessionContainer>  m_protocolSessionContainer;
    std::vector<std::int64_t>                   m_sessionIdsStopForwardingFromSession;
    std::vector<std::int64_t>                   m_sessionIdsStopForwardingToSession;
    bool                                        m_startMessageForwarding = false;
    std::vector<std::pair<IProtocolSessionPtr, IMessagePtr>>    m_messagesForForwarding;

    std::recursive_mutex                        m_mutex;

};

}   // namespace finalmq
