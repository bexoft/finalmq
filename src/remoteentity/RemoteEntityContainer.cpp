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

#include "remoteentity/RemoteEntityContainer.h"
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocols/ProtocolDelimiter.h"



namespace finalmq {


RemoteEntityContainer::RemoteEntityContainer()
    : m_streamConnectionContainer(std::make_unique<ProtocolSessionContainer>())
{

}

RemoteEntityContainer::~RemoteEntityContainer()
{

}


// IRemoteEntityContainer

void RemoteEntityContainer::init(int cycleTime, int checkReconnectInterval)
{
    m_streamConnectionContainer->init(cycleTime, checkReconnectInterval);
}

int RemoteEntityContainer::bind(const std::string& endpoint, RemoteEntityProtocol protocol)
{
    return m_streamConnectionContainer->bind(endpoint, this, createProtocolFactory(protocol));
}

void RemoteEntityContainer::unbind(const std::string& endpoint)
{
    m_streamConnectionContainer->unbind(endpoint);
}

void RemoteEntityContainer::connect(const std::string& endpoint, RemoteEntityProtocol protocol, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connect(endpoint, this, createProtocol(protocol), reconnectInterval, totalReconnectDuration);
}

void RemoteEntityContainer::threadEntry()
{
    m_streamConnectionContainer->threadEntry();
}

bool RemoteEntityContainer::terminatePollerLoop(int timeout)
{
    return m_streamConnectionContainer->terminatePollerLoop(timeout);
}


#ifdef USE_OPENSSL
int RemoteEntityContainer::bindSsl(const std::string& endpoint, RemoteEntityProtocol protocol, const CertificateData& certificateData)
{
    return m_streamConnectionContainer->bindSsl(endpoint, this, createProtocolFactory(protocol), certificateData);
}

void RemoteEntityContainer::connectSsl(const std::string& endpoint, RemoteEntityProtocol protocol, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connectSsl(endpoint, this, createProtocol(protocol), certificateData, reconnectInterval, totalReconnectDuration);
}

#endif

int RemoteEntityContainer::registerEntity(hybrid_ptr<IRemoteEntity> RemoteEntity, const std::string& name)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!name.empty())
    {
        auto it = m_name2entityId.find(name);
        if (it != m_name2entityId.end())
        {
            return INVALID_ENTITYID;
        }
    }

    EntityId entityId = m_nextEntityId;
    ++m_nextEntityId;

    if (!name.empty())
    {
        m_name2entityId[name] = entityId;
    }

    m_entityId2entity[entityId] = RemoteEntity;

    return entityId;
}

void RemoteEntityContainer::unregisterEntity(int entityId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_name2entityId.begin(); it != m_name2entityId.end(); ++it)
    {
        if (it->second == entityId)
        {
            m_name2entityId.erase(it);
            break;
        }
    }
    m_entityId2entity.erase(entityId);
}

// IProtocolSessionCallback
void RemoteEntityContainer::connected(const IProtocolSessionPtr& /*session*/)
{

}

void RemoteEntityContainer::disconnected(const IProtocolSessionPtr& /*session*/)
{

}

void RemoteEntityContainer::received(const IProtocolSessionPtr& /*session*/, const IMessagePtr& /*message*/)
{

}

void RemoteEntityContainer::socketConnected(const IProtocolSessionPtr& /*session*/)
{

}

void RemoteEntityContainer::socketDisconnected(const IProtocolSessionPtr& /*session*/)
{

}


IProtocolFactoryPtr RemoteEntityContainer::createProtocolFactory(RemoteEntityProtocol protocol)
{
    switch (protocol)
    {
    case RemoteEntityProtocol::PROT_PROTO:
        return std::make_shared<ProtocolHeaderBinarySizeFactory>();
    case RemoteEntityProtocol::PROT_JSON:
        return std::make_shared<ProtocolDelimiterFactory>("\n");
    }
    assert(false);
    return nullptr;
}


IProtocolPtr RemoteEntityContainer::createProtocol(RemoteEntityProtocol protocol)
{
    switch (protocol)
    {
    case RemoteEntityProtocol::PROT_PROTO:
        return std::make_shared<ProtocolHeaderBinarySize>();
    case RemoteEntityProtocol::PROT_JSON:
        return std::make_shared<ProtocolDelimiter>("\n");
    }
    assert(false);
    return nullptr;
}



}   // namespace finalmq
