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

#include "finalmq/remoteentity/RemoteEntity.h"
#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/helpers/Utils.h"

#include <algorithm>


using finalmq::MsgMode;
using finalmq::Status;
using finalmq::Header;
using finalmq::ConnectEntity;
using finalmq::ConnectEntityReply;
using finalmq::DisconnectEntity;


namespace finalmq {

static const std::string FMQ_METHOD = "fmq_method";
static const std::string EMPTY_PATH;


PeerEvent::PeerEvent()
{
}
PeerEvent::PeerEvent(Enum en)
    : m_value(en)
{
}
PeerEvent::operator const Enum&() const
{
    return m_value;
}
PeerEvent::operator Enum&()
{
    return m_value;
}
const PeerEvent& PeerEvent::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& PeerEvent::toName() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
const std::string& PeerEvent::toString() const
{
    return _enumInfo.getMetaEnum().getAliasByValue(m_value);
}
void PeerEvent::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo PeerEvent::_enumInfo = {
    "PeerEvent", "", {
        {"PEER_CONNECTED", 0, "", "connected"},
        {"PEER_DISCONNECTED", 1, "", "disconnected"},
     }
};



///////////////////////////////////


PeerManager::PeerManager()
{
}


std::vector<PeerId> PeerManager::getAllPeers() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<PeerId> peers;
    peers.reserve(m_peers.size());
    std::for_each(m_peers.begin(), m_peers.end(), [&peers] (const auto& entry) {
        peers.push_back(entry.first);
    });
    return peers;
}


std::vector<PeerId> PeerManager::getAllPeersWithSession(IProtocolSessionPtr session) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<PeerId> peers;
    peers.reserve(m_peers.size());
    std::for_each(m_peers.begin(), m_peers.end(), [&peers, &session] (const auto& entry) {
        if (entry.second->session.getSession() == session)
        {
            peers.push_back(entry.first);
        }
    });
    return peers;
}

std::vector<PeerId> PeerManager::getAllPeersWithVirtualSession(IProtocolSessionPtr session, const std::string& virtualSessionId) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<PeerId> peers;
    peers.reserve(m_peers.size());
    std::for_each(m_peers.begin(), m_peers.end(), [&peers, &session, &virtualSessionId](const auto& entry) {
        if (entry.second->session.getSession() == session && entry.second->virtualSessionId == virtualSessionId)
        {
            peers.push_back(entry.first);
        }
    });
    return peers;
}


void PeerManager::updatePeer(PeerId peerId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<PeerManager::Peer> peer = getPeer(peerId);
    if (peer)
    {
        peer->entityId = entityId;
        peer->entityName = entityName;
        assert(peer->session);
        std::int64_t sessionId = peer->session.getSessionId();

        if (peer->virtualSessionId != virtualSessionId)
        {
            assert(peer->virtualSessionId.empty());
            m_sessionEntityToPeerId[sessionId][virtualSessionId] = m_sessionEntityToPeerId[sessionId][peer->virtualSessionId];
            m_sessionEntityToPeerId[sessionId].erase(peer->virtualSessionId);
            peer->virtualSessionId = virtualSessionId;
        }

        if (entityId != ENTITYID_INVALID)
        {
            m_sessionEntityToPeerId[sessionId][peer->virtualSessionId].first[entityId] = peerId;
        }
        if (!entityName.empty())
        {
            m_sessionEntityToPeerId[sessionId][peer->virtualSessionId].second[entityName] = peerId;
        }

        std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
        lock.unlock();

        // fire peer event CONNECTED
        if (funcPeerEvent && *funcPeerEvent)
        {
            (*funcPeerEvent)(peerId, peer->session, entityId, PeerEvent::PEER_CONNECTED, false);
        }
    }
}


bool PeerManager::removePeer(PeerId peerId, bool& incoming)
{
    bool found = false;
    incoming = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<PeerManager::Peer> peer = getPeer(peerId);
    if (peer)
    {
        found = true;
        if (peer->session)
        {
            removePeerFromSessionEntityToPeerId(peer->session.getSessionId(), peer->virtualSessionId, peer->entityId, peer->entityName);
        }
        incoming = peer->incoming;
        m_peers.erase(peerId);

        std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
        lock.unlock();

        // fire peer event DISCONNECTED
        if (funcPeerEvent && *funcPeerEvent)
        {
            (*funcPeerEvent)(peerId, peer->session, peer->entityId, PeerEvent::PEER_DISCONNECTED, incoming);
        }
    }
    return found;
}


