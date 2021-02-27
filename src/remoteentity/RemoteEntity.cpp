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
#include "finalmq/protocolconnection/ProtocolMessage.h"
#include "finalmq/helpers/ModulenameFinalmq.h"

#include <algorithm>


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;
using finalmq::remoteentity::ConnectEntity;
using finalmq::remoteentity::ConnectEntityReply;
using finalmq::remoteentity::DisconnectEntity;


namespace finalmq {

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
const std::string& PeerEvent::toString() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
void PeerEvent::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo PeerEvent::_enumInfo = {
    "PeerEvent", "", {
        {"PEER_CONNECTING", 0, ""},
        {"PEER_CONNECTED", 1, ""},
        {"PEER_DISCONNECTED", 2, ""},
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
        if (entry.second.session == session)
        {
            peers.push_back(entry.first);
        }
    });
    return peers;
}

void PeerManager::updatePeer(PeerId peerId, EntityId entityId, const std::string& entityName)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        Peer& peer = it->second;
        peer.entityId = entityId;
        peer.entityName = entityName;
        assert(peer.session);
        std::int64_t sessionId = peer.session->getSessionId();
        if (entityId != ENTITYID_INVALID)
        {
            m_sessionEntityToPeerId[sessionId].first[entityId] = peerId;
        }
        if (!entityName.empty())
        {
            m_sessionEntityToPeerId[sessionId].second[entityName] = peerId;
        }

        std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
        lock.unlock();

        // fire peer event CONNECTED
        if (funcPeerEvent && *funcPeerEvent)
        {
            (*funcPeerEvent)(peerId, PeerEvent::PEER_CONNECTED, false);
        }
    }
}


bool PeerManager::removePeer(PeerId peerId, bool& incoming)
{
    bool found = false;
    incoming = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        found = true;
        const Peer& peer = it->second;
        if (peer.session)
        {
            removePeerFromSessionEntityToPeerId(peer.session->getSessionId(), peer.entityId, peer.entityName);
        }
        incoming = peer.incoming;
        m_peers.erase(it);

        std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
        lock.unlock();

        // fire peer event DISCONNECTED
        if (funcPeerEvent && *funcPeerEvent)
        {
            (*funcPeerEvent)(peerId, PeerEvent::PEER_DISCONNECTED, incoming);
        }
    }
    return found;
}


void PeerManager::removePeerFromSessionEntityToPeerId(std::int64_t sessionId, EntityId entityId, const std::string& entityName)
{
    // remove from m_sessionEntityToPeerId
    auto it1 = m_sessionEntityToPeerId.find(sessionId);
    if (it1 != m_sessionEntityToPeerId.end())
    {
        auto& entityIdContainer = it1->second.first;
        auto itA = entityIdContainer.find(entityId);
        if (itA != entityIdContainer.end())
        {
            entityIdContainer.erase(itA);
        }
        auto& entityNameContainer = it1->second.second;
        auto itB = entityNameContainer.find(entityName);
        if (itB != entityNameContainer.end())
        {
            entityNameContainer.erase(itB);
        }

        if (entityIdContainer.empty() && entityNameContainer.empty())
        {
            m_sessionEntityToPeerId.erase(it1);
        }
    }
}

PeerId PeerManager::getPeerId(std::int64_t sessionId, EntityId entityId, const std::string& entityName) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return getPeerIdIntern(sessionId, entityId, entityName);
}



PeerId PeerManager::getPeerIdIntern(std::int64_t sessionId, EntityId entityId, const std::string& entityName) const
{
    auto it = m_sessionEntityToPeerId.find(sessionId);
    if (it != m_sessionEntityToPeerId.end())
    {
        if (entityId != ENTITYID_INVALID)
        {
            const auto& entityIdContainer = it->second.first;
            auto itA = entityIdContainer.find(entityId);
            if (itA != entityIdContainer.end())
            {
                return itA->second;
            }
        }
        else
        {
            const auto& entityNameContainer = it->second.second;
            auto itB = entityNameContainer.find(entityName);
            if (itB != entityNameContainer.end())
            {
                return itB->second;
            }
        }
    }

    return PEERID_INVALID;
}


