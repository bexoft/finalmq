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

#include "remoteentity/RemoteEntity.h"

#include <algorithm>

using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;
using finalmq::remoteentity::EntityConnect;
using finalmq::remoteentity::EntityConnectReply;
using finalmq::remoteentity::EntityDisconnect;


namespace finalmq {



void SessionIdEntityIdToPeerId::updatePeer(std::int64_t sessionId, EntityId entityId, const std::string& entityName, PeerId peerId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_sessionEntityToPeerId[sessionId].first[entityId] = peerId;
    m_sessionEntityToPeerId[sessionId].second[entityName] = peerId;
}

void SessionIdEntityIdToPeerId::removePeer(std::int64_t sessionId, EntityId entityId, const std::string& entityName)
{
    std::unique_lock<std::mutex> lock(m_mutex);
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

PeerId SessionIdEntityIdToPeerId::getPeerId(std::int64_t sessionId, EntityId entityId, const std::string& entityName) const
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



//////////////////////////////////////////////

RemoteEntity::RemoteEntity()
    : m_sessionIdEntityIdToPeerId(std::make_shared<SessionIdEntityIdToPeerId>())
{
    registerCommand<EntityConnect>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<EntityConnect>& request) {
        assert(request);
        bool added{};
        addPeer(replyContext->session(), request->entityid, request->entityName, true, added);
        replyContext->reply(EntityConnectReply(m_entityId, m_entityName));
    });
    registerCommand<EntityDisconnect>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<EntityDisconnect>& /*request*/) {
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
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        const auto& peer = it->second;
        IProtocolSessionPtr session = peer.session;
        Header header{peer.entityId, (peer.entityId == ENTITYID_INVALID) ? peer.entityName : std::string(), m_entityId, MsgMode::MSG_REQUEST, Status::STATUS_OK, structBase.getStructInfo().getTypeName(), correlationId};
        lock.unlock();

        ok = RemoteEntityFormat::send(session, header, structBase);
        if (!ok)
        {
            removePeer(peerId, Status::STATUS_SESSION_DISCONNECTED);
        }
    }
    else
    {
        triggerReply(correlationId, Status::STATUS_PEER_DISCONNECTED, nullptr);
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
    m_requests.emplace(correlationId, std::make_unique<Request>(peerId, std::move(funcReply)));
    lock.unlock();
    bool ok = sendRequest(peerId, structBase, correlationId);
    return ok;
}


void RemoteEntity::triggerReply(CorrelationId correlationId, Status status, const StructBasePtr& structBase)
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

    if (request && request->func)
    {
        request->func(request->peerId, status, structBase);
    }
}



PeerId RemoteEntity::connectIntern(const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId)
{
    bool added{};
    PeerId peerId = addPeer(session, entityId, entityName, false, added);

    if (added)
    {
        requestReply<EntityConnectReply>(peerId, EntityConnect{m_entityId, m_entityName}, [this] (PeerId peerId, remoteentity::Status /*status*/, const std::shared_ptr<EntityConnectReply>& reply) {
            if (reply)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                auto it = m_peers.find(peerId);
                if (it != m_peers.end())
                {
                    Peer& peer = it->second;
                    peer.entityId = reply->entityid;
                    peer.entityName = reply->entityName;
                    assert(m_sessionIdEntityIdToPeerId);
                    m_sessionIdEntityIdToPeerId->updatePeer(peer.session->getSessionId(), peer.entityId, peer.entityName, peerId);
                }
            }
        });
    }

    return peerId;
}

PeerId RemoteEntity::connect(const IProtocolSessionPtr& session, const std::string& entityName)
{
    return connectIntern(session, entityName, ENTITYID_INVALID);
}

PeerId RemoteEntity::connect(const IProtocolSessionPtr& session, EntityId entityId)
{
    return connectIntern(session, "", entityId);
}


void RemoteEntity::disconnect(PeerId peerId)
{
    sendRequest(peerId, EntityDisconnect(), CORRELATIONID_NONE);
    removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
}


