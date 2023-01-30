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


using finalmq::MsgMode;
using finalmq::Status;
using finalmq::Header;


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
const std::string& ConnectionEvent::toName() const
{
    return ConnectionEvent::_enumInfo.getMetaEnum().getNameByValue(m_value);
}
const std::string& ConnectionEvent::toString() const
{
    return ConnectionEvent::_enumInfo.getMetaEnum().getAliasByValue(m_value);
}
void ConnectionEvent::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo ConnectionEvent::_enumInfo = {
    "ConnectionEvent", "", {
        {"CONNECTIONEVENT_CONNECTED", 0, "", "connected"},
        {"CONNECTIONEVENT_DISCONNECTED", 1, "", "disconnected"},
        {"CONNECTIONEVENT_SOCKET_CONNECTED", 2, "", "socket connected"},
        {"CONNECTIONEVENT_SOCKET_DISCONNECTED", 3, "", "socket disconnected"},
     }
};



///////////////////////////////////

RemoteEntityContainer::RemoteEntityContainer()
    : m_protocolSessionContainer(std::make_unique<ProtocolSessionContainer>())
    , m_executor(m_protocolSessionContainer->getExecutor())
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
    m_name2entity.clear();
    m_entityId2entity.clear();
    m_entitiesChanged.fetch_add(1, std::memory_order_acq_rel);
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


bool RemoteEntityContainer::isTimerExpired(std::chrono::time_point<std::chrono::steady_clock>& lastTime, int interval)
{
    bool expired = false;
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

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

//static const int INTERVAL_CHECK = 5000;


void RemoteEntityContainer::init(const IExecutorPtr& executor, int cycleTime, FuncTimer funcTimer, bool storeRawDataInReceiveStruct, int checkReconnectInterval)
{
    m_storeRawDataInReceiveStruct = storeRawDataInReceiveStruct;
    m_protocolSessionContainer->init(executor, cycleTime, std::move(funcTimer), checkReconnectInterval);
}

static std::string endpointToProtocolEndpoint(const std::string& endpoint, std::string* contentTypeName = nullptr)
{
    size_t ixEndpoint = endpoint.find_last_of(':');
    if (ixEndpoint == std::string::npos)
    {
        return "";
    }
    if (contentTypeName)
    {
        *contentTypeName = endpoint.substr(ixEndpoint + 1, endpoint.size() - (ixEndpoint + 1));
    }
    return endpoint.substr(0, ixEndpoint);
}


int RemoteEntityContainer::bind(const std::string& endpoint, const BindProperties& bindProperties)
{
    std::string contentTypeName;
    const std::string endpointProtocol = endpointToProtocolEndpoint(endpoint, &contentTypeName);
    if (endpointProtocol.empty())
    {
        return -1;
    }
    int contentType = RemoteEntityFormatRegistry::instance().getContentType(contentTypeName);
    if (contentType == 0)
    {
        streamError << "ContentType not found: " << contentTypeName;
        return -1;
    }

    return m_protocolSessionContainer->bind(endpointProtocol, this, bindProperties, contentType);
}

void RemoteEntityContainer::unbind(const std::string& endpoint)
{
    const std::string endpointProtocol = endpointToProtocolEndpoint(endpoint);
    m_protocolSessionContainer->unbind(endpointProtocol);
}

SessionInfo RemoteEntityContainer::connect(const std::string& endpoint, const ConnectProperties& connectProperties)
{
    size_t ixEndpoint = endpoint.find_last_of(':');
    if (ixEndpoint == std::string::npos)
    {
        return {};
    }
    std::string contentTypeName = endpoint.substr(ixEndpoint + 1, endpoint.size() - (ixEndpoint + 1));
    int contentType = RemoteEntityFormatRegistry::instance().getContentType(contentTypeName);
    if (contentType == 0)
    {
        streamError << "ContentType not found: " << contentTypeName;
        return {};
    }
    std::string endpointProtocol = endpoint.substr(0, ixEndpoint);

    IProtocolSessionPtr session = m_protocolSessionContainer->connect(endpointProtocol, this, connectProperties, contentType);
    subscribeEntityNames(session);
    std::weak_ptr<IRemoteEntityContainer> thisEntityContainer = weak_from_this();
    return createSessionInfo(session);
}

SessionInfo RemoteEntityContainer::createSessionInfo(const IProtocolSessionPtr& session)
{
    std::weak_ptr<IRemoteEntityContainer> thisEntityContainer = weak_from_this();
    if (thisEntityContainer.lock())
    {
        return { thisEntityContainer, session };
    }
    else
    {
        return { this, session };
    }
}

void RemoteEntityContainer::subscribeEntityNames(const IProtocolSessionPtr& session)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<std::string> subscribtions;
    subscribtions.reserve(m_name2entity.size());
    for (auto it = m_name2entity.begin(); it != m_name2entity.end(); ++it)
    {
        const std::string& subscribtion = it->first;
        if (subscribtion[subscribtion.size() - 1] != '*')
        {
            subscribtions.push_back(it->first + "/#");
        }
    }
    lock.unlock();

    if (session)
    {
        session->subscribe(subscribtions);
    }
}

