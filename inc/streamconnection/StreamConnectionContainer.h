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


namespace finalmq {


struct IStreamConnectionContainer
{
    virtual ~IStreamConnectionContainer() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const BindProperties& bindProperties = {}) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IStreamConnectionPtr createConnection(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const ConnectProperties& connectionProperties = {}) = 0;
    virtual IStreamConnectionPtr createConnection(hybrid_ptr<IStreamConnectionCallback> callback) = 0;
    virtual bool setEndpoint(const IStreamConnectionPtr& streamConnection, const std::string& endpoint, const ConnectProperties& connectionProperties = {}) = 0;
    virtual std::vector< IStreamConnectionPtr > getAllConnections() const = 0;
    virtual IStreamConnectionPtr getConnection(std::int64_t connectionId) const = 0;
    virtual void run() = 0;
    virtual bool terminatePollerLoop(int timeout) = 0;
};



class SYMBOLEXP StreamConnectionContainer : public IStreamConnectionContainer
{
public:
    StreamConnectionContainer();
    ~StreamConnectionContainer();

private:
    // IStreamConnectionContainer
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const BindProperties& bindProperties = {}) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IStreamConnectionPtr createConnection(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const ConnectProperties& connectionProperties = {}) override;
    virtual IStreamConnectionPtr createConnection(hybrid_ptr<IStreamConnectionCallback> callback) override;
    virtual bool setEndpoint(const IStreamConnectionPtr& streamConnection, const std::string& endpoint, const ConnectProperties& connectionProperties = {}) override;
    virtual std::vector< IStreamConnectionPtr > getAllConnections() const override;
    virtual IStreamConnectionPtr getConnection(std::int64_t connectionId) const override;
    virtual void run() override;
    virtual bool terminatePollerLoop(int timeout) override;

    void terminatePollerLoop();

    void pollerLoop();

    struct BindData
    {
        ConnectionData                          connectionData;
        SocketPtr                               socket;
        hybrid_ptr<IStreamConnectionCallback>   callback;
    };

    std::unordered_map<SOCKET, BindData>::iterator findBindByEndpoint(const std::string& endpoint);
    IStreamConnectionPrivatePtr findConnectionBySd(SOCKET sd);
    void removeConnection(const SocketDescriptorPtr& sd, std::int64_t connectionId);
    void disconnectIntern(const IStreamConnectionPrivatePtr& connectionDisconnect, const SocketDescriptorPtr& sd);
    IStreamConnectionPrivatePtr addConnection(const SocketPtr& socket, ConnectionData& connectionData, hybrid_ptr<IStreamConnectionCallback> callback);
    void handleConnectionEvents(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, const DescriptorInfo& info);
    void handleBindEvents(const DescriptorInfo& info);
    void handleReceive(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, int bytesToRead);
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
        hybrid_ptr<IStreamConnectionCallback> callback;
    };
    bool sslAccepting(SslAcceptingData& sslAcceptingData);
    std::unordered_map<SOCKET, SslAcceptingData>                    m_sslAcceptings;
#endif
};

}   // namespace finalmq
