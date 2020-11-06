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

#include "protocolconnection/ProtocolSessionContainer.h"

namespace finalmq {


using ObjectId = int;


struct IRemoteObject
{
    virtual ~IRemoteObject() {}
};



enum class RemoteObjectProtocol
{
    PROT_PROTO,
    PROT_JSON
};

struct IRemoteObjectContainer
{
    virtual ~IRemoteObjectContainer() {}

    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) = 0;
    virtual int bind(const std::string& endpoint, RemoteObjectProtocol protocol) = 0;
    virtual void unbind(const std::string& endpoint) = 0;
    virtual void connect(const std::string& endpoint, RemoteObjectProtocol protocol, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
    virtual void threadEntry() = 0;
    virtual bool terminatePollerLoop(int timeout) = 0;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, RemoteObjectProtocol protocol, const CertificateData& certificateData) = 0;
    virtual void connectSsl(const std::string& endpoint, RemoteObjectProtocol protocol, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) = 0;
#endif

    virtual ObjectId registerObject(hybrid_ptr<IRemoteObject> remoteObject, const std::string& name = "") = 0;
    virtual void unregisterObject(ObjectId objectId) = 0;
};

typedef std::shared_ptr<IRemoteObjectContainer> IRemoteObjectContainerPtr;



//struct IStreamConnectionContainer;


static constexpr int INVALID_OBJECTID = -1;



class RemoteObjectContainer : public IRemoteObjectContainer
                            , public IProtocolSessionCallback
{
public:
    RemoteObjectContainer();
    virtual ~RemoteObjectContainer();

private:
    // IRemoteObjectContainer
    virtual void init(int cycleTime = 100, int checkReconnectInterval = 1000) override;
    virtual int bind(const std::string& endpoint, RemoteObjectProtocol protocol) override;
    virtual void unbind(const std::string& endpoint) override;
    virtual void connect(const std::string& endpoint, RemoteObjectProtocol protocol, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
    virtual void threadEntry() override;
    virtual bool terminatePollerLoop(int timeout) override;

#ifdef USE_OPENSSL
    virtual int bindSsl(const std::string& endpoint, RemoteObjectProtocol protocol, const CertificateData& certificateData) override;
    virtual void connectSsl(const std::string& endpoint, RemoteObjectProtocol protocol, const CertificateData& certificateData, int reconnectInterval = 5000, int totalReconnectDuration = -1) override;
#endif

    virtual ObjectId registerObject(hybrid_ptr<IRemoteObject> remoteObject, const std::string& name = "") override;
    virtual void unregisterObject(ObjectId objectId) override;

    // IProtocolSessionCallback
    virtual void connected(const IProtocolSessionPtr& session) override;
    virtual void disconnected(const IProtocolSessionPtr& session) override;
    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message) override;
    virtual void socketConnected(const IProtocolSessionPtr& session) override;
    virtual void socketDisconnected(const IProtocolSessionPtr& session) override;

private:
    static IProtocolFactoryPtr createProtocolFactory(RemoteObjectProtocol protocol);
    static IProtocolPtr createProtocol(RemoteObjectProtocol protocol);


    std::unique_ptr<IProtocolSessionContainer>                  m_streamConnectionContainer;
    std::unordered_map<std::string, ObjectId>                   m_name2objectId;
    std::unordered_map<ObjectId, hybrid_ptr<IRemoteObject>>     m_objectId2object;
    int                                                         m_nextObjectId = 1;
    mutable std::mutex                                          m_mutex;
};

}   // namespace finalmq
