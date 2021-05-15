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
#include "finalmq/helpers/IExecutor.h"
#include "IProtocol.h"
#include "ProtocolSessionList.h"
#include "IProtocolSession.h"


namespace finalmq {




struct IProtocolSessionPrivate : public IProtocolSession
{
    virtual bool connect() = 0;
    virtual void createConnection() = 0;
    virtual int64_t setConnection(const IStreamConnectionPtr& connection, bool verified) = 0;
    virtual void setProtocolConnection(const IProtocolPtr& protocol, const IStreamConnectionPtr& connection) = 0;
};

typedef std::shared_ptr<IProtocolSessionPrivate> IProtocolSessionPrivatePtr;


struct IStreamConnectionContainer;


class ProtocolSession : public IProtocolSessionPrivate
                      , public IProtocolCallback
                      , public std::enable_shared_from_this<ProtocolSession>
{
public:
    ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const BindProperties& bindProperties, int contentType);
    ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpoint, const ConnectProperties& connectProperties, int contentType);
    ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, int contentType);
    ProtocolSession(hybrid_ptr<IProtocolSessionCallback> callback, const IExecutorPtr& executor, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer);

    virtual ~ProtocolSession();

private:
    // IProtocolSession
    virtual IMessagePtr createMessage() const override;
    virtual bool sendMessage(const IMessagePtr& msg) override;
    virtual std::int64_t getSessionId() const;
    virtual ConnectionData getConnectionData() const override;
//    virtual SocketPtr getSocket() override;
    virtual int getContentType() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool needsReply() const override;
    virtual bool isMultiConnectionSession() const override;
    virtual bool isSendRequestByPoll() const override;
    virtual void disconnect() override;
    virtual bool connect(const std::string& endpoint, const ConnectProperties& connectionProperties = {}) override;
    virtual bool connectProtocol(const std::string& endpoint, const IProtocolPtr& protocol, const ConnectProperties& connectionProperties = {}, int contentType = 0) override;

    //// IStreamConnectionCallback
    //virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    //virtual void disconnected(const IStreamConnectionPtr& connection) override;
    //virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;

    // IProtocolSessionPrivate
    virtual bool connect() override;
    virtual void createConnection() override;
    virtual int64_t setConnection(const IStreamConnectionPtr& connection, bool verified) override;
    virtual void setProtocolConnection(const IProtocolPtr& protocol, const IStreamConnectionPtr& connection) override;

    // IProtocolCallback
    virtual void connected() override;
    virtual void disconnected() override;
    virtual void received(const IMessagePtr& message, std::int64_t connectionId) override;
    virtual void socketConnected() override;
    virtual void socketDisconnected() override;
    virtual void reconnect() override;
    virtual bool findSessionByName(const std::string& sessionName) override;
    virtual void setSessionName(const std::string& sessionName) override;

    IMessagePtr convertMessageToProtocol(const IMessagePtr& msg);
    void initProtocolValues();
    void cleanupMultiConnection();
    void sendBufferedMessages();
    void addSessionToList(bool verified);

    struct ProtocolConnection
    {
        IProtocolPtr                                protocol;
        IStreamConnectionPtr                        connection;
    };

    hybrid_ptr<IProtocolSessionCallback>                    m_callback;
    IExecutorPtr                                            m_executor;
    ProtocolConnection                                      m_protocolConnection;
    std::unordered_map<std::int64_t, ProtocolConnection>    m_multiConnections;

    int64_t                                         m_sessionId = 0;
    std::weak_ptr<IProtocolSessionList>             m_protocolSessionList;
    int                                             m_contentType = 0;
    std::uint32_t                                   m_protocolId = 0;
    bool                                            m_protocolFlagMessagesResendable = false;
    bool                                            m_protocolFlagSupportMetainfo = false;
    bool                                            m_protocolFlagNeedsReply = false;
    bool                                            m_protocolFlagIsMultiConnectionSession = false;
    bool                                            m_protocolFlagIsSendRequestByPoll = false;
    IProtocol::FuncCreateMessage                    m_messageFactory;
    std::atomic_bool                                m_protocolSet;
    bool                                            m_triggerConnected = false;
    bool                                            m_incomingConnection = false;

    std::shared_ptr<IStreamConnectionContainer>     m_streamConnectionContainer;
    std::string                                     m_endpoint;

    BindProperties                                  m_bindProperties;
    ConnectProperties                               m_connectionProperties;

    std::deque<IMessagePtr>                         m_messages;

    mutable std::mutex                              m_mutex;
};

}   // namespace finalmq