PeerManager::ReadyToSend PeerManager::getRequestHeader(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId, remoteentity::Header& header, IProtocolSessionPtr& session)
{
    ReadyToSend readyToSend = RTS_PEER_NOT_AVAILABLE;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        auto& peer = it->second;
        if (peer.session)
        {
            readyToSend = RTS_READY;
            session = peer.session;
            const std::string* typeName = structBase.getRawType();
            if (typeName == nullptr)
            {
                typeName = &structBase.getStructInfo().getTypeName();
            }
            assert(typeName);
            header = {peer.entityId, (peer.entityId == ENTITYID_INVALID) ? peer.entityName : std::string(), m_entityId, MsgMode::MSG_REQUEST, Status::STATUS_OK, *typeName, correlationId};
        }
        else
        {
            readyToSend = RTS_SESSION_NOT_AVAILABLE;
            peer.requests.emplace_back(Request{structBase.clone(), correlationId});
        }
    }
    return readyToSend;
}





std::string PeerManager::getEntityName(const PeerId& peerId)
{
    std::string entityName;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        Peer& peer = it->second;
        entityName = peer.entityName;
    }
    lock.unlock();
    return entityName;
}


PeerId PeerManager::addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool incoming, bool& added)
{
//    assert(entityId != ENTITYID_INVALID || !entityName.empty());
    added = false;

    std::unique_lock<std::mutex> lock(m_mutex);

    PeerId peerId = getPeerIdIntern(session->getSessionId(), entityId, entityName);

    if (peerId == PEERID_INVALID)
    {
        peerId = m_nextPeerId;
        ++m_nextPeerId;
        Peer& peer = m_peers[peerId];
        peer.session = session;
        peer.entityId = entityId;
        peer.entityName = entityName;
        peer.incoming = incoming;
        added = true;
        std::int64_t sessionId = session->getSessionId();
        if (entityId != ENTITYID_INVALID)
        {
            m_sessionEntityToPeerId[sessionId].first[entityId] = peerId;
        }
        if (!entityName.empty())
        {
            m_sessionEntityToPeerId[sessionId].second[entityName] = peerId;
        }

        std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
        lock.unlock();

        // fire peer event CONNECTED
        if (funcPeerEvent && *funcPeerEvent)
        {
            (*funcPeerEvent)(peerId, incoming ? PeerEvent::PEER_CONNECTED : PeerEvent::PEER_CONNECTING, incoming);
        }
    }
    return peerId;
}


PeerId PeerManager::addPeer()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    PeerId peerId = m_nextPeerId;
    ++m_nextPeerId;
    m_peers[peerId];

    std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
    lock.unlock();

    // fire peer event CONNECTED
    if (funcPeerEvent && *funcPeerEvent)
    {
        (*funcPeerEvent)(peerId, PeerEvent::PEER_CONNECTING, false);
    }

    return peerId;
}


std::deque<PeerManager::Request> PeerManager::connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName)
{
    std::deque<PeerManager::Request> requests;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        Peer& peer = it->second;
        peer.session = session;
        peer.entityId = entityId;
        peer.entityName = entityName;
        std::int64_t sessionId = session->getSessionId();
        if (entityId != ENTITYID_INVALID)
        {
            m_sessionEntityToPeerId[sessionId].first[entityId] = peerId;
        }
        if (!entityName.empty())
        {
            m_sessionEntityToPeerId[sessionId].second[entityName] = peerId;
        }

        requests = std::move(peer.requests);
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

IProtocolSessionPtr PeerManager::getSession(PeerId peerId) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        const Peer& peer = it->second;
        return peer.session;
    }
    return nullptr;
}





//////////////////////////////////////////////

RemoteEntity::RemoteEntity()
    : m_peerManager(std::make_shared<PeerManager>())
{
    registerCommand<ConnectEntity>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<ConnectEntity>& request) {
        assert(request);
        bool added{};
        addPeer(replyContext->session(), replyContext->entityId(), request->entityName, true, added);
        replyContext->reply(ConnectEntityReply(m_entityId, m_entityName));
    });
    registerCommand<DisconnectEntity>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<DisconnectEntity>& /*request*/) {
        PeerId peerId = replyContext->peerId();
        removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
    });
}



// IRemoteEntity

CorrelationId RemoteEntity::getNextCorrelationId() const
{
    CorrelationId id = m_nextCorrelationId.fetch_add(1);
    return id;
}




