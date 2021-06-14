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

#include "finalmq/remoteentity/RemoteEntity.h"
#include "finalmq/remoteentity/FileTransferReply.h"


namespace finalmq {

class SYMBOLEXP ConnectionEvent
{
public:
    enum Enum : std::int32_t {
        CONNECTIONEVENT_CONNECTED = 0,
        CONNECTIONEVENT_DISCONNECTED = 1,
        CONNECTIONEVENT_SOCKET_CONNECTED = 2,
        CONNECTIONEVENT_SOCKET_DISCONNECTED = 3,
    };

    ConnectionEvent();
    ConnectionEvent(Enum en);
    operator const Enum&() const;
    operator Enum&();
    const ConnectionEvent& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = CONNECTIONEVENT_CONNECTED;
    static const EnumInfo _enumInfo;
};


typedef std::function<void(const IProtocolSessionPtr& session, ConnectionEvent connectionEvent)> FuncConnectionEvent;


struct IRemoteEntityContainer
{
    virtual ~IRemoteEntityContainer() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000, FuncPollerLoopTimer funcTimer = {}, const IExecutorPtr& executor = nullptr, bool storeRawDataInReceiveStruct = false) = 0;
    virtual int bind(const std::string& endpoint, const BindProperties& bindProperties = {}) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const ConnectProperties& connectProperties = {}) = 0;
    virtual void run() = 0;
    virtual void terminatePollerLoop() = 0;
    virtual IExecutorPtr getExecutor() const = 0;

    virtual EntityId registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, const std::string& name = "") = 0;
    virtual void addPureDataPaths(std::vector<std::string>& paths) = 0;
    virtual void unregisterEntity(EntityId entityId) = 0;
    virtual void registerConnectionEvent(FuncConnectionEvent funcConnectionEvent) = 0;

    virtual std::vector<EntityId> getAllEntities() const = 0;
    virtual hybrid_ptr<IRemoteEntity> getEntity(EntityId) const = 0;
};

typedef std::shared_ptr<IRemoteEntityContainer> IRemoteEntityContainerPtr;





class SYMBOLEXP RemoteEntityContainer : public IRemoteEntityContainer
                                      , public IProtocolSessionCallback
{
public:
    RemoteEntityContainer();
    virtual ~RemoteEntityContainer();

    // IRemoteEntityContainer
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000, FuncPollerLoopTimer funcTimer = {}, const IExecutorPtr& executor = nullptr, bool storeRawDataInReceiveStruct = false) override;
    virtual int bind(const std::string& endpoint, const BindProperties& bindProperties = {}) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const ConnectProperties& connectProperties = {}) override;
    virtual void run() override;
    virtual void terminatePollerLoop() override;
    virtual IExecutorPtr getExecutor() const override;

    virtual EntityId registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, const std::string& name = "") override;
    virtual void addPureDataPaths(std::vector<std::string>& paths) override;
    virtual void unregisterEntity(EntityId entityId) override;
    virtual void registerConnectionEvent(FuncConnectionEvent funcConnectionEvent) override;

    virtual std::vector<EntityId> getAllEntities() const override;
    virtual hybrid_ptr<IRemoteEntity> getEntity(EntityId entityId) const override;

private:
    // IProtocolSessionCallback
    virtual void connected(const IProtocolSessionPtr& session) override;
    virtual void disconnected(const IProtocolSessionPtr& session) override;
    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message) override;
    virtual void socketConnected(const IProtocolSessionPtr& session) override;
    virtual void socketDisconnected(const IProtocolSessionPtr& session) override;

    inline void triggerConnectionEvent(const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) const;
    void deinit();
    bool isPureDataPath(const std::string& path);

    static bool isTimerExpired(std::chrono::time_point<std::chrono::system_clock>& lastTime, int interval);


    std::unique_ptr<IProtocolSessionContainer>                  m_protocolSessionContainer;
    std::unordered_map<std::string, EntityId>                   m_name2entityId;
    std::unordered_map<EntityId, hybrid_ptr<IRemoteEntity>>     m_entityId2entity;
    EntityId                                                    m_nextEntityId = 1;
    std::shared_ptr<FuncConnectionEvent>                        m_funcConnectionEvent;
    bool                                                        m_storeRawDataInReceiveStruct = false;
    std::chrono::time_point<std::chrono::system_clock>          m_lastCheckTime;
    std::list<std::string>                                      m_pureDataPaths;
    std::list<std::string>                                      m_pureDataPathPrefixes;
    std::shared_ptr<FileTransferReply>                          m_fileTransferReply;
    IExecutorPtr                                                m_executor;
    mutable std::mutex                                          m_mutex;
};

}   // namespace finalmq
