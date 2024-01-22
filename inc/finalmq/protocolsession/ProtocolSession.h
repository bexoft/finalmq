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

#include <unordered_set>

#include "IProtocol.h"
#include "IProtocolSession.h"
#include "ProtocolSessionList.h"
#include "finalmq/helpers/IExecutor.h"
#include "finalmq/helpers/PollingTimer.h"
#include "finalmq/streamconnection/StreamConnection.h"
#include "finalmq/variant/Variant.h"

namespace finalmq
{
struct IProtocolSessionPrivate : public IProtocolSession
{
    virtual ~IProtocolSessionPrivate() override
    {}
    virtual bool connect() = 0;
    //    virtual void createConnection() = 0;
    virtual void setConnection(const IStreamConnectionPtr& connection, bool verified) = 0;
    virtual void setProtocol(const IProtocolPtr& protocol) = 0;
    virtual void setSessionNameInternal(const std::string& sessionName) = 0;
    virtual void cycleTime() = 0;

    virtual bool connect(const std::string& endpoint, const ConnectProperties& connectionProperties = {}, int contentType = 0) override = 0;
};

typedef std::shared_ptr<IProtocolSessionPrivate> IProtocolSessionPrivatePtr;

struct IStreamConnectionContainer;

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
class ProtocolSession : public IProtocolSessionPrivate, public IProtocolCallback, public std::enable_shared_from_this<ProtocolSession>
{
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
public:
    ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const IProtocolPtr& protocol, const std::shared_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType);
    ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const IProtocolFactoryPtr& protocolFactory, const std::shared_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpointStreamConnection, const ConnectProperties& connectProperties, int contentType);
    ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IExecutorPtr& executorPollerThread, const std::shared_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer);

    virtual ~ProtocolSession();

private:
    // IProtocolSession
    virtual IMessagePtr createMessage() const override;
    virtual void sendMessage(const IMessagePtr& msg, bool isReply = false) override;
    virtual std::int64_t getSessionId() const override;
    virtual ConnectionData getConnectionData() const override;
    virtual int getContentType() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool needsReply() const override;
    virtual bool isMultiConnectionSession() const override;
    virtual bool isSendRequestByPoll() const override;
    virtual bool doesSupportFileTransfer() const override;
    virtual bool isSynchronousRequestReply() const override;
    virtual void disconnect() override;
    //    virtual bool connect(const std::string& endpoint, const ConnectProperties& connectionProperties = {}) override;
    virtual bool connect(const std::string& endpoint, const ConnectProperties& connectionProperties = {}, int contentType = 0) override;
    virtual IExecutorPtr getExecutor() const override;
    virtual void subscribe(const std::vector<std::string>& subscribtions) override;
    virtual const Variant& getFormatData() const override;

    //// IStreamConnectionCallback
    //virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    //virtual void disconnected(const IStreamConnectionPtr& connection) override;
    //virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;

    // IProtocolSessionPrivate
    virtual bool connect() override;
    //    virtual void createConnection() override;
    virtual void setConnection(const IStreamConnectionPtr& connection, bool verified) override;
    virtual void setProtocol(const IProtocolPtr& protocol) override;
    virtual void setSessionNameInternal(const std::string& sessionName) override;
    virtual void cycleTime() override;

    // IProtocolCallback
    virtual void connected() override;
    virtual void disconnected() override;
    virtual void disconnectedVirtualSession(const std::string& virtualSessionId) override;
    virtual void received(const IMessagePtr& message, std::int64_t connectionId) override;
    virtual void socketConnected() override;
    virtual void socketDisconnected() override;
    virtual void reconnect() override;
    virtual bool findSessionByName(const std::string& sessionName, const IProtocolPtr& protocol) override;
    virtual void setSessionName(const std::string& sessionName, const IProtocolPtr& protocol, const IStreamConnectionPtr& connection) override;
    virtual void pollRequest(const IProtocolPtr& protocol, int timeout, int pollCountMax) override;
    virtual void activity() override;
    virtual void setActivityTimeout(int timeout) override;
    virtual void setPollMaxRequests(int maxRequests) override;
    virtual void disconnectedMultiConnection(const IProtocolPtr& protocol) override;

    IMessagePtr convertMessageToProtocol(const IMessagePtr& msg);
    void initProtocolValues();
    void sendBufferedMessages();
    void addSessionToList(bool verified);
    void getProtocolFromConnectionId(IProtocolPtr& protocol, std::int64_t connectionId);
    void sendMessage(const IMessagePtr& message, const IProtocolPtr& protocol);
    void cleanupMultiConnection();
    void pollRelease();

    bool hasPendingRequests() const;
    IProtocolPtr allocateRequestConnection();
    IProtocolPtr createRequestConnection();
    void sendNextRequests();

    const hybrid_ptr<IProtocolSessionCallback> m_callback;
    const IExecutorPtr m_executor;
    const IExecutorPtr m_executorPollerThread;
    IProtocolPtr m_protocol{};
    std::atomic<std::int64_t> m_connectionId{0};
    std::unordered_map<std::int64_t, IProtocolPtr> m_multiProtocols{};
    std::unordered_set<std::int64_t> m_unallocatedConnections{};

    const std::weak_ptr<IProtocolSessionList> m_protocolSessionList;
    const int64_t m_sessionId = 0;
    const int64_t m_instanceId = 0;

    std::atomic<int> m_contentType{false};
    std::uint32_t m_protocolId = false;
    std::atomic<bool> m_protocolFlagMessagesResendable{false};
    std::atomic<bool> m_protocolFlagSupportMetainfo{false};
    std::atomic<bool> m_protocolFlagNeedsReply{false};
    std::atomic<bool> m_protocolFlagIsMultiConnectionSession{false};
    std::atomic<bool> m_protocolFlagIsSendRequestByPoll{false};
    std::atomic<bool> m_protocolFlagSupportFileTransfer{false};
    std::atomic<bool> m_protocolFlagSynchronousRequestReply{false};

    IProtocolFactoryPtr m_protocolFactory{};
    IProtocol::FuncCreateMessage m_messageFactory{};
    std::atomic_bool m_protocolSet{false};
    bool m_triggeredConnected = false;
    bool m_triggeredDisconnected = false;

    const std::shared_ptr<IStreamConnectionContainer> m_streamConnectionContainer{};
    std::string m_endpointStreamConnection{};

    const BindProperties m_bindProperties{};
    ConnectProperties m_connectionProperties{};
    Variant m_protocolData{};
    IProtocolSessionDataPtr m_protocolSessionData{};
    Variant m_formatData{};
    int m_maxSynchReqRepConnections = -1;

    std::deque<IMessagePtr> m_messagesBuffered{};
    std::unordered_map<std::int64_t, Variant> m_runningRequests{};

    std::deque<IMessagePtr> m_pollMessages{};
    int m_pollMaxRequests = 10000;
    //    IMessagePtr                                     m_pollReply;
    IProtocolPtr m_pollProtocol = nullptr;
    PollingTimer m_pollTimer{};
    int m_pollCountMax = 0;
    int m_pollCounter = 0;

    std::atomic<int> m_activityTimeout{-1};
    PollingTimer m_activityTimer{};

    bool m_verified = false;
    std::string m_sessionName{};

    mutable std::mutex m_mutex{};
};

} // namespace finalmq