void RemoteEntityContainer::subscribeSessions(const std::string& name)
{
    if (name.empty())
    {
        return;
    }
    std::vector< IProtocolSessionPtr > sessions = m_protocolSessionContainer->getAllSessions();
    for (auto it = sessions.begin(); it != sessions.end(); ++it)
    {
        if (*it && (name[name.size() - 1] != '*'))
        {
            (*it)->subscribe({ name + "/#" });
        }
    }
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

IExecutorPtr RemoteEntityContainer::getExecutor() const
{
    return m_executor;
}


EntityId RemoteEntityContainer::registerEntity(hybrid_ptr<IRemoteEntity> remoteEntity, std::string name)
{
    // remove last '/', if available
    if (!name.empty() && name[name.size() - 1] == '/')
    {
        name.resize(name.size() - 1);
    }

    auto re = remoteEntity.lock();
    if (!re)
    {
        streamError << "entity is invalid";
        return ENTITYID_INVALID;
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    if (!name.empty())
    {
        auto it = m_name2entity.find(name);
        if (it != m_name2entity.end())
        {
            streamError << "Entity with name " << name << " was already registered";
            return ENTITYID_INVALID;
        }
    }

    EntityId entityId = re->getEntityId();

    m_entityId2name[entityId] = name;

    if (!name.empty())
    {
        m_name2entity[name] = remoteEntity;
    }

    m_entityId2entity[entityId] = remoteEntity;
    m_entitiesChanged.fetch_add(1, std::memory_order_acq_rel);
    lock.unlock();

    if (!name.empty())
    {
        subscribeSessions(name);
    }

    return entityId;
}


//void RemoteEntityContainer::addPureDataPaths(std::vector<std::string>& paths)
//{
//    for (size_t i = 0; i < paths.size(); ++i)
//    {
//        const std::string& path = paths[i];
//        if (!path.empty())
//        {
//            if (path[path.size() - 1] == '*')
//            {
//                m_pureDataPathPrefixes.emplace_back(path.data(), path.size() - 1);
//            }
//            else
//            {
//                m_pureDataPaths.emplace_back(path);
//            }
//        }
//    }
//}



void RemoteEntityContainer::unregisterEntity(EntityId entityId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_name2entity.begin(); it != m_name2entity.end(); ++it)
    {
        auto remoteEntity = it->second.lock();
        if (!remoteEntity || remoteEntity->getEntityId() == entityId)
        {
            m_name2entity.erase(it);
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
    std::unique_lock<std::mutex> lock(m_mutex);
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
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<EntityId> entities;
    auto it = m_entityId2entity.find(entityId);
    if (it !=  m_entityId2entity.end())
    {
        return it->second;
    }
    return nullptr;
}

std::string RemoteEntityContainer::getEntityName(EntityId entityId, bool& registered) const
{
    registered = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_entityId2name.find(entityId);
    if (it != m_entityId2name.end())
    {
        registered = true;
        return it->second;
    }
    return {};
}



inline void RemoteEntityContainer::triggerConnectionEvent(const SessionInfo& session, ConnectionEvent connectionEvent) const
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
    triggerConnectionEvent(createSessionInfo(session), ConnectionEvent::CONNECTIONEVENT_CONNECTED);
}

void RemoteEntityContainer::disconnected(const IProtocolSessionPtr& session)
{
    triggerConnectionEvent(createSessionInfo(session), ConnectionEvent::CONNECTIONEVENT_DISCONNECTED);

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


void RemoteEntityContainer::disconnectedVirtualSession(const IProtocolSessionPtr& session, const std::string& virtualSessionId)
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
            entity->virtualSessionDisconnected(session, virtualSessionId);
        }
    }
}


//bool RemoteEntityContainer::isPureDataPath(const std::string& path)
//{
//    for (auto it = m_pureDataPathPrefixes.begin(); it != m_pureDataPathPrefixes.end(); ++it)
//    {
//        const std::string& prefix = *it;
//        if (path.size() >= prefix.size() && path.compare(0, prefix.size(), prefix) == 0)
//        {
//            return true;
//        }
//    }
//    for (auto it = m_pureDataPaths.begin(); it != m_pureDataPaths.end(); ++it)
//    {
//        const std::string& pathCompare = *it;
//        if (path == pathCompare)
//        {
//            return true;
//        }
//    }
//    return false;
//}



static const std::string FMQ_PATH = "fmq_path";