void PeerManager::removePeerFromSessionEntityToPeerId(std::int64_t sessionId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName)
{
    // mutex already locked
    // remove from m_sessionEntityToPeerId
    auto it1 = m_sessionEntityToPeerId.find(sessionId);
    if (it1 != m_sessionEntityToPeerId.end())
    {
        auto it2 = it1->second.find(virtualSessionId);
        if (it2 != it1->second.end())
        {
            auto& entityIdContainer = it2->second.first;
            auto itA = entityIdContainer.find(entityId);
            if (itA != entityIdContainer.end())
            {
                entityIdContainer.erase(itA);
            }
            auto& entityNameContainer = it2->second.second;
            auto itB = entityNameContainer.find(entityName);
            if (itB != entityNameContainer.end())
            {
                entityNameContainer.erase(itB);
            }

            if (entityIdContainer.empty() && entityNameContainer.empty())
            {
                it1->second.erase(it2);
            }
        }
        if (it1->second.empty())
        {
            m_sessionEntityToPeerId.erase(it1);
        }
    }
}

PeerId PeerManager::getPeerId(std::int64_t sessionId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return getPeerIdIntern(sessionId, virtualSessionId, entityId, entityName);
}



PeerId PeerManager::getPeerIdIntern(std::int64_t sessionId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName) const
{
    // mutex already locked
    auto it1 = m_sessionEntityToPeerId.find(sessionId);
    if (it1 != m_sessionEntityToPeerId.end())
    {
        auto it2 = it1->second.find(virtualSessionId);
        if (it2 != it1->second.end())
        {
            if (entityId != ENTITYID_INVALID)
            {
                const auto& entityIdContainer = it2->second.first;
                auto itA = entityIdContainer.find(entityId);
                if (itA != entityIdContainer.end())
                {
                    return itA->second;
                }
            }
            else
            {
                const auto& entityNameContainer = it2->second.second;
                auto itB = entityNameContainer.find(entityName);
                if (itB != entityNameContainer.end())
                {
                    return itB->second;
                }
            }
        }
    }

    return PEERID_INVALID;
}



std::shared_ptr<PeerManager::Peer> PeerManager::getPeer(const PeerId& peerId) const
{
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        return it->second;
    }
    return {};
}



PeerManager::ReadyToSend PeerManager::getRequestHeader(const PeerId& peerId, const std::string& path, const StructBase& structBase, CorrelationId correlationId, Header& header, IProtocolSessionPtr& session, std::string& virtualSessionId)
{
    ReadyToSend readyToSend = RTS_PEER_NOT_AVAILABLE;

    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<PeerManager::Peer> peer = getPeer(peerId);
    if (peer)
    {
        if (peer->session)
        {
            session = peer->session.getSession();
            virtualSessionId = peer->virtualSessionId;
            const std::string* typeName = structBase.getRawType();
            if (typeName == nullptr)
            {
                typeName = &structBase.getStructInfo().getTypeName();
            }
            assert(typeName);
            header = { peer->entityId, (peer->entityId == ENTITYID_INVALID) ? peer->entityName : std::string(), m_entityId, MsgMode::MSG_REQUEST, Status::STATUS_OK, path, *typeName, correlationId, {} };
            readyToSend = RTS_READY;
        }
        else
        {
            readyToSend = RTS_SESSION_NOT_AVAILABLE;
            peer->requests.emplace_back(Request{structBase.clone(), correlationId});
        }
    }
    return readyToSend;
}



std::string PeerManager::getEntityName(const PeerId& peerId)
{
    std::string entityName;
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<PeerManager::Peer> peer = getPeer(peerId);
    if (peer)
    {
        entityName = peer->entityName;
    }
    lock.unlock();
    return entityName;
}


PeerId PeerManager::addPeer(const SessionInfo& session, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName, bool incoming, bool& added, const std::function<void()>& funcBeforeFirePeerEvent)
{
//    assert(entityId != ENTITYID_INVALID || !entityName.empty());
    added = false;

    std::unique_lock<std::mutex> lock(m_mutex);

    PeerId peerId = getPeerIdIntern(session.getSessionId(), virtualSessionId, entityId, entityName);

    if (peerId == PEERID_INVALID)
    {
        peerId = m_nextPeerId;
        ++m_nextPeerId;
        std::shared_ptr<Peer> peer = std::make_shared<Peer>();
        peer->session = session;
        peer->virtualSessionId = virtualSessionId;
        peer->entityId = entityId;
        peer->entityName = entityName;
        peer->incoming = incoming;
        m_peers[peerId] = peer;
        added = true;
        std::int64_t sessionId = session.getSessionId();
        if (entityId != ENTITYID_INVALID)
        {
            m_sessionEntityToPeerId[sessionId][peer->virtualSessionId].first[entityId] = peerId;
        }
        if (!entityName.empty())
        {
            m_sessionEntityToPeerId[sessionId][peer->virtualSessionId].second[entityName] = peerId;
        }

        std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
        lock.unlock();

        if (funcBeforeFirePeerEvent)
        {
            funcBeforeFirePeerEvent();
        }

        // fire peer event CONNECTED
        if (incoming)
        {
            if (funcPeerEvent && *funcPeerEvent)
            {
                (*funcPeerEvent)(peerId, session, entityId, PeerEvent::PEER_CONNECTED, incoming);
            }
        }
    }
    else
    {
        lock.unlock();
        if (funcBeforeFirePeerEvent)
        {
            funcBeforeFirePeerEvent();
        }
    }
    return peerId;
}