bool RemoteEntity::sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId)
{
    bool ok = false;
    Header header;
    IProtocolSessionPtr session;

    // the mutex lock is important for RTS_CONNECT_NOT_AVAILABLE / RTS_READY handling. See connectIntern(PeerId ...)
    std::unique_lock<std::mutex> lock(m_mutex);
    PeerManager::ReadyToSend readyToSend = m_peerManager->getRequestHeader(peerId, structBase, correlationId, header, session);
    lock.unlock();

    if (readyToSend == PeerManager::ReadyToSend::RTS_READY)
    {
        assert(session);
        ok = RemoteEntityFormatRegistry::instance().send(session, header, &structBase);
        if (!ok)
        {
            removePeer(peerId, Status::STATUS_SESSION_DISCONNECTED);
        }
    }
    else if (readyToSend == PeerManager::ReadyToSend::RTS_SESSION_NOT_AVAILABLE)
    {
        ok = true;
    }
    else
    {
        replyReceived(correlationId, Status::STATUS_PEER_DISCONNECTED, nullptr);
    }
    return ok;
}


bool RemoteEntity::sendEvent(const PeerId& peerId, const StructBase& structBase)
{
    bool ok = sendRequest(peerId, structBase, CORRELATIONID_NONE);
    return ok;
}

bool RemoteEntity::sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply)
{
    CorrelationId correlationId = getNextCorrelationId();
    std::unique_lock<std::mutex> lock(m_mutex);
    m_requests.emplace(correlationId, std::make_unique<Request>(peerId, std::make_shared<FuncReply>(std::move(funcReply))));
    lock.unlock();
    bool ok = sendRequest(peerId, structBase, correlationId);
    return ok;
}



void RemoteEntity::replyReceived(CorrelationId correlationId, Status status, const StructBasePtr& structBase)
{
    std::unique_ptr<Request> request;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_requests.find(correlationId);
    if (it != m_requests.end())
    {
        request = std::move(it->second);
        assert(request);
        m_requests.erase(it);
    }
    lock.unlock();

    if (request && request->func && *request->func)
    {
        (*request->func)(request->peerId, status, structBase);
    }
}


void RemoteEntity::registerReplyEvent(FuncReplyEvent funcReplyEvent)
{
    m_funcReplyEvent = std::move(funcReplyEvent);
}





void RemoteEntity::sendConnectEntity(PeerId peerId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect)
{
    requestReply<ConnectEntityReply>(peerId, ConnectEntity{m_entityName},
                                     [this, funcReplyConnect] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<ConnectEntityReply>& replyReceived) {
        if (funcReplyConnect && *funcReplyConnect)
        {
            (*funcReplyConnect)(peerId, status);
        }
        if (replyReceived)
        {
            m_peerManager->updatePeer(peerId, replyReceived->entityid, replyReceived->entityName);
        }
        else if (status == Status::STATUS_ENTITY_NOT_FOUND)
        {
            std::string entityName = m_peerManager->getEntityName(peerId);
            streamError << "Entity not found: " << entityName;
            removePeer(peerId, status);
        }
    });
}

PeerId RemoteEntity::connectIntern(const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect)
{
    bool added{};
    PeerId peerId = addPeer(session, entityId, entityName, false, added);

    if (added)
    {
        sendConnectEntity(peerId, funcReplyConnect);
    }

    return peerId;
}

PeerId RemoteEntity::connect(const IProtocolSessionPtr& session, const std::string& entityName, FuncReplyConnect funcReplyConnect)
{
    return connectIntern(session, entityName, ENTITYID_INVALID, std::make_shared<FuncReplyConnect>(std::move(funcReplyConnect)));
}

PeerId RemoteEntity::connect(const IProtocolSessionPtr& session, EntityId entityId, FuncReplyConnect funcReplyConnect)
{
    return connectIntern(session, "", entityId, std::make_shared<FuncReplyConnect>(std::move(funcReplyConnect)));
}


void RemoteEntity::disconnect(PeerId peerId)
{
    sendRequest(peerId, DisconnectEntity(), CORRELATIONID_NONE);
    removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
}


void RemoteEntity::removePeer(PeerId peerId, remoteentity::Status status)
{
    if (peerId != PEERID_INVALID)
    {
        bool incoming = false;
        bool found = m_peerManager->removePeer(peerId, incoming);

        if (found)
        {
            // get pending calls
            std::unique_lock<std::mutex> lock(m_mutex);
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
            }
        }
    }
}



std::vector<PeerId> RemoteEntity::getAllPeers() const
{
    return m_peerManager->getAllPeers();
}


