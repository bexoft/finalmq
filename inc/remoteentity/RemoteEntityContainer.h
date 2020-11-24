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

#include "remoteentity/RemoteEntity.h"

namespace finalmq {

enum class ConnectionEvent
{
    CONNECTIONEVENT_CONNECTED,
    CONNECTIONEVENT_DISCONNECTED,
    CONNECTIONEVENT_SOCKET_CONNECTED,
    CONNECTIONEVENT_SOCKET_DISCONNECTED,
};
typedef std::function<void(const IProtocolSessionPtr& session, ConnectionEvent connectionEvent)> FuncConnectionEvent;


struct IRemoteEntityContainer
{
    virtual ~IRemoteEntityContainer() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, const IProtocolFactoryPtr protocolFactory, RemoteEntityContentType contentType) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const IProtocolPtr& protocol, RemoteEntityContentType contentType, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
    virtual void threadEntry() = 0;
    virtual bool terminatePollerLoop(int timeout) = 0;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, const IProtocolFactoryPtr protocolFactory, RemoteEntityContentType contentType, const CertificateData& certificateData) = 0;
    virtual IProtocolSessionPtr connectSsl(const std::string& endpoint, const IProtocolPtr& protocol, RemoteEntityContentType contentType, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
#endif

    virtual EntityId registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, const std::string& name = "") = 0;
    virtual void unregisterEntity(EntityId entityId) = 0;
    virtual void registerConnectionEvent(FuncConnectionEvent funcConnectionEvent) = 0;
};

typedef std::shared_ptr<IRemoteEntityContainer> IRemoteEntityContainerPtr;



class RemoteEntityContainer : public IRemoteEntityContainer
                            , public IProtocolSessionCallback
{
public:
    RemoteEntityContainer();
    virtual ~RemoteEntityContainer();

private:
    // IRemoteEntityContainer
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, const IProtocolFactoryPtr protocolFactory, RemoteEntityContentType contentType) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual IProtocolSessionPtr connect(const std::string& endpoint, const IProtocolPtr& protocol, RemoteEntityContentType contentType, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
    virtual void threadEntry() override;
    virtual bool terminatePollerLoop(int timeout) override;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, const IProtocolFactoryPtr protocolFactory, RemoteEntityContentType contentType, const CertificateData& certificateData) override;
    virtual IProtocolSessionPtr connectSsl(const std::string& endpoint, const IProtocolPtr& protocol, RemoteEntityContentType contentType, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
#endif

    virtual EntityId registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, const std::string& name = "") override;
    virtual void unregisterEntity(EntityId entityId) override;
    virtual void registerConnectionEvent(FuncConnectionEvent funcConnectionEvent) override;

    // IProtocolSessionCallback
    virtual void connected(const IProtocolSessionPtr& session) override;
    virtual void disconnected(const IProtocolSessionPtr& session) override;
    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message) override;
    virtual void socketConnected(const IProtocolSessionPtr& session) override;
    virtual void socketDisconnected(const IProtocolSessionPtr& session) override;

private:

    inline void triggerConnectionEvent(const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) const;

    std::unique_ptr<IProtocolSessionContainer>                  m_streamConnectionContainer;
    std::unordered_map<std::string, EntityId>                   m_name2entityId;
    std::unordered_map<EntityId, hybrid_ptr<IRemoteEntity>>     m_entityId2entity;
    EntityId                                                    m_nextEntityId = 1;
    std::shared_ptr<FuncConnectionEvent>                        m_funcConnectionEvent;
    mutable std::mutex                                          m_mutex;
};

}   // namespace finalmq
