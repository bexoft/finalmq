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

#include "finalmq/remoteentity/FmqRegistryClient.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/remoteentity/RemoteEntityFormatProto.h"

#include "finalmq/protocols/ProtocolHeaderBinarySize.h"
#include "finalmq/protocols/ProtocolDelimiter.h"

#include <thread>


using finalmq::fmqreg::RegisterService;
using finalmq::fmqreg::GetService;
using finalmq::fmqreg::GetServiceReply;


#define PORTNUMBER_PROTO    "18180"

namespace finalmq {

FmqRegistryClient::FmqRegistryClient(const hybrid_ptr<IRemoteEntityContainer>& remoteEntityContainer)
    : m_remoteEntityContainer(remoteEntityContainer)
{
}



void FmqRegistryClient::init()
{
    if (!m_init)
    {
        m_init = true;
        m_entityRegistry = std::make_shared<RemoteEntity>();
        auto remoteEntityContainer = m_remoteEntityContainer.lock();
        if (remoteEntityContainer)
        {
            remoteEntityContainer->registerEntity(m_entityRegistry);
        }
    }
}


static ssize_t pickEndpointEntry(const std::vector<fmqreg::Endpoint>& endpoints, bool ssl, bool local)
{
    ssize_t index = -1;
    ssize_t indexProtoTcp = -1;
    ssize_t indexProtoUds = -1;
    ssize_t indexJsonTcp = -1;
    ssize_t indexJsonUds = -1;
    for (ssize_t i = 0; i < static_cast<ssize_t>(endpoints.size()); ++i)
    {
        const fmqreg::Endpoint& endpoint = endpoints[i];
        if (endpoint.ssl == ssl)
        {
            if (endpoint.socketprotocol == fmqreg::SocketProtocol::SOCKET_TCP)
            {
                if (endpoint.contenttype == RemoteEntityFormatProto::CONTENT_TYPE)
                {
                    indexProtoTcp = i;
                }
                else if (endpoint.contenttype == RemoteEntityFormatJson::CONTENT_TYPE)
                {
                    indexJsonTcp = i;
                }
            }
            else if (endpoint.socketprotocol == fmqreg::SocketProtocol::SOCKET_UNIXDOMAIN)
            {
                if (endpoint.contenttype == RemoteEntityFormatProto::CONTENT_TYPE)
                {
                    indexProtoUds = i;
                }
                else if (endpoint.contenttype == RemoteEntityFormatJson::CONTENT_TYPE)
                {
                    indexJsonUds = i;
                }
            }
        }
    }

    if (indexProtoUds != -1 && local)
    {
        index = indexProtoUds;
    }
    else if (indexProtoTcp != -1)
    {
        index = indexProtoTcp;
    }
    else if (indexJsonUds != -1 && local)
    {
        index = indexJsonUds;
    }
    else if (indexJsonTcp != -1)
    {
        index = indexJsonTcp;
    }
    return index;
}



class FuncGetServiceReplyAndConnect
{
public:
    FuncGetServiceReplyAndConnect(int retries, const IRemoteEntityPtr& entityRegistry, PeerId peerIdRegistry, const std::string& serviceName, const hybrid_ptr<IRemoteEntityContainer>& remoteEntityContainer, const IProtocolSessionPtr& sessionRegistry, const ConnectProperties& connectProperties, EntityId entityId, PeerId peerId, bool local, const std::string& hostname)
        : m_retries(retries)
        , m_entityRegistry(entityRegistry)
        , m_peerIdRegistry(peerIdRegistry)
        , m_serviceName(serviceName)
        , m_remoteEntityContainer(remoteEntityContainer)
        , m_sessionRegistry(sessionRegistry)
        , m_connectProperties(connectProperties)
        , m_entityId(entityId)
        , m_peerId(peerId)
        , m_local(local)
        , m_hostname(hostname)
    {
    }

