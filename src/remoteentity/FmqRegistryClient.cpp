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

#include "remoteentity/FmqRegistryClient.h"
#include "protocols/ProtocolHeaderBinarySize.h"

using finalmq::fmqreg::RegisterService;
using finalmq::fmqreg::GetService;
using finalmq::fmqreg::GetServiceReply;


#define PORTNUMBER_PROTO    "18180"


namespace finalmq {

FmqRegistryClient::FmqRegistryClient(const IRemoteEntityContainerPtr& remoteEntityContainer)
    : m_remoteEntityContainer(remoteEntityContainer)
{
}



void FmqRegistryClient::init()
{
    if (!m_init)
    {
        m_init = true;
        m_entityRegistry = std::make_shared<RemoteEntity>();
        m_remoteEntityContainer->registerEntity(m_entityRegistry);
    }
}



PeerId FmqRegistryClient::connectService(const std::string& serviceName, hybrid_ptr<IRemoteEntity> remoteEntity, const ConnectProperties& connectProperties, FuncReplyConnect funcReplyConnect)
{
    PeerId peerId = PEERID_INVALID;

    auto re = remoteEntity.lock();
    if (re)
    {
        if (!re->isEntityRegistered())
        {
            m_remoteEntityContainer->registerEntity(remoteEntity);
        }
        peerId = re->createPeer(std::move(funcReplyConnect));
    }

    std::string hostname;
    std::string remainingServiceName = serviceName;
    std::string::size_type n = serviceName.find('/');
    if (n != std::string::npos)
    {
        hostname = serviceName.substr(0, n);
        remainingServiceName = serviceName.substr(n+1);
    }
    if (hostname.empty())
    {
        hostname = "127.0.0.1";
    }

    ConnectProperties connectPropertiesRegistry;
    connectPropertiesRegistry.reconnectInterval = connectProperties.reconnectInterval;
    connectPropertiesRegistry.totalReconnectDuration = connectProperties.totalReconnectDuration;
    IProtocolSessionPtr sessionRegistry = createRegistrySession(hostname, connectPropertiesRegistry);
    assert(sessionRegistry);
    PeerId peerIdRegistry = m_entityRegistry->connect(sessionRegistry, "fmqreg");
    m_entityRegistry->requestReply<GetServiceReply>(peerIdRegistry, GetService{remainingServiceName}, [this, sessionRegistry, connectProperties, remoteEntity, peerId]
                                                    (PeerId /*peerIdRegistry*/, remoteentity::Status /*status*/, const std::shared_ptr<GetServiceReply>& reply) {
        if (reply)
        {
            auto re = remoteEntity.lock();
            if (re)
            {
                IProtocolSessionPtr session;// = m_remoteEntityContainer->connect(reply->service.endpoints[0].endpoint, reply->service.endpoints[0].framingprotocol, reply->service.endpoints[0].contenttype, connectProperties);
                re->connect(peerId, session, reply->service.entityname, reply->service.entityid);
            }
        }
        sessionRegistry->disconnect();
    });

    return peerId;
}




void FmqRegistryClient::registerService(const finalmq::fmqreg::Service& service)
{
    init();

    ConnectProperties connectProperties;
    connectProperties.totalReconnectDuration = 60000;
    IProtocolSessionPtr session = createRegistrySession("127.0.0.1", connectProperties);
    assert(session);
    PeerId peerId = m_entityRegistry->connect(session, "fmqreg");
    m_entityRegistry->sendRequest(peerId, RegisterService{service}, [session] (PeerId /*peerId*/, remoteentity::Status /*status*/, const StructBasePtr& /*reply*/) {
        session->disconnect();
    });
}





IProtocolSessionPtr FmqRegistryClient::createRegistrySession(const std::string& hostname, const ConnectProperties& connectProperties)
{
    std::string endpoint = "tcp://";
    endpoint += hostname;
    endpoint += ":";
    endpoint += PORTNUMBER_PROTO;
    IProtocolSessionPtr session = m_remoteEntityContainer->connect(endpoint, std::make_shared<ProtocolHeaderBinarySize>(), RemoteEntityContentType::CONTENTTYPE_PROTO, connectProperties);
    return session;
}



}   // namespace finalmq
