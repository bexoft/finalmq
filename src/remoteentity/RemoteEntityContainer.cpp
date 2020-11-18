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

#include "remoteentity/entitydata.fmq.h"


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;
using finalmq::remoteentity::ErrorReply;


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

int RemoteEntityContainer::bind(const std::string& endpoint, IProtocolFactoryPtr protocolFactory, RemoteEntityContentType contentType)
{
    return m_streamConnectionContainer->bind(endpoint, this, protocolFactory, contentType);
}

void RemoteEntityContainer::unbind(const std::string& endpoint)
{
    m_streamConnectionContainer->unbind(endpoint);
}

void RemoteEntityContainer::connect(const std::string& endpoint, const IProtocolPtr& protocol, RemoteEntityContentType contentType, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connect(endpoint, this, protocol, reconnectInterval, totalReconnectDuration, contentType);
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
int RemoteEntityContainer::bindSsl(const std::string& endpoint, const IProtocolFactoryPtr protocolFactory, RemoteEntityContentType contentType, const CertificateData& certificateData)
{
    return m_streamConnectionContainer->bindSsl(endpoint, this, protocolFactory, certificateData, contentType);
}

void RemoteEntityContainer::connectSsl(const std::string& endpoint, const IProtocolPtr& protocol, RemoteEntityContentType contentType, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connectSsl(endpoint, this, protocol, certificateData, reconnectInterval, totalReconnectDuration, contentType);
}

#endif

EntityId RemoteEntityContainer::registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, const std::string& name)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!name.empty())
    {
        auto it = m_name2entityId.find(name);
        if (it != m_name2entityId.end())
        {
            return ENTITYID_INVALID;
        }
    }

    EntityId entityId = m_nextEntityId;
    ++m_nextEntityId;

    if (!name.empty())
    {
        m_name2entityId[name] = entityId;
    }

    auto re = remoteEntity.lock();
    if (re)
    {
        re->initEntity(entityId);
    }
    m_entityId2entity[entityId] = remoteEntity;

    return entityId;
}

void RemoteEntityContainer::unregisterEntity(EntityId entityId)
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

void RemoteEntityContainer::disconnected(const IProtocolSessionPtr& session)
{
    std::vector<hybrid_ptr<IRemoteEntity>> entities;
    std::unique_lock<std::mutex> lock(m_mutex);
    entities.reserve(m_entityId2entity.size());
    for (auto it = m_entityId2entity.begin(); it != m_entityId2entity.end(); ++it)
    {
        entities.push_back(it->second);
    }
    lock.unlock();

    for (size_t i = 0; i < entities.size(); ++i)
    {
        auto entity = entities[i].lock();
        if (entity)
        {
            entity->sessionDisconnected(session);
        }
    }
}


void RemoteEntityContainer::received(const IProtocolSessionPtr& session, const IMessagePtr& message)
{
    assert(session);
    assert(message);

    Header header;
    std::shared_ptr<StructBase> structBase = RemoteEntityFormat::parseMessage(*message, session->getContentType(), header);

    Status status = Status::STATUS_INVALID_MESSAGE;
    hybrid_ptr<IRemoteEntity> remoteEntity;
    if (structBase)
    {
        status = Status::STATUS_ENTITY_NOT_FOUND;
        EntityId entityId = header.destid;
        std::unique_lock<std::mutex> lock(m_mutex);
        if (entityId == ENTITYID_INVALID)
        {
            auto itName = m_name2entityId.find(header.destname);
            if (itName != m_name2entityId.end())
            {
                entityId = itName->second;
            }
        }
        if (entityId != ENTITYID_INVALID)
        {
            auto it = m_entityId2entity.find(header.destid);
            if (it != m_entityId2entity.end())
            {
                remoteEntity = it->second;
                status = Status::STATUS_OK;
            }
        }
    }

    auto entity = remoteEntity.lock();
    if (entity)
    {
        assert(status == Status::STATUS_OK);
        entity->received(session, header, *structBase);
    }
    else
    {
        assert(status != Status::STATUS_OK);
        Header headerReply{header.srcid, "", header.destid, MsgMode::MSG_REPLY, status, ErrorReply::structInfo().getTypeName(), header.corrid};
        RemoteEntityFormat::send(session, headerReply, ErrorReply());
    }
}

void RemoteEntityContainer::socketConnected(const IProtocolSessionPtr& /*session*/)
{

}

void RemoteEntityContainer::socketDisconnected(const IProtocolSessionPtr& /*session*/)
{

}



}   // namespace finalmq
