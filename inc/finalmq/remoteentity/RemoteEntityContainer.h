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
    /**
     * @brief ~IRemoteEntityContainer is the virtual destructor of the interface.
     */
    virtual ~IRemoteEntityContainer() {}

    /**
     * @brief init initializes the instance. Call it once after constructing the object.
     * @param executor decouples the callback thread context from the polling thread.
     * @param cycleTime is a timer interval in [ms] in which the poller thread will trigger a timer callback (see parameter funcTimer).
     * @param funcTimer is a callback that is been called every cycleTime.
     * @param storeRawDataInReceiveStruct is a flag. It is usually false. But if you wish to have the raw data inside a message struct, then you can set this flag to true.
     * @param checkReconnectInterval is the timer interval in [ms] in which the reconnect timers will be checked (the reconnect timers are not checked every cycleTime). Unit tests which test reconnection, set this parameter to 1ms to have faster tests.
     */
    virtual void init(const IExecutorPtr& executor = nullptr, int cycleTime = 100, FuncTimer funcTimer = nullptr, bool storeRawDataInReceiveStruct = false, int checkReconnectInterval = 1000) = 0;

    ///
    /// @brief bind opens a listener socket.
    /// @param endpoint defines the type of socket, port or socket name, protocol and the data encoding.
    /// examples:
    /// The space between // and * is in the comment, because
    /// "tcp://*:3000/httpserver:json" = tcp socket, port 3000, http protocol, json data encoding
    /// "ipc://my_uds/delimiter_lf:json" = unix domain socket, name my_uds, delimiter linefeed protocol, json data encoding
    /// "tcp://*:3000/headersize:protobuf" = tcp socket, port 3000, 4 bytes payload size in header, protobuf data encoding
    /// @param bindProperties contains properties for the bind, e.g. SSL/TLS properties.
    /// @return 0 on success and -1 on failure.
    ///
    virtual int bind(const std::string& endpoint, const BindProperties& bindProperties = {}) = 0;

    /**
     * @brief unbind removes the listening socket
     * @param endpoint must match with the endpoint from the bind call.
     */
    virtual void unbind(const std::string& endpoint) = 0;

    /**
     * @brief connect to a peer endpoint.
     * @param endpoint defines the type of socket, hostname or IP address, port or socket name, protocol and the data encoding.
     * examples:
     * The space between // and * is in the comment, because
     * "tcp://localhost:3000/httpserver:json" = tcp socket, port 3000, http protocol, json data encoding
     * "ipc://my_uds/delimiter_lf:json" = unix domain socket, name my_uds, delimiter linefeed protocol, json data encoding
     * "tcp://127.0.0.1:3000/headersize:protobuf" = tcp socket, port 3000, 4 bytes payload size in header, protobuf data encoding
     * @param connectProperties contains properties for the connect, e.g. SSL/TLS properties.
     * @return the session.
     */
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const ConnectProperties& connectProperties = {}) = 0;

    /**
     * @brief run activates the poller loop (it handles all socket connections). You can call it from any thread. All callbacks will be called from the thread which calles run(). run() blocks till terminatePollerLoop() is called. Do not call it in case of passing an executor in the init() (in this case the callbacks will be called from the executor's context).
     */
    virtual void run() = 0;

    /**
     * @brief terminatePollerLoop releases the poller loop.
     */
    virtual void terminatePollerLoop() = 0;

    /**
     * @brief getExecutor gets the executor (there is also an executor in case if you do not pass an executor at init()).
     * Use the executor to run code in the executor context.
     * Example:
     * entityContainer.getExecutor()->addAction([]() {
     *     // put here application code that will be executed in the context of the executor
     * });
     * @return the executor.
     */
    virtual IExecutorPtr getExecutor() const = 0;

    /**
     * @brief registerEntity registers an entity. Multiple entities can be registered.
     * @param remoteEntity the entity that shall be registered.
     * @param name is the name, how a peer can find the entity.
     * @return the entity ID.
     */
    virtual EntityId registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, std::string name = "") = 0;

    /**
     * @brief addPureDataPaths define the paths for receiving pure binary data. You can also use '*' at the end
     * of the path to include all sub-paths. You will use this method to accept e.g. a file upload via HTTP to
     * a given path.
     * @param paths
     */
    virtual void addPureDataPaths(std::vector<std::string>& paths) = 0;

    /**
     * @brief unregisterEntity unregisters an entity.
     * @param entityId is the entity ID that shall be unregistered.
     */
    virtual void unregisterEntity(EntityId entityId) = 0;

    /**
     * @brief registerConnectionEvent registers a function that will be called whenever a connection state has changed.
     * @param funcConnectionEvent is the callback function.
     */
    virtual void registerConnectionEvent(FuncConnectionEvent funcConnectionEvent) = 0;

    /**
     * @brief getAllEntities gets all entities as IDs.
     * @return a list of entities.
     */
    virtual std::vector<EntityId> getAllEntities() const = 0;

    /**
     * @brief getEntity gets an entity object by its ID.
     * @return the entity.
     */
    virtual hybrid_ptr<IRemoteEntity> getEntity(EntityId) const = 0;
};

typedef std::shared_ptr<IRemoteEntityContainer> IRemoteEntityContainerPtr;





class SYMBOLEXP RemoteEntityContainer : public IRemoteEntityContainer
                                      , private IProtocolSessionCallback
{
public:
    RemoteEntityContainer();
    virtual ~RemoteEntityContainer();

    // IRemoteEntityContainer
    virtual void init(const IExecutorPtr& executor = nullptr, int cycleTime = 100, FuncTimer funcTimer = {}, bool storeRawDataInReceiveStruct = false, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, const BindProperties& bindProperties = {}) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const ConnectProperties& connectProperties = {}) override;
    virtual void run() override;
    virtual void terminatePollerLoop() override;
    virtual IExecutorPtr getExecutor() const override;

    virtual EntityId registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, std::string name = "") override;
    virtual void addPureDataPaths(std::vector<std::string>& paths) override;
    virtual void unregisterEntity(EntityId entityId) override;
    virtual void registerConnectionEvent(FuncConnectionEvent funcConnectionEvent) override;

    virtual std::vector<EntityId> getAllEntities() const override;
    virtual hybrid_ptr<IRemoteEntity> getEntity(EntityId entityId) const override;

private:
    // IProtocolSessionCallback
    virtual void connected(const IProtocolSessionPtr& session) override;
    virtual void disconnected(const IProtocolSessionPtr& session) override;
    virtual void disconnectedVirtualSession(const IProtocolSessionPtr& session, const std::string& virtualSessionId) override;
    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message) override;
    virtual void socketConnected(const IProtocolSessionPtr& session) override;
    virtual void socketDisconnected(const IProtocolSessionPtr& session) override;

    inline void triggerConnectionEvent(const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) const;
    void deinit();
    bool isPureDataPath(const std::string& path);
    void subscribeEntityNames(const IProtocolSessionPtr& session);
    void subscribeSessions(const std::string& name);

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

    std::atomic_bool                                            m_entityNamesChanged = {};
    std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>  m_name2Entity;

    mutable std::mutex                                          m_mutex;
};

}   // namespace finalmq
