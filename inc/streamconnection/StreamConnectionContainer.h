#pragma once


#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>

#include "helpers/hybrid_ptr.h"
#include "ConnectionData.h"
#include "poller/Poller.h"
#include "Socket.h"
#include "StreamConnection.h"
#include "helpers/CondVar.h"




struct IStreamConnectionContainer
{
    virtual ~IStreamConnectionContainer() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IStreamConnectionPtr createConnection(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
    virtual std::vector< IStreamConnectionPtr > getAllConnections() const = 0;
    virtual IStreamConnectionPtr getConnection(std::int64_t connectionId) const = 0;
    virtual void threadEntry() = 0;
    virtual bool terminatePollerLoop(int timeout) = 0;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback, const CertificateData& certificateData) = 0;
    virtual IStreamConnectionPtr createConnectionSsl(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
#endif
};



class StreamConnectionContainer : public IStreamConnectionContainer
{
public:
    StreamConnectionContainer();
    ~StreamConnectionContainer();

private:
    // IStreamConnectionContainer
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IStreamConnectionPtr createConnection(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
    virtual std::vector< IStreamConnectionPtr > getAllConnections() const override;
    virtual IStreamConnectionPtr getConnection(std::int64_t connectionId) const override;
    virtual void threadEntry() override;
    virtual bool terminatePollerLoop(int timeout) override;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback, const CertificateData& certificateData) override;
    virtual IStreamConnectionPtr createConnectionSsl(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
#endif

    void terminatePollerLoop();
    int bindIntern(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callbackDefault, bool ssl, const CertificateData& certificateData);
    IStreamConnectionPtr createConnectionIntern(const std::string& endpoint, bex::hybrid_ptr<IStreamConnectionCallback> callback, bool ssl, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration);

    void pollerLoop();

    struct BindData
    {
        ConnectionData                              connectionData;
        SocketPtr                                   socket;
        bex::hybrid_ptr<IStreamConnectionCallback>  callback;
    };

    std::unordered_map<SOCKET, BindData>::iterator findBindByEndpoint(const std::string& endpoint);
    IStreamConnectionPrivatePtr findConnectionBySd(SOCKET sd);
    void removeConnection(const SocketDescriptorPtr& sd, std::int64_t connectionId);
    void disconnectIntern(const IStreamConnectionPrivatePtr& connectionDisconnect, const SocketDescriptorPtr& sd);
    IStreamConnectionPrivatePtr addConnection(const SocketPtr& socket, ConnectionData& connectionData, bex::hybrid_ptr<IStreamConnectionCallback> callback);
    void handleConnectionEvents(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, const DescriptorInfo& info);
    void handleBindEvents(const DescriptorInfo& info);
    bool isReconnectTimerExpired();
    void doReconnect();

    std::shared_ptr<IPoller>                                        m_poller;
    std::unordered_map<SOCKET, BindData>                            m_sd2binds;
    std::unordered_map<std::int64_t, IStreamConnectionPrivatePtr>   m_connectionId2Connection;
    std::unordered_map<SOCKET, IStreamConnectionPrivatePtr>         m_sd2Connection;
    std::int64_t                                                    m_nextConnectionId = 1;
    bool                                                            m_terminatePollerLoop = false;
    CondVar                                                         m_pollerLoopTerminated;
    int                                                             m_cycleTime = 100;
    double                                                          m_checkReconnectInterval = 1000;
    mutable std::mutex                                              m_mutex;

    std::chrono::time_point<std::chrono::system_clock>              m_lastReconnectTime;

#ifdef USE_OPENSSL
    struct SslAcceptingData
    {
        SocketPtr socket;
        ConnectionData connectionData;
        bex::hybrid_ptr<IStreamConnectionCallback> callback;
    };
    bool sslAccepting(SslAcceptingData& sslAcceptingData);
    std::unordered_map<SOCKET, SslAcceptingData>                    m_sslAcceptings;
#endif
};