void RemoteEntity::removePeer(PeerId peerId, remoteentity::Status status)
{
    if (peerId != PEERID_INVALID)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_peers.find(peerId);
        if (it != m_peers.end())
        {
            const Peer peer = it->second;

            // remove from m_sessionEntityToPeerId
            assert(peer.session);
            assert(m_sessionIdEntityIdToPeerId);
            m_sessionIdEntityIdToPeerId->removePeer(peer.session->getSessionId(), peer.entityId, peer.entityName);
            m_peers.erase(it);

            // get pending calls
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
            std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;
            lock.unlock();

            // release pending calls
            for (size_t i = 0; i < requests.size(); ++i)
            {
                std::unique_ptr<Request>& request = requests[i];
                assert(request);
                if (request->func)
                {
                    request->func(request->peerId, status, nullptr);
                }
            }

            // fire peer event DISCONNECTED
            if (funcPeerEvent && *funcPeerEvent)
            {
                (*funcPeerEvent)(peerId, PeerEvent::PEER_DISCONNECTED, peer.incoming);
            }
        }
    }
}



std::vector<PeerId> RemoteEntity::getAllPeers() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<PeerId> peers;
    peers.reserve(m_peers.size());
    std::for_each(m_peers.begin(), m_peers.end(), [&peers] (const auto& entry) {
        peers.push_back(entry.first);
    });
    return peers;
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
    std::unique_lock<std::mutex> lock(m_mutex);
    m_funcPeerEvent = std::move(func);
}



void RemoteEntity::initEntity(EntityId entityId, const std::string& entityName)
{
    m_entityId = entityId;
    m_entityName = entityName;
}


void RemoteEntity::sessionDisconnected(const IProtocolSessionPtr& session)
{
    std::vector<PeerId> peerIds;
    peerIds.reserve(m_peers.size());
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
    {
        const Peer& peer = it->second;
        if (peer.session == session)
        {
            peerIds.push_back(it->first);
        }
    }
    lock.unlock();

    for (size_t i = 0; i < peerIds.size(); ++i)
    {
        removePeer(peerIds[i], Status::STATUS_SESSION_DISCONNECTED);
    }
}




PeerId RemoteEntity::getPeerId(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName) const
{
    assert(m_sessionIdEntityIdToPeerId);
    return m_sessionIdEntityIdToPeerId->getPeerId(session->getSessionId(), entityId, entityName);
}



PeerId RemoteEntity::addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool incoming, bool& added)
{
    added = false;

    std::unique_lock<std::mutex> lock(m_mutex);

    PeerId peerId = getPeerId(session, entityId, entityName);

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
        assert(m_sessionIdEntityIdToPeerId);
        m_sessionIdEntityIdToPeerId->updatePeer(session->getSessionId(), entityId, entityName, peerId);
        std::shared_ptr<FuncPeerEvent> funcPeerEvent = m_funcPeerEvent;

        lock.unlock();

        // fire peer event CONNECTED
        if (funcPeerEvent && *funcPeerEvent)
        {
            (*funcPeerEvent)(peerId, PeerEvent::PEER_CONNECTED, incoming);
        }
    }

    return peerId;
}


void RemoteEntity::receivedRequest(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase)
{
    assert(structBase);

    ReplyContextUPtr replyContext = std::make_unique<ReplyContext>(m_sessionIdEntityIdToPeerId, session, header.srcid, m_entityId, header.corrid);
    assert(replyContext);

    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_funcCommands.find(header.type);
    if (it != m_funcCommands.end())
    {
        std::shared_ptr<FuncCommand> func = it->second;
        lock.unlock();
        assert(func);
        if (*func)
        {
            (*func)(replyContext, structBase);
        }
    }
    else
    {
        replyContext->reply(Status::STATUS_REQUEST_NOT_FOUND);
    }
}

void RemoteEntity::receivedReply(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase)
{
    triggerReply(header.corrid, header.status, structBase);

    if (header.status == Status::STATUS_ENTITY_NOT_FOUND)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        PeerId peerId = getPeerId(session, header.srcid, "");
        lock.unlock();
        removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
    }
}

}   // namespace finalmq
