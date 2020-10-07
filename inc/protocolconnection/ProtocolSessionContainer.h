#pragma once

#include "ProtocolSession.h"
#include "IProtocol.h"
#include "ProtocolSessionList.h"

#include <unordered_map>



struct IProtocolSessionContainer
{
    virtual ~IProtocolSessionContainer() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const = 0;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const = 0;
    virtual void threadEntry() = 0;
    virtual bool waitForTerminationOfPollerLoop(int timeout) = 0;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const CertificateData& certificateData) = 0;
    virtual IProtocolSessionPtr connectSsl(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
#endif
};

typedef std::shared_ptr<IProtocolSessionContainer> IProtocolSessionContainerPtr;



struct IStreamConnectionContainer;


class ProtocolBind : public IStreamConnectionCallback
{
public:
    ProtocolBind(bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const std::weak_ptr<IProtocolSessionList>& protocolSessionList);

private:
    // IStreamConnectionCallback
    virtual bex::hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;

    bex::hybrid_ptr<IProtocolSessionCallback>    m_callback;
    IProtocolFactoryPtr                          m_protocolFactory;
    std::weak_ptr<IProtocolSessionList>          m_protocolSessionList;
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
    virtual int bind(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const override;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const override;
    virtual void threadEntry() override;
    virtual bool waitForTerminationOfPollerLoop(int timeout) override;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const CertificateData& certificateData) override;
    virtual IProtocolSessionPtr connectSsl(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
#endif

private:
    IProtocolSessionListPtr                                     m_protocolSessionList;
    std::unordered_map<std::string,  ProtocolBindPtr>           m_endpoint2Bind;
    std::shared_ptr<IStreamConnectionContainer>                 m_streamConnectionContainer;
    mutable std::mutex                                          m_mutex;
};