    void operator() (PeerId /*peerIdRegistry*/, remoteentity::Status /*status*/, const std::shared_ptr<GetServiceReply>& reply)
    {
        bool connectDone = false;
        bool retry = false;
        auto remoteEntityContainer = m_remoteEntityContainer.lock();
        if (remoteEntityContainer)
        {
            auto re = remoteEntityContainer->getEntity(m_entityId).lock();
            if (re)
            {
                if (reply && !reply->service.name.empty())
                {
                    ssize_t endpointIndex = pickEndpointEntry(reply->service.endpoints, m_connectProperties.certificateData.ssl, m_local);
                    if (endpointIndex != -1)
                    {
                        const fmqreg::Endpoint& endpointEntry = reply->service.endpoints[endpointIndex];
                        std::string endpoint = endpointEntry.endpoint;

                        // replace * by hostname in endpoint
                        std::string::size_type pos = endpoint.find("*");
                        if (pos != std::string::npos)
                        {
                            endpoint.replace(pos, std::string("*").length(), m_hostname);
                        }

                        IProtocolPtr protocol;
                        if (endpointEntry.framingprotocol == ProtocolDelimiter::PROTOCOL_ID)
                        {
                            protocol = std::make_shared<ProtocolDelimiter>("\n");
                        }
                        else if (endpointEntry.framingprotocol == ProtocolHeaderBinarySize::PROTOCOL_ID)
                        {
                            protocol = std::make_shared<ProtocolHeaderBinarySize>();
                        }

                        if (protocol)
                        {
                            connectDone = true;
                            IProtocolSessionPtr session = remoteEntityContainer->connect(endpoint, protocol, endpointEntry.contenttype, m_connectProperties);
                            re->connect(m_peerId, session, reply->service.entityname, reply->service.entityid);
                        }
                    }
                }
                else
                {
                    if (m_retries > 0 || m_retries == -1)
                    {
                        retry = true;
                        if (m_retries > 0)
                        {
                            --m_retries;
                        }
                        FuncGetServiceReplyAndConnect funcGetServiceReply(*this);
                        std::thread thread([funcGetServiceReply] () {
                            std::this_thread::sleep_for(std::chrono::milliseconds(funcGetServiceReply.m_connectProperties.reconnectInterval));
                            funcGetServiceReply.m_entityRegistry->requestReply<GetServiceReply>(funcGetServiceReply.m_peerIdRegistry, GetService{funcGetServiceReply.m_serviceName}, funcGetServiceReply);
                        });
                        thread.detach();
                    }
                }
                if (!connectDone && !retry)
                {
                    re->disconnect(m_peerId);
                }
            }
        }
        if (!retry)
        {
            m_sessionRegistry->disconnect();
        }
    }

private:
    int                         m_retries;
    IRemoteEntityPtr            m_entityRegistry;
    PeerId                      m_peerIdRegistry;
    std::string                 m_serviceName;
    hybrid_ptr<IRemoteEntityContainer>   m_remoteEntityContainer;
    IProtocolSessionPtr         m_sessionRegistry;
    ConnectProperties           m_connectProperties;
    EntityId                    m_entityId;
    PeerId                      m_peerId;
    bool                        m_local;
    std::string                 m_hostname;
};



PeerId FmqRegistryClient::connectService(const std::string& serviceName, EntityId entityId, const ConnectProperties& connectProperties, FuncReplyConnect funcReplyConnect)
{
    init();

    auto remoteEntityContainer = m_remoteEntityContainer.lock();
    if (!remoteEntityContainer)
    {
        return PEERID_INVALID;
    }

    auto re = remoteEntityContainer->getEntity(entityId).lock();
    if (!re)
    {
        return PEERID_INVALID;
    }

    PeerId peerId = re->createPeer(std::move(funcReplyConnect));

    std::string hostname;
    std::string remainingServiceName = serviceName;
    std::string::size_type n = serviceName.find('/');
    if (n != std::string::npos)
    {
        hostname = serviceName.substr(0, n);
        remainingServiceName = serviceName.substr(n+1);
    }
    if (hostname.empty() || hostname == "localhost")
    {
        hostname = "127.0.0.1";
    }

    bool local = false;
    if (hostname == "127.0.0.1")
    {
        local = true;
    }

    ConnectProperties connectPropertiesRegistry;
    connectPropertiesRegistry.reconnectInterval = connectProperties.reconnectInterval;
    connectPropertiesRegistry.totalReconnectDuration = connectProperties.totalReconnectDuration;
    IProtocolSessionPtr sessionRegistry = createRegistrySession(hostname, connectPropertiesRegistry);
    if (sessionRegistry)
    {
        PeerId peerIdRegistry = m_entityRegistry->connect(sessionRegistry, "fmqreg");

        int retries = -1;
        if (connectProperties.totalReconnectDuration >= 0)
        {
            retries = connectProperties.totalReconnectDuration / connectProperties.reconnectInterval;
        }
        FuncGetServiceReplyAndConnect funcGetServiceReply(retries, m_entityRegistry, peerIdRegistry, remainingServiceName, m_remoteEntityContainer, sessionRegistry, connectProperties, entityId, peerId, local, hostname);
        m_entityRegistry->requestReply<GetServiceReply>(peerIdRegistry, GetService{remainingServiceName}, funcGetServiceReply);
    }

    return peerId;
}



void FmqRegistryClient::getService(const std::string& serviceName, FuncGetServiceReply funcGetServiceReply)
{
    init();

    std::string hostname;
    std::string remainingServiceName = serviceName;
    std::string::size_type n = serviceName.find('/');
    if (n != std::string::npos)
    {
        hostname = serviceName.substr(0, n);
        remainingServiceName = serviceName.substr(n+1);
    }
    if (hostname.empty() || hostname == "localhost")
    {
        hostname = "127.0.0.1";
    }

    ConnectProperties connectPropertiesRegistry;
    connectPropertiesRegistry.totalReconnectDuration = 0;
    IProtocolSessionPtr sessionRegistry = createRegistrySession(hostname, connectPropertiesRegistry);
    if (sessionRegistry)
    {
        PeerId peerIdRegistry = m_entityRegistry->connect(sessionRegistry, "fmqreg");

        m_entityRegistry->requestReply<GetServiceReply>(peerIdRegistry, GetService{remainingServiceName}, [funcGetServiceReply{std::move(funcGetServiceReply)}]
                (PeerId /*peerId*/, remoteentity::Status status, const std::shared_ptr<GetServiceReply>& reply) {
            if (funcGetServiceReply)
            {
                funcGetServiceReply(status, reply);
            }
        });
    }
}





void FmqRegistryClient::registerService(const finalmq::fmqreg::Service& service, int retryDurationMs)
{
    init();

    ConnectProperties connectProperties;
    connectProperties.totalReconnectDuration = retryDurationMs;
    IProtocolSessionPtr session = createRegistrySession("127.0.0.1", connectProperties);
    if (session)
    {
        PeerId peerId = m_entityRegistry->connect(session, "fmqreg");
        m_entityRegistry->sendRequest(peerId, RegisterService{service}, [session] (PeerId /*peerId*/, remoteentity::Status /*status*/, const StructBasePtr& /*reply*/) {
            session->disconnect();
        });
    }
}





IProtocolSessionPtr FmqRegistryClient::createRegistrySession(const std::string& hostname, const ConnectProperties& connectProperties)
{
    std::string endpoint = "tcp://";
    endpoint += hostname;
    endpoint += ":";
    endpoint += PORTNUMBER_PROTO;
    IProtocolSessionPtr session;
    auto remoteEntityContainer = m_remoteEntityContainer.lock();
    if (remoteEntityContainer)
    {
        session = remoteEntityContainer->connect(endpoint, std::make_shared<ProtocolHeaderBinarySize>(), RemoteEntityFormatProto::CONTENT_TYPE, connectProperties);
    }
    return session;
}



}   // namespace finalmq