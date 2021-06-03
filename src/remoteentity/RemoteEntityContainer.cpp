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

#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/variant/VariantValues.h"

#include "finalmq/remoteentity/entitydata.fmq.h"


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;


namespace finalmq {



ConnectionEvent::ConnectionEvent()
{
}
ConnectionEvent::ConnectionEvent(Enum en)
    : m_value(en)
{
}
ConnectionEvent::operator const Enum&() const
{
    return m_value;
}
ConnectionEvent::operator Enum&()
{
    return m_value;
}
const ConnectionEvent& ConnectionEvent::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& ConnectionEvent::toString() const
{
    return ConnectionEvent::_enumInfo.getMetaEnum().getNameByValue(m_value);
}
void ConnectionEvent::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo ConnectionEvent::_enumInfo = {
    "ConnectionEvent", "", {
        {"CONNECTIONEVENT_CONNECTED", 0, ""},
        {"CONNECTIONEVENT_DISCONNECTED", 1, ""},
        {"CONNECTIONEVENT_SOCKET_CONNECTED", 2, ""},
        {"CONNECTIONEVENT_SOCKET_DISCONNECTED", 3, ""},
     }
};



///////////////////////////////////

RemoteEntityContainer::RemoteEntityContainer()
    : m_protocolSessionContainer(std::make_unique<ProtocolSessionContainer>())
    , m_fileTransferReply(std::make_shared<FileTransferReply>())
{
}

RemoteEntityContainer::~RemoteEntityContainer()
{
    deinit();
}


void RemoteEntityContainer::deinit()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<hybrid_ptr<IRemoteEntity>> entities;
    entities.reserve(m_entityId2entity.size());
    for (auto it = m_entityId2entity.begin(); it != m_entityId2entity.end(); ++it)
    {
        entities.push_back(it->second);
    }
    m_name2entityId.clear();
    m_entityId2entity.clear();
    lock.unlock();

    for (size_t i = 0; i < entities.size(); ++i)
    {
        auto entity = entities[i].lock();
        if (entity)
        {
            entity->deinit();
        }
    }
}



// IRemoteEntityContainer


bool RemoteEntityContainer::isTimerExpired(std::chrono::time_point<std::chrono::system_clock>& lastTime, int interval)
{
    bool expired = false;
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    std::chrono::duration<double> dur = now - lastTime;
    int delta = static_cast<int>(dur.count() * 1000);
    if (delta >= interval)
    {
        lastTime = now;
        expired = true;
    }
    else if (delta < 0)
    {
        lastTime = now;
    }

    return expired;
}

static const int INTERVAL_CHECK = 5000;


void RemoteEntityContainer::init(int cycleTime, int checkReconnectInterval, FuncPollerLoopTimer funcTimer, const IExecutorPtr& executor, bool storeRawDataInReceiveStruct)
{
    m_storeRawDataInReceiveStruct = storeRawDataInReceiveStruct;
    m_protocolSessionContainer->init(cycleTime, checkReconnectInterval, std::move(funcTimer), executor);

    //registerEntity([this]() {
    //    std::shared_ptr<RemoteEntity> entity = std::make_shared<RemoteEntity>();
    //    entity->registerCommand<COMMAND>([this](ReplyContextUPtr& replyContext, const std::shared_ptr<COMMAND>& request) {
    //    });
    //    return entity;
    //}(), "fmq");
}

int RemoteEntityContainer::bind(const std::string& endpoint, IProtocolFactoryPtr protocolFactory, int contentType, const BindProperties& bindProperties)
{
    return m_protocolSessionContainer->bind(endpoint, this, protocolFactory, bindProperties, contentType);
}

void RemoteEntityContainer::unbind(const std::string& endpoint)
{
    m_protocolSessionContainer->unbind(endpoint);
}

IProtocolSessionPtr RemoteEntityContainer::connect(const std::string& endpoint, const IProtocolPtr& protocol, int contentType, const ConnectProperties& connectProperties)
{
    return m_protocolSessionContainer->connect(endpoint, this, protocol, connectProperties, contentType);
}


void RemoteEntityContainer::run()
{
    m_protocolSessionContainer->run();
    deinit();
}

void RemoteEntityContainer::terminatePollerLoop()
{
    m_protocolSessionContainer->terminatePollerLoop();
}


EntityId RemoteEntityContainer::registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, const std::string& name)
{
    auto re = remoteEntity.lock();
    if (!re || re->isEntityRegistered())
    {
        streamError << "entity is invalid or already registered";
        return ENTITYID_INVALID;
    }

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

    re->initEntity(entityId, name, m_fileTransferReply);
    m_entityId2entity[entityId] = remoteEntity;

    return entityId;
}


void RemoteEntityContainer::addPureDataPaths(std::vector<std::string>& paths)
{
    for (size_t i = 0; i < paths.size(); ++i)
    {
        const std::string& path = paths[i];
        if (!path.empty())
        {
            if (path[path.size() - 1] == '*')
            {
                m_pureDataPathPrefixes.emplace_back(path.data(), path.size() - 1);
            }
            else
            {
                m_pureDataPaths.emplace_back(path);
            }
        }
    }
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


void RemoteEntityContainer::registerConnectionEvent(FuncConnectionEvent funcConnectionEvent)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_funcConnectionEvent = std::make_shared<FuncConnectionEvent>(std::move(funcConnectionEvent));
}


std::vector<EntityId> RemoteEntityContainer::getAllEntities() const
{
    std::vector<EntityId> entities;
    entities.reserve(m_entityId2entity.size());
    for (auto it = m_entityId2entity.begin(); it != m_entityId2entity.end(); ++it)
    {
        entities.push_back(it->first);
    }
    return entities;
}

hybrid_ptr<IRemoteEntity> RemoteEntityContainer::getEntity(EntityId entityId) const
{
    std::vector<EntityId> entities;
    auto it = m_entityId2entity.find(entityId);
    if (it !=  m_entityId2entity.end())
    {
        return it->second;
    }
    return nullptr;
}




inline void RemoteEntityContainer::triggerConnectionEvent(const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<FuncConnectionEvent> funcConnectionEvent = m_funcConnectionEvent;
    lock.unlock();
    if (funcConnectionEvent && *funcConnectionEvent)
    {
        (*funcConnectionEvent)(session, connectionEvent);
    }
}


// IProtocolSessionCallback
void RemoteEntityContainer::connected(const IProtocolSessionPtr& session)
{
    triggerConnectionEvent(session, ConnectionEvent::CONNECTIONEVENT_CONNECTED);
}

void RemoteEntityContainer::disconnected(const IProtocolSessionPtr& session)
{
    triggerConnectionEvent(session, ConnectionEvent::CONNECTIONEVENT_DISCONNECTED);

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


bool RemoteEntityContainer::isPureDataPath(const std::string& path)
{
    for (auto it = m_pureDataPathPrefixes.begin(); it != m_pureDataPathPrefixes.end(); ++it)
    {
        const std::string& prefix = *it;
        if (path.size() >= prefix.size() && path.compare(0, prefix.size(), prefix) == 0)
        {
            return true;
        }
    }
    for (auto it = m_pureDataPaths.begin(); it != m_pureDataPaths.end(); ++it)
    {
        const std::string& pathCompare = *it;
        if (path == pathCompare)
        {
            return true;
        }
    }
    return false;
}



static const std::string FMQ_PATH = "fmq_path";

void RemoteEntityContainer::received(const IProtocolSessionPtr& session, const IMessagePtr& message)
{
    assert(session);
    assert(message);

    bool pureData = false;
    if (session->doesSupportMetainfo())
    {
        const std::string* path = message->getControlData().getData<std::string>(FMQ_PATH);
        if (path)
        {
            if (isPureDataPath(*path))
            {
                pureData = true;
            }
        }
    }

    bool syntaxError = false;
    ReceiveData receiveData{ session, message, {}, {} };
    if (!pureData)
    {
        if (!session->doesSupportMetainfo())
        {
            receiveData.structBase = RemoteEntityFormatRegistry::instance().parse(*message, session->getContentType(), m_storeRawDataInReceiveStruct, receiveData.header, syntaxError);
        }
        else
        {
            receiveData.structBase = RemoteEntityFormatRegistry::instance().parseHeaderInMetainfo(*message, session->getContentType(), m_storeRawDataInReceiveStruct, receiveData.header, syntaxError);
        }
    }
    else
    {
        receiveData.structBase = RemoteEntityFormatRegistry::instance().parsePureData(*message, receiveData.header);
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    EntityId entityId = receiveData.header.destid;
    if (entityId == ENTITYID_INVALID || (entityId == ENTITYID_DEFAULT && !receiveData.header.destname.empty()))
    {
        auto itName = m_name2entityId.find(receiveData.header.destname);
        if (itName == m_name2entityId.end())
        {
            itName = m_name2entityId.find("*");
        }
        if (itName != m_name2entityId.end())
        {
            entityId = itName->second;
        }
    }
    hybrid_ptr<IRemoteEntity> remoteEntity;
    if (entityId != ENTITYID_INVALID)
    {
        auto it = m_entityId2entity.find(entityId);
        if (it != m_entityId2entity.end())
        {
            remoteEntity = it->second;
        }
    }
    lock.unlock();

    const std::string& type = receiveData.header.type;

    auto entity = remoteEntity.lock();
    if (receiveData.header.mode == MsgMode::MSG_REQUEST)
    {
        Status replyStatus = Status::STATUS_OK;
        if (!syntaxError)
        {
            if (entity)
            {
                entity->receivedRequest(receiveData);
            }
            else
            {
                replyStatus = Status::STATUS_ENTITY_NOT_FOUND;
            }
        }
        else
        {
            replyStatus = Status::STATUS_SYNTAX_ERROR;
        }
        if (replyStatus != Status::STATUS_OK)
        {
            Header headerReply{ receiveData.header.srcid, "", entityId, MsgMode::MSG_REPLY, replyStatus, "", receiveData.header.corrid, {} };
            RemoteEntityFormatRegistry::instance().send(session, headerReply, std::move(message->getEchoData()));
        }
    }
    else if (receiveData.header.mode == MsgMode::MSG_REPLY)
    {
        if (entity)
        {
            if (!receiveData.structBase && receiveData.header.status == Status::STATUS_OK && !type.empty())
            {
                receiveData.header.status = Status::STATUS_REPLYTYPE_NOT_KNOWN;
            }
            entity->receivedReply(receiveData);
        }
    }
}

void RemoteEntityContainer::socketConnected(const IProtocolSessionPtr& session)
{
    triggerConnectionEvent(session, ConnectionEvent::CONNECTIONEVENT_SOCKET_CONNECTED);
}

void RemoteEntityContainer::socketDisconnected(const IProtocolSessionPtr& session)
{
    triggerConnectionEvent(session, ConnectionEvent::CONNECTIONEVENT_SOCKET_DISCONNECTED);
}



}   // namespace finalmq