PeerId PeerManager::addPeer()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    PeerId peerId = m_nextPeerId;
    ++m_nextPeerId;
    std::shared_ptr<Peer> peer = std::make_shared<Peer>();
    m_peers[peerId] = peer;

    //std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
    lock.unlock();

    //// fire peer event CONNECTED
    //if (funcPeerEvent && *funcPeerEvent)
    //{
    //    (*funcPeerEvent)(peerId, PeerEvent::PEER_CONNECTING, false);
    //}

    return peerId;
}


std::deque<PeerManager::Request> PeerManager::connect(PeerId peerId, const SessionInfo& session, EntityId entityId, const std::string& entityName)
{
    std::deque<PeerManager::Request> requests;
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<PeerManager::Peer> peer = getPeer(peerId);
    if (peer)
    {
        peer->session = session;
        peer->entityId = entityId;
        peer->entityName = entityName;
        std::int64_t sessionId = session.getSessionId();
        if (entityId != ENTITYID_INVALID)
        {
            m_sessionEntityToPeerId[sessionId][peer->virtualSessionId].first[entityId] = peerId;
        }
        if (!entityName.empty())
        {
            m_sessionEntityToPeerId[sessionId][peer->virtualSessionId].second[entityName] = peerId;
        }

        requests = std::move(peer->requests);
        peer->requests.clear();
    }

    return requests;
}



void PeerManager::setEntityId(EntityId entityId)
{
    m_entityId = entityId;
}



void PeerManager::setPeerEvent(const std::shared_ptr<FuncPeerEvent>& funcPeerEvent)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_funcPeerEvent = funcPeerEvent;
}

const SessionInfo& PeerManager::getSession(PeerId peerId) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<PeerManager::Peer> peer = getPeer(peerId);
    if (peer)
    {
        return peer->session;
    }
    static SessionInfo sessionEmpty;
    return sessionEmpty;
}





//////////////////////////////////////////////


std::atomic<EntityId> RemoteEntity::m_entityIdNext{0};


RemoteEntity::RemoteEntity()
    : m_entityId(++m_entityIdNext)
    , m_peerManager(std::make_shared<PeerManager>())
{
    m_peerManager->setEntityId(m_entityId);
    registerCommand<ConnectEntity>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<ConnectEntity>& request) {
        assert(request);
        bool added{};
        const std::string& virtualSessionId = requestContext->getVirtualSessionId();
        m_peerManager->addPeer(requestContext->session(), virtualSessionId, requestContext->entityId(), request->entityName, true, added, [this, &requestContext]() {
            // send reply before the connect peer event is triggered. So that the peer gets first the connect reply and afterwards a possible greeting message maybe triggered by the connect peer event.
            // no lock for m_entityId and m_entityName, because they are already set in initEntity, they will not be changed, anymore.
            std::string ownEntityName;
            auto ec = requestContext->session().getEntityContainer().lock();
            if (ec)
            {
                bool registered = false;
                ownEntityName = ec->getEntityName(m_entityId, registered);
                assert(registered);   // entity is already registered, otherwise the request call could not be received
                requestContext->reply(ConnectEntityReply(m_entityId, ownEntityName));
            }
            else
            {
                requestContext->reply(Status::STATUS_ENTITY_NOT_FOUND);
            }
        });
    });
    registerCommand<DisconnectEntity>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<DisconnectEntity>& /*request*/) {
        PeerId peerId = requestContext->peerId();
        removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
    });
}


RemoteEntity::~RemoteEntity()
{
    deinit();
}


// IRemoteEntity

CorrelationId RemoteEntity::getNextCorrelationId() const
{
    CorrelationId id = m_nextCorrelationId.fetch_add(1);
    return id;
}



void RemoteEntity::sendEvent(const PeerId& peerId, const StructBase& structBase)
{
    sendRequest(peerId, EMPTY_PATH, structBase, CORRELATIONID_NONE);
}

void RemoteEntity::sendEvent(const PeerId& peerId, const std::string& path, const StructBase& structBase)
{
    sendRequest(peerId, path, structBase, CORRELATIONID_NONE);
}