struct ThreadLocalDataEntities
{
    std::int64_t                                                changeId = 0;
    std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>  name2entityNoLock;
    std::unordered_map<EntityId, hybrid_ptr<IRemoteEntity>>     entityId2entityNoLock;
};
thread_local ThreadLocalDataEntities t_threadLocalDataEntities;

void RemoteEntityContainer::received(const IProtocolSessionPtr& session, const IMessagePtr& message)
{
    assert(session);
    assert(message);
    
    ThreadLocalDataEntities& threadLocalDataEntities = t_threadLocalDataEntities;
    std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2entityNoLock = threadLocalDataEntities.name2entityNoLock;
    std::unordered_map<EntityId, hybrid_ptr<IRemoteEntity>>& entityId2entityNoLock = threadLocalDataEntities.entityId2entityNoLock;
    std::int64_t changeId = m_entitiesChanged.load(std::memory_order_acquire);
    if (changeId != threadLocalDataEntities.changeId)
    {
        threadLocalDataEntities.changeId = changeId;
        std::unique_lock<std::mutex> lock(m_mutex);
        name2entityNoLock = m_name2entity;
        entityId2entityNoLock = m_entityId2entity;
    }

    //bool pureData = false;
    //if (session->doesSupportMetainfo())
    //{
    //    const std::string* path = message->getMetainfo(FMQ_PATH);
    //    if (path)
    //    {
    //        if (isPureDataPath(*path))
    //        {
    //            pureData = true;
    //        }
    //    }
    //}

    int formatStatus = 0;
    ReceiveData receiveData{ createSessionInfo(session), {}, message, {}, false, {} };
    //if (!pureData)
    //{
        if (!session->doesSupportMetainfo())
        {
            receiveData.structBase = RemoteEntityFormatRegistry::instance().parse(*message, session->getContentType(), m_storeRawDataInReceiveStruct, name2entityNoLock, receiveData.header, formatStatus);
        }
        else
        {
            receiveData.structBase = RemoteEntityFormatRegistry::instance().parseHeaderInMetainfo(*message, session->getContentType(), m_storeRawDataInReceiveStruct, name2entityNoLock, receiveData.header, formatStatus);
        }
    //}
    //else
    //{
    //    receiveData.structBase = RemoteEntityFormatRegistry::instance().parsePureData(*message, receiveData.header);
    //}

    EntityId entityId = receiveData.header.destid;
    bool foundEntity = false;
    hybrid_ptr<IRemoteEntity> remoteEntity;
    if (entityId == ENTITYID_INVALID || (entityId == ENTITYID_DEFAULT && !receiveData.header.destname.empty()))
    {
        auto itName = name2entityNoLock.find(receiveData.header.destname);
        if (itName == name2entityNoLock.end())
        {
            itName = name2entityNoLock.find("*");
        }
        if (itName != name2entityNoLock.end())
        {
            remoteEntity = itName->second;
            foundEntity = true;
        }
    }
    if (!foundEntity && entityId != ENTITYID_INVALID)
    {
        auto it = entityId2entityNoLock.find(entityId);
        if (it != entityId2entityNoLock.end())
        {
            remoteEntity = it->second;
        }
    }

    const std::string& type = receiveData.header.type;

    auto entity = remoteEntity.lock();
    if (receiveData.header.mode == MsgMode::MSG_REQUEST)
    {
        const IMessage::Metainfo& metainfo = receiveData.message->getAllMetainfo();
        if (!metainfo.empty())
        {
            auto it = metainfo.find(FMQ_VIRTUAL_SESSION_ID);
            if (it != metainfo.end())
            {
                receiveData.virtualSessionId = it->second;
            }
        }
        receiveData.automaticConnect = ((formatStatus & FORMATSTATUS_AUTOMATIC_CONNECT) != 0);
        Status replyStatus = Status::STATUS_OK;
        if (!(formatStatus & FORMATSTATUS_SYNTAX_ERROR))
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
            if (entity && (entityId == ENTITYID_INVALID || entityId == ENTITYID_DEFAULT))
            {
                entityId = entity->getEntityId();
            }
            Header headerReply{ receiveData.header.srcid, "", entityId, MsgMode::MSG_REPLY, replyStatus, {}, {}, receiveData.header.corrid, {} };
            RemoteEntityFormatRegistry::instance().send(session, receiveData.virtualSessionId, headerReply, std::move(message->getEchoData()));
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
    triggerConnectionEvent(createSessionInfo(session), ConnectionEvent::CONNECTIONEVENT_SOCKET_CONNECTED);
}

void RemoteEntityContainer::socketDisconnected(const IProtocolSessionPtr& session)
{
    triggerConnectionEvent(createSessionInfo(session), ConnectionEvent::CONNECTIONEVENT_SOCKET_DISCONNECTED);
}



}   // namespace finalmq
