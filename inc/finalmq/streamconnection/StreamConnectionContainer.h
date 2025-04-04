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
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "ConnectionData.h"
#include "Socket.h"
#include "StreamConnection.h"
#include "finalmq/helpers/CondVar.h"
#include "finalmq/helpers/IExecutor.h"
#include "finalmq/helpers/hybrid_ptr.h"
#include "finalmq/poller/Poller.h"

namespace finalmq
{
typedef std::function<void()> FuncPollerLoopTimer;

struct IStreamConnectionContainer
{
    virtual ~IStreamConnectionContainer()
    {}

    virtual void init(int cycleTime = 100, FuncPollerLoopTimer funcTimer = {}, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const BindProperties& bindProperties = {}) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IStreamConnectionPtr connect(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const ConnectProperties& connectionProperties = {}) = 0;
    virtual IStreamConnectionPtr createConnection(hybrid_ptr<IStreamConnectionCallback> callback) = 0;
    virtual bool connect(const std::string& endpoint, const IStreamConnectionPtr& streamConnection, const ConnectProperties& connectionProperties = {}) = 0;
    virtual std::vector<IStreamConnectionPtr> getAllConnections() const = 0;
    virtual IStreamConnectionPtr getConnection(std::int64_t connectionId) const = 0;
    virtual void run() = 0;
    virtual void terminatePollerLoop() = 0;
    virtual IExecutorPtr getPollerThreadExecutor() const = 0;
};

class SYMBOLEXP StreamConnectionContainer : public IStreamConnectionContainer
{
public:
    StreamConnectionContainer();
    ~StreamConnectionContainer();

    static void getConnectPropertiesFromEndpoint(const std::string& endpoint, ConnectProperties& connectProperties);
    static void getBindPropertiesFromEndpoint(const std::string& endpoint, BindProperties& bindProperties);


private:
    // IStreamConnectionContainer
    virtual void init(int cycleTime = 100, FuncPollerLoopTimer funcTimer = {}, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const BindProperties& bindProperties = {}) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IStreamConnectionPtr connect(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const ConnectProperties& connectionProperties = {}) override;
    virtual IStreamConnectionPtr createConnection(hybrid_ptr<IStreamConnectionCallback> callback) override;
    virtual bool connect(const std::string& endpoint, const IStreamConnectionPtr& streamConnection, const ConnectProperties& connectionProperties = {}) override;
    virtual std::vector<IStreamConnectionPtr> getAllConnections() const override;
    virtual IStreamConnectionPtr getConnection(std::int64_t connectionId) const override;
    virtual void run() override;
    virtual void terminatePollerLoop() override;
    virtual IExecutorPtr getPollerThreadExecutor() const override;

    void pollerLoop();

    struct BindData
    {
        ConnectionData connectionData{};
        SocketPtr socket{};
        hybrid_ptr<IStreamConnectionCallback> callback{};
    };

    std::unordered_map<SOCKET, BindData>::iterator findBindByEndpoint(const std::string& endpoint);
    IStreamConnectionPrivatePtr findConnectionBySdOnlyForPollerLoop(SOCKET sd);
    IStreamConnectionPrivatePtr findConnectionById(std::int64_t connectionId);
    bool createSocket(const IStreamConnectionPtr& streamConnection, ConnectionData& connectionData, const ConnectProperties& connectionProperties);
    void removeConnection(const SocketDescriptorPtr& sd, std::int64_t connectionId);
    void disconnectIntern(const IStreamConnectionPrivatePtr& connectionDisconnect, const SocketDescriptorPtr& sd);
    IStreamConnectionPrivatePtr addConnection(const SocketPtr& socket, ConnectionData& connectionData, hybrid_ptr<IStreamConnectionCallback> callback);
    void handleConnectionEvents(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, const DescriptorInfo& info);
    void handleBindEvents(const DescriptorInfo& info);
    void handleReceive(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, int bytesToRead);
    static bool isTimerExpired(std::chrono::time_point<std::chrono::steady_clock>& lastTime, int interval);
    void doReconnect();

    const std::shared_ptr<IPoller> m_poller{};
    std::unordered_map<SOCKET, BindData> m_sd2binds{};
    std::unordered_map<std::int64_t, IStreamConnectionPrivatePtr> m_connectionId2Connection{};
    std::unordered_map<SOCKET, IStreamConnectionPrivatePtr> m_sd2Connection{};
    std::unordered_map<SOCKET, IStreamConnectionPrivatePtr> m_sd2ConnectionPollerLoop{};
    std::atomic_flag m_connectionsStable{};
    static std::atomic_int64_t m_nextConnectionId;
    std::atomic_bool m_terminatePollerLoop{false};
    int m_cycleTime = 100;
    int m_checkReconnectInterval = 1000;
    FuncPollerLoopTimer m_funcTimer{};
    const IExecutorPtr m_executorPollerThread;
    std::unique_ptr<IExecutorWorker> m_executorWorker{};
    std::thread m_threadTimer{};
    mutable std::mutex m_mutex{};

    std::chrono::time_point<std::chrono::steady_clock> m_lastReconnectTime{};

#ifdef USE_OPENSSL
    struct SslAcceptingData
    {
        SocketPtr socket;
        ConnectionData connectionData;
        hybrid_ptr<IStreamConnectionCallback> callback;
    };
    bool sslAccepting(SslAcceptingData& sslAcceptingData);
    std::unordered_map<SOCKET, SslAcceptingData> m_sslAcceptings{};
#endif
};

} // namespace finalmq