void RemoteEntity::sendEventToAllPeers(const StructBase& structBase)
{
    std::vector<PeerId> peers = getAllPeers();
    for (size_t i = 0; i < peers.size(); ++i)
    {
        sendRequest(peers[i], EMPTY_PATH, structBase, CORRELATIONID_NONE);
    }
}

void RemoteEntity::sendEventToAllPeers(const std::string& path, const StructBase& structBase)
{
    std::vector<PeerId> peers = getAllPeers();
    for (size_t i = 0; i < peers.size(); ++i)
    {
        sendRequest(peers[i], path, structBase, CORRELATIONID_NONE);
    }
}


CorrelationId RemoteEntity::sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply)
{
    CorrelationId correlationId = getNextCorrelationId();
    std::unique_lock<std::mutex> lock(m_mutexRequests);
    m_requests.emplace(correlationId, std::make_unique<Request>(peerId, std::make_shared<FuncReply>(std::move(funcReply))));
    lock.unlock();
    sendRequest(peerId, EMPTY_PATH, structBase, correlationId);
    return correlationId;
}


CorrelationId RemoteEntity::sendRequest(const PeerId& peerId, const std::string& path, const StructBase& structBase, FuncReply funcReply)
{
    CorrelationId correlationId = getNextCorrelationId();
    std::unique_lock<std::mutex> lock(m_mutexRequests);
    m_requests.emplace(correlationId, std::make_unique<Request>(peerId, std::make_shared<FuncReply>(std::move(funcReply))));
    lock.unlock();
    sendRequest(peerId, path, structBase, correlationId);
    return correlationId;
}


void RemoteEntity::sendRequest(const PeerId& peerId, const std::string& path, const StructBase& structBase, CorrelationId correlationId, IMessage::Metainfo* metainfo)
{
    //// if not initialized (entity not registered)
    //if (!m_initialized.load(std::memory_order_acquire))
    //{
    //    streamError << "entity not registered. before you send data, you must register the entity to a RemoteEntityContainer";
    //    throw std::logic_error("entity not registered. before you send data, you must register the entity to a RemoteEntityContainer");
    //}

    Status status = Status::STATUS_OK;
    Header header;
    IProtocolSessionPtr session;
    std::string virtualSessionId;

    // the mutex lock is important for RTS_CONNECT_NOT_AVAILABLE / RTS_READY handling. See connectIntern(PeerId ...)
    std::unique_lock<std::mutex> lock(m_mutex);
    PeerManager::ReadyToSend readyToSend = m_peerManager->getRequestHeader(peerId, path, structBase, correlationId, header, session, virtualSessionId);
    lock.unlock();

    if (readyToSend == PeerManager::ReadyToSend::RTS_READY)
    {
        assert(session);
        RemoteEntityFormatRegistry::instance().send(session, virtualSessionId, header, {}, &structBase, metainfo);
    }
    else if (readyToSend == PeerManager::ReadyToSend::RTS_SESSION_NOT_AVAILABLE)
    {
        // message was stored in getRequestHeader
    }
    else
    {
        status = Status::STATUS_PEER_DISCONNECTED;
    }

    if (status != Status::STATUS_OK)
    {
        ReceiveData receiveData;
        receiveData.header.corrid = correlationId;
        receiveData.header.status = status;
        receiveData.message = std::make_shared<ProtocolMessage>(0);
        if (session != nullptr)
        {
            IExecutorPtr executor = session->getExecutor();
            assert(executor);
            executor->addAction([this, receiveData]()
                {
                    receivedReply(receiveData);
                });
        }
        else
        {
            receivedReply(receiveData);
        }
    }
}


void RemoteEntity::sendEvent(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase)
{
    sendRequest(peerId, EMPTY_PATH, structBase, CORRELATIONID_NONE, &metainfo);
}

void RemoteEntity::sendEvent(const PeerId& peerId, const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase)
{
    sendRequest(peerId, path, structBase, CORRELATIONID_NONE, &metainfo);
}

void RemoteEntity::sendEventToAllPeers(IMessage::Metainfo&& metainfo, const StructBase& structBase)
{
    std::vector<PeerId> peers = getAllPeers();
    for (size_t i = 0; i < peers.size(); ++i)
    {
        sendRequest(peers[i], EMPTY_PATH, structBase, CORRELATIONID_NONE, &metainfo);
    }
}

void RemoteEntity::sendEventToAllPeers(const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase)
{
    std::vector<PeerId> peers = getAllPeers();
    for (size_t i = 0; i < peers.size(); ++i)
    {
        sendRequest(peers[i], path, structBase, CORRELATIONID_NONE, &metainfo);
    }
}


