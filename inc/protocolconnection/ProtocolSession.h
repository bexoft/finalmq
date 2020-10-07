#pragma once

#include "streamconnection/StreamConnection.h"
#include "IProtocol.h"
#include "ProtocolSessionList.h"
#include "IProtocolSession.h"




struct IProtocolSessionPrivate : public IProtocolSession
                               , public IStreamConnectionCallback
{
    virtual void connect() = 0;
    virtual int64_t setConnection(const IStreamConnectionPtr& connection) = 0;
};

typedef std::shared_ptr<IProtocolSessionPrivate> IProtocolSessionPrivatePtr;


struct IStreamConnectionContainer;


class ProtocolSession : public IProtocolSessionPrivate
                      , public IProtocolCallback
                      , public std::enable_shared_from_this<ProtocolSession>
{
public:
    ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList);
    ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpoint, int reconnectInterval, int totalReconnectDuration);

#ifdef USE_OPENSSL
    ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const CertificateData& certificateData);
    ProtocolSession(bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const std::weak_ptr<IProtocolSessionList>& protocolSessionList, const std::shared_ptr<IStreamConnectionContainer>& streamConnectionContainer, const std::string& endpoint, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration);
#endif

    virtual ~ProtocolSession();

private:
    // IProtocolSession
    virtual IMessagePtr createMessage() const override;
    virtual bool sendMessage(const IMessagePtr& msg) override;
    virtual std::int64_t getSessionId() const;
    virtual const ConnectionData& getConnectionData() const override;
    virtual SocketPtr getSocket() override;
    virtual void disconnect() override;

    // IStreamConnectionCallback
    virtual bex::hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;

    // IProtocolSessionPrivate
    virtual void connect() override;
    virtual int64_t setConnection(const IStreamConnectionPtr& connection) override;

    // IProtocolCallback
    virtual void connected() override;
    virtual void disconnected() override;
    virtual void received(const IMessagePtr& message) override;
    virtual void socketConnected() override;
    virtual void socketDisconnected() override;
    virtual void reconnect() override;

    IStreamConnectionPtr                            m_connection;
    bex::hybrid_ptr<IProtocolSessionCallback>       m_callback;
    IProtocolPtr                                    m_protocol;
    int64_t                                         m_sessionId = 0;
    std::weak_ptr<IProtocolSessionList>             m_protocolSessionList;

    std::shared_ptr<IStreamConnectionContainer>     m_streamConnectionContainer;
    const std::string                               m_endpoint;
    const int                                       m_reconnectInterval = 5000;
    const int                                       m_totalReconnectDuration = -1;

#ifdef USE_OPENSSL
    bool                                            m_ssl = false;
    CertificateData                                 m_certificateData;
#endif

    mutable std::mutex                              m_mutex;
};

