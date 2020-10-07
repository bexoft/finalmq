#pragma once

#include "protocolconnection/ProtocolSessionContainer.h"


struct IConnectionHub
{
    virtual ~IConnectionHub() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, IProtocolFactoryPtr protocolFactory) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const IProtocolPtr& protocol, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const = 0;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const = 0;
    virtual void threadEntry() = 0;
    virtual bool waitForTerminationOfPollerLoop(int timeout) = 0;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, IProtocolFactoryPtr protocolFactory, const CertificateData& certificateData) = 0;
    virtual IProtocolSessionPtr connectSsl(const std::string& endpoint, const IProtocolPtr& protocol, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
#endif

    virtual void startMessageForwarding() = 0;
    virtual void stopForwardingFromSession(std::int64_t sessionId) = 0;
    virtual void stopForwardingToSession(std::int64_t sessionId) = 0;
};




class ConnectionHub : public IConnectionHub
                    , private IProtocolSessionCallback
{
public:
    ConnectionHub();

private:
    // IConnectionHub
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, IProtocolFactoryPtr protocolFactory) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const IProtocolPtr& protocol, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const override;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const override;
    virtual void threadEntry() override;
    virtual bool waitForTerminationOfPollerLoop(int timeout) override;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, IProtocolFactoryPtr protocolFactory, const CertificateData& certificateData) override;
    virtual IProtocolSessionPtr connectSsl(const std::string& endpoint, const IProtocolPtr& protocol, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
#endif

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