CorrelationId RemoteEntity::sendRequest(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply)
{
    CorrelationId correlationId = getNextCorrelationId();
    std::unique_lock<std::mutex> lock(m_mutexRequests);
    m_requests.emplace(correlationId, std::make_unique<Request>(peerId, std::make_shared<FuncReplyMeta>(std::move(funcReply))));
    lock.unlock();
    sendRequest(peerId, EMPTY_PATH, structBase, correlationId, &metainfo);
    return correlationId;
}


CorrelationId RemoteEntity::sendRequest(const PeerId& peerId, const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply)
{
    CorrelationId correlationId = getNextCorrelationId();
    std::unique_lock<std::mutex> lock(m_mutexRequests);
    m_requests.emplace(correlationId, std::make_unique<Request>(peerId, std::make_shared<FuncReplyMeta>(std::move(funcReply))));
    lock.unlock();
    sendRequest(peerId, path, structBase, correlationId, &metainfo);
    return correlationId;
}


bool RemoteEntity::cancelReply(CorrelationId correlationId)
{
    if (correlationId == CORRELATIONID_NONE)
    {
        return false;
    }
    std::unique_lock<std::mutex> lock(m_mutexRequests);
    auto it = m_requests.find(correlationId);
    if (it != m_requests.end())
    {
        m_requests.erase(it);
        return true;
    }
    else
    {
        return false;
    }
}


void RemoteEntity::replyReceived(const ReceiveData& receiveData)
{
    std::unique_ptr<Request> request;
    std::unique_lock<std::mutex> lock(m_mutexRequests);
    auto it = m_requests.find(receiveData.header.corrid);
    if (it != m_requests.end())
    {
        request = std::move(it->second);
        assert(request);
        m_requests.erase(it);
    }
    lock.unlock();

    if (request)
    {
        if (request->func && *request->func)
        {
            (*request->func)(request->peerId, receiveData.header.status, receiveData.structBase);
        }
        else if (request->funcMeta && *request->funcMeta)
        {
            (*request->funcMeta)(request->peerId, receiveData.header.status, receiveData.message->getAllMetainfo(), receiveData.structBase);
        }
    }
}


void RemoteEntity::registerReplyEvent(FuncReplyEvent funcReplyEvent)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_funcsReplyEvent.emplace_back(std::move(funcReplyEvent));
    m_funcsReplyEventChanged.fetch_add(1, std::memory_order_acq_rel);
}



PeerId RemoteEntity::createPublishPeer(const SessionInfo& session, const std::string& entityName)
{
    bool added = false;
    PeerId peerId = PEERID_INVALID;
    peerId = m_peerManager->addPeer(session, "", ENTITYID_INVALID, entityName, true, added, {});
    return peerId;
}




void RemoteEntity::sendConnectEntity(PeerId peerId, IRemoteEntityContainer& entityContainer, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect)
{
    bool registered = false;
    std::string ownEntityName = entityContainer.getEntityName(m_entityId, registered);

    if (!registered)
    {
        // entity not registered, yet
        std::weak_ptr<RemoteEntity> thisEntityWeak = weak_from_this();
        hybrid_ptr<IRemoteEntity> thisEntity;
        if (thisEntity.lock())
        {
            thisEntity = thisEntityWeak;
        }
        else
        {
            thisEntity = this;
        }
        entityContainer.registerEntity(thisEntity);
    }

    requestReply<ConnectEntityReply>(peerId, {}, ConnectEntity{ ownEntityName },
                                     [this, funcReplyConnect] (PeerId peerId, Status status,
                                         IMessage::Metainfo& metainfo, const std::shared_ptr<ConnectEntityReply>& replyReceived) {
        if (funcReplyConnect && *funcReplyConnect)
        {
            (*funcReplyConnect)(peerId, status);
        }
        if (replyReceived)
        {
            const std::string& virtualSessionId = metainfo[FMQ_VIRTUAL_SESSION_ID];
            m_peerManager->updatePeer(peerId, virtualSessionId, replyReceived->entityId, replyReceived->entityName);
        }
        else if (status == Status::STATUS_ENTITY_NOT_FOUND)
        {
            std::string entityName = m_peerManager->getEntityName(peerId);
            streamError << "Entity not found: " << entityName;
            removePeer(peerId, status);
        }
    });
}

PeerId RemoteEntity::connectIntern(const SessionInfo& session, const std::string& virtualSessionId, const std::string& entityName, EntityId entityId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect)
{
    bool added{};
    PeerId peerId = m_peerManager->addPeer(session, virtualSessionId, entityId, entityName, false, added, {});
    if (added)
    {
        auto ec = session.getEntityContainer().lock();
        if (ec)
        {
            sendConnectEntity(peerId, *ec, funcReplyConnect);
        }
        else
        {
            if (funcReplyConnect && *funcReplyConnect)
            {
                (*funcReplyConnect)(peerId, Status::STATUS_PEER_DISCONNECTED);
            }
        }
    }

    return peerId;
}