PeerId RemoteEntity::createPeer(FuncReplyConnect funcReplyConnect)
{
    PeerId peerId = m_peerManager->addPeer();
    sendConnectEntity(peerId, std::make_shared<FuncReplyConnect>(std::move(funcReplyConnect)));
    return peerId;
}


void RemoteEntity::connectIntern(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId)
{
    // the mutex lock is important for RTS_CONNECT_NOT_AVAILABLE / RTS_READY handling. See connectIntern(PeerId ...)
    std::unique_lock<std::mutex> lock(m_mutex);
    std::deque<PeerManager::Request> pendingRequests = m_peerManager->connect(peerId, session, entityId, entityName);

    bool ok = true;
    for (size_t i = 0; i < pendingRequests.size() && ok; ++i)
    {
        const PeerManager::Request request = pendingRequests[i];
        Header header;
        IProtocolSessionPtr sessionRet;
        PeerManager::ReadyToSend readyToSend = PeerManager::ReadyToSend::RTS_PEER_NOT_AVAILABLE;
        assert(request.structBase);
        readyToSend = m_peerManager->getRequestHeader(peerId, *request.structBase, request.correlationId, header, sessionRet);

        if (readyToSend == PeerManager::ReadyToSend::RTS_READY)
        {
            assert(session);
            ok = RemoteEntityFormatRegistry::instance().send(sessionRet, header, request.structBase.get());
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



void RemoteEntity::connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName)
{
    connectIntern(peerId, session, entityName, ENTITYID_INVALID);
}

void RemoteEntity::connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId)
{
    connectIntern(peerId, session, "", entityId);
}

void RemoteEntity::connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId)
{
    connectIntern(peerId, session, entityName, entityId);
}


void RemoteEntity::registerCommandFunction(const std::string& functionName, FuncCommand funcCommand)
{
    std::shared_ptr<FuncCommand> func = std::make_shared<FuncCommand>(std::move(funcCommand));
    std::unique_lock<std::mutex> lock(m_mutex);
    m_funcCommands.emplace(functionName, func);
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


IProtocolSessionPtr RemoteEntity::getSession(PeerId peerId) const
{
    return m_peerManager->getSession(peerId);
}



bool RemoteEntity::isEntityRegistered() const
{
    return (m_entityId != ENTITYID_INVALID);
}



void RemoteEntity::initEntity(EntityId entityId, const std::string& entityName)
{
    m_entityId = entityId;
    m_entityName = entityName;
    m_peerManager->setEntityId(entityId);
}


void RemoteEntity::sessionDisconnected(const IProtocolSessionPtr& session)
{
    std::vector<PeerId> peerIds = m_peerManager->getAllPeersWithSession(session);

    for (size_t i = 0; i < peerIds.size(); ++i)
    {
        removePeer(peerIds[i], Status::STATUS_SESSION_DISCONNECTED);
    }
}






PeerId RemoteEntity::addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool incoming, bool& added)
{
    PeerId peerId = m_peerManager->addPeer(session, entityId, entityName, incoming, added);
    return peerId;
}


void RemoteEntity::receivedRequest(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase)
{
    assert(structBase);

    ReplyContextUPtr replyContext = std::make_unique<ReplyContext>(m_peerManager, session, header.srcid, m_entityId, header.corrid);
    assert(replyContext);

    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_funcCommands.find(header.type);
    if (it == m_funcCommands.end())
    {
        it = m_funcCommands.find("*");
    }
    if (it != m_funcCommands.end())
    {
        std::shared_ptr<FuncCommand> func = it->second;
        lock.unlock();
        assert(func);
        if (*func)
        {
            (*func)(replyContext, structBase);
        }
        else
        {
            replyContext->reply(Status::STATUS_REQUEST_NOT_FOUND);
        }
    }
    else
    {
        replyContext->reply(Status::STATUS_REQUEST_NOT_FOUND);
    }
}

void RemoteEntity::receivedReply(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase)
{
    bool replyHandled = false;
    if (m_funcReplyEvent)
    {
        replyHandled = m_funcReplyEvent(header.corrid, header.status, structBase);
    }
    if (!replyHandled)
    {
        replyReceived(header.corrid, header.status, structBase);
    }

    if (header.status == Status::STATUS_ENTITY_NOT_FOUND &&
        header.srcid != ENTITYID_INVALID)
    {
        assert(m_peerManager);
        PeerId peerId = m_peerManager->getPeerId(session->getSessionId(), header.srcid, "");
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