PeerId RemoteEntity::connect(const SessionInfo& session, const std::string& entityName, FuncReplyConnect funcReplyConnect)
{
    return connectIntern(session, "", entityName, ENTITYID_INVALID, std::make_shared<FuncReplyConnect>(std::move(funcReplyConnect)));
}

PeerId RemoteEntity::connect(const SessionInfo& session, EntityId entityId, FuncReplyConnect funcReplyConnect)
{
    return connectIntern(session, "", "", entityId, std::make_shared<FuncReplyConnect>(std::move(funcReplyConnect)));
}


void RemoteEntity::disconnect(PeerId peerId)
{
    sendRequest(peerId, EMPTY_PATH, DisconnectEntity(), CORRELATIONID_NONE);
    removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
}


void RemoteEntity::removePeer(PeerId peerId, Status status)
{
    if (peerId != PEERID_INVALID)
    {
        bool incoming = false;
        bool found = m_peerManager->removePeer(peerId, incoming);

        if (found)
        {
            // get pending calls
            std::unique_lock<std::mutex> lock(m_mutexRequests);
            std::vector<std::unique_ptr<Request>> requests;
            requests.reserve(m_requests.size());
            for (auto it = m_requests.begin(); it != m_requests.end(); )
            {
                if (it->second->peerId == peerId)
                {
                    requests.push_back(std::move(it->second));
                    it = m_requests.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            lock.unlock();

            // release pending calls
            for (size_t i = 0; i < requests.size(); ++i)
            {
                std::unique_ptr<Request>& request = requests[i];
                assert(request);
                if (request->func && *request->func)
                {
                    (*request->func)(request->peerId, status, nullptr);
                }
                else if (request->funcMeta && *request->funcMeta)
                {
                    IMessage::Metainfo metainfoEmty;
                    (*request->funcMeta)(request->peerId, status, metainfoEmty, nullptr);
                }
            }
        }
    }
}



std::vector<PeerId> RemoteEntity::getAllPeers() const
{
    return m_peerManager->getAllPeers();
}


PeerId RemoteEntity::createPeer(IRemoteEntityContainer& entityContainer, FuncReplyConnect funcReplyConnect)
{
    PeerId peerId = m_peerManager->addPeer();
    sendConnectEntity(peerId, entityContainer, std::make_shared<FuncReplyConnect>(std::move(funcReplyConnect)));
    return peerId;
}



void RemoteEntity::connectIntern(PeerId peerId, const SessionInfo& session, const std::string& entityName, EntityId entityId)
{
    // the mutex lock is important for RTS_CONNECT_NOT_AVAILABLE / RTS_READY handling. See connectIntern(PeerId ...)
    std::unique_lock<std::mutex> lock(m_mutex);
    std::deque<PeerManager::Request> pendingRequests = m_peerManager->connect(peerId, session, entityId, entityName);

    bool ok = true;
    for (size_t i = 0; i < pendingRequests.size() && ok; ++i)
    {
        const PeerManager::Request& request = pendingRequests[i];
        Header header;
        IProtocolSessionPtr sessionRet;
        std::string virtualSessionIdRet;
        assert(request.structBase);
        PeerManager::ReadyToSend readyToSend = m_peerManager->getRequestHeader(peerId, EMPTY_PATH, *request.structBase, request.correlationId, header, sessionRet, virtualSessionIdRet);

        if (readyToSend == PeerManager::ReadyToSend::RTS_READY)
        {
            assert(session);
            RemoteEntityFormatRegistry::instance().send(sessionRet, virtualSessionIdRet, header, {}, request.structBase.get());
            ok = true;
        }
        else
        {
            ok = false;
        }
    }
    lock.unlock();

    if (!ok)
    {
        removePeer(peerId, Status::STATUS_SESSION_DISCONNECTED);
    }
}



void RemoteEntity::connect(PeerId peerId, const SessionInfo& session, const std::string& entityName)
{
    connectIntern(peerId, session, entityName, ENTITYID_INVALID);
}

void RemoteEntity::connect(PeerId peerId, const SessionInfo& session, EntityId entityId)
{
    connectIntern(peerId, session, "", entityId);
}

void RemoteEntity::connect(PeerId peerId, const SessionInfo& session, const std::string& entityName, EntityId entityId)
{
    connectIntern(peerId, session, entityName, entityId);
}


void RemoteEntity::registerCommandFunction(const std::string& path, const std::string& type, FuncCommand funcCommand)
{
    std::shared_ptr<FuncCommand> func = std::make_shared<FuncCommand>(std::move(funcCommand));
    std::unique_lock<std::mutex> lock(m_mutexFunctions);
    if (path.find('{') != std::string::npos)
    {
        FunctionVar funcVar;
        funcVar.type = type;
        funcVar.func = func;
        Utils::split(path, 0, path.size(), '/', funcVar.pathEntries);
        m_funcCommandsVar.emplace_back(std::move(funcVar));
    }
    else if (path.find('*') != std::string::npos)
    {
        FunctionVar funcVar;
        funcVar.type = type;
        funcVar.func = func;
        Utils::split(path, 0, path.size(), '/', funcVar.pathEntries);
        m_funcCommandsVarStar.emplace_back(std::move(funcVar));
    }
    else
    {
        m_funcCommandsStatic[path] = { type, func };
    }
}


std::string RemoteEntity::getTypeOfCommandFunction(std::string& path, const std::string* method)
{
    const RemoteEntity::Function* function = getFunction(path);
    if (!function && method)
    {
        std::string pathWithMethod = path;
        pathWithMethod += '/';
        pathWithMethod += *method;
        function = getFunction(pathWithMethod);
        if (function)
        {
            path = std::move(pathWithMethod);
        }
    }
    if (function)
    {
        return function->type;
    }
    return {};
}


void RemoteEntity::registerPeerEvent(FuncPeerEvent funcPeerEvent)
{
    std::shared_ptr<FuncPeerEvent> func = std::make_shared<FuncPeerEvent>(std::move(funcPeerEvent));
    m_peerManager->setPeerEvent(func);
}


EntityId RemoteEntity::getEntityId() const
{
    return m_entityId;
}


SessionInfo RemoteEntity::getSession(PeerId peerId) const
{
    return m_peerManager->getSession(peerId);
}


void RemoteEntity::sessionDisconnected(const IProtocolSessionPtr& session)
{
    std::vector<PeerId> peerIds = m_peerManager->getAllPeersWithSession(session);

    for (size_t i = 0; i < peerIds.size(); ++i)
    {
        removePeer(peerIds[i], Status::STATUS_SESSION_DISCONNECTED);
    }
}

void RemoteEntity::virtualSessionDisconnected(const IProtocolSessionPtr& session, const std::string& virtualSessionId)
{
    std::vector<PeerId> peerIds = m_peerManager->getAllPeersWithVirtualSession(session, virtualSessionId);

    for (size_t i = 0; i < peerIds.size(); ++i)
    {
        removePeer(peerIds[i], Status::STATUS_SESSION_DISCONNECTED);
    }
}


const RemoteEntity::Function* RemoteEntity::getFunction(const std::string& path, IMessage::Metainfo* keys) const
{
    std::unique_lock<std::mutex> lock(m_mutexFunctions);

    auto it1 = m_funcCommandsStatic.find(path);
    if (it1 != m_funcCommandsStatic.end())
    {
        return &it1->second;
    }

    std::vector<std::string> pathEntries;
    Utils::split(path, 0, path.size(), '/', pathEntries);

    if (!m_funcCommandsVar.empty())
    {
        for (auto it2 = m_funcCommandsVar.begin(); it2 != m_funcCommandsVar.end(); ++it2)
        {
            const FunctionVar& funcVar = *it2;
            if (funcVar.pathEntries.size() == pathEntries.size())
            {
                bool match = true;
                for (size_t i = 0; i < pathEntries.size() && match; ++i)
                {
                    const std::string& funcVarEntry = funcVar.pathEntries[i];
                    if (funcVarEntry.size() >= 2 && funcVarEntry[0] == '{')
                    {
                        if (keys && funcVarEntry.size() >= 3)
                        {
                            static const std::string PATH_PREFIX = "PATH_";
                            std::string key = PATH_PREFIX;
                            key.insert(key.end(), funcVarEntry.data() + 1, funcVarEntry.data() + funcVarEntry.size() - 1);
                            (*keys)[std::move(key)] = pathEntries[i];
                        }
                    }
                    else
                    {
                        if (funcVarEntry != pathEntries[i])
                        {
                            match = false;
                        }
                    }
                }
                if (match)
                {
                    return &funcVar;
                }
            }
        }
    }

    if (!m_funcCommandsVarStar.empty())
    {
        for (auto it2 = m_funcCommandsVarStar.begin(); it2 != m_funcCommandsVarStar.end(); ++it2)
        {
            const FunctionVar& funcVar = *it2;
            bool match = true;
            size_t j = 0;
            for (size_t i = 0; i < funcVar.pathEntries.size() && match; ++i)
            {
                const std::string& funcVarEntry = funcVar.pathEntries[i];
                if (funcVarEntry[0] == '*')
                {
                    std::string nextEntry;
                    ++i;
                    bool nextEntryAvailable = (i < funcVar.pathEntries.size());
                    if (nextEntryAvailable)
                    {
                        nextEntry = funcVar.pathEntries[i];
                    }
                    std::string value;
                    match = false;
                    for (; j < pathEntries.size() && !match; ++j)
                    {
                        bool matchNextEntry = (nextEntryAvailable && pathEntries[j] == nextEntry);
                        bool matchLastEntry = (!nextEntryAvailable && (j == pathEntries.size() - 1));

                        if (!matchNextEntry)
                        {
                            if (!value.empty())
                            {
                                value += '/';
                            }
                            value += pathEntries[j];
                        }

                        if (matchNextEntry || matchLastEntry)
                        {
                            if (keys && funcVarEntry.size() >= 3)
                            {
                                static const std::string PATH_PREFIX = "PATH_";
                                std::string key = PATH_PREFIX;
                                key.insert(key.end(), funcVarEntry.data() + 1, funcVarEntry.data() + funcVarEntry.size() - 1);
                                (*keys)[std::move(key)] = std::move(value);
                            }
                            match = true;
                        }
                    }
                }
                else
                {
                    if (funcVarEntry != pathEntries[j])
                    {
                        match = false;
                    }
                    ++j;
                }
            }
            if (match && j == pathEntries.size())
            {
                return &funcVar;
            }
        }
    }

    return nullptr;
}



void RemoteEntity::receivedRequest(ReceiveData& receiveData)
{
    if (receiveData.automaticConnect)
    {
        static std::string STR_DUMMY = "dummy";
        bool added{};
        std::uint64_t srcid = receiveData.header.srcid;
        if (srcid == 0)
        {
            srcid = ENTITYID_INVALID;
        }
        m_peerManager->addPeer(receiveData.session, receiveData.virtualSessionId, srcid, STR_DUMMY, true, added, nullptr);
    }

    std::shared_ptr<FuncCommand> func;
    const RemoteEntity::Function* funcData = getFunction(receiveData.header.path, &receiveData.message->getAllMetainfo());
    if (funcData)
    {
        func = funcData->func;
        assert(func);
    }

    RequestContextPtr requestContext = std::make_shared<RequestContext>(m_peerManager, m_entityId, receiveData);
    assert(requestContext);

    if (func && *func)
    {
        (*func)(requestContext, receiveData.structBase);
    }
    else
    {
        requestContext->reply(Status::STATUS_REQUEST_NOT_FOUND);
    }
}


struct ThreadLocalDataReplyEvent
{
    std::int64_t                changeId = 0;
    std::vector<FuncReplyEvent> funcsReplyEventNoLock;
};
thread_local ThreadLocalDataReplyEvent t_threadLocalDataReplyEvent;


void RemoteEntity::receivedReply(const ReceiveData& receiveData)
{
    ThreadLocalDataReplyEvent& threadLocalDataReplyEvent = t_threadLocalDataReplyEvent;
    std::vector<FuncReplyEvent>& funcsReplyEventNoLock = threadLocalDataReplyEvent.funcsReplyEventNoLock;
    std::int64_t changeId = m_funcsReplyEventChanged.load(std::memory_order_acquire);
    if (changeId != threadLocalDataReplyEvent.changeId)
    {
        threadLocalDataReplyEvent.changeId = changeId;
        std::unique_lock<std::mutex> lock(m_mutex);
        funcsReplyEventNoLock = m_funcsReplyEvent;
    }

    bool replyHandled = false;
    for (size_t i = 0; i < funcsReplyEventNoLock.size(); ++i)
    {
        replyHandled = funcsReplyEventNoLock[i](receiveData.header.corrid, receiveData.header.status, receiveData.message->getAllMetainfo(), receiveData.structBase);
        if (replyHandled)
        {
            break;
        }
    }
    if (!replyHandled)
    {
        replyReceived(receiveData);
    }

    if (receiveData.header.status == Status::STATUS_ENTITY_NOT_FOUND &&
        receiveData.header.srcid != ENTITYID_INVALID)
    {
        assert(m_peerManager);
        assert(receiveData.session);
        const std::string& virtualSessionId = receiveData.message->getAllMetainfo()[FMQ_VIRTUAL_SESSION_ID];
        PeerId peerId = m_peerManager->getPeerId(receiveData.session.getSessionId(), virtualSessionId, receiveData.header.srcid, "");
        removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
    }
}


void RemoteEntity::deinit()
{
    std::vector<PeerId> peers = getAllPeers();
    for (size_t i = 0; i < peers.size(); ++i)
    {
        removePeer(peers[i], Status::STATUS_PEER_DISCONNECTED);
    }
}



}   // namespace finalmq
