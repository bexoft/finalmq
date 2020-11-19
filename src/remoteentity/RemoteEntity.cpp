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
#include "remoteentity/entitydata.fmq.h"

#include <algorithm>

using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;
using finalmq::remoteentity::EntityConnect;
using finalmq::remoteentity::EntityConnectReply;
using finalmq::remoteentity::EntityDisconnect;


namespace finalmq {


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
        Header header = {peer.entityId, peer.entityName, m_entityId, MsgMode::MSG_REQUEST, Status::STATUS_OK, structBase.getStructInfo().getTypeName(), correlationId};
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


bool RemoteEntity::sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply)
{
    CorrelationId correlationId = getNextCorrelationId();
    std::unique_lock<std::mutex> lock(m_mutex);
    m_requests[correlationId] = {peerId, std::make_shared<FuncReply>(std::move(funcReply))};
    lock.unlock();
    bool ok = sendRequest(peerId, structBase, correlationId);
    return ok;
}


void RemoteEntity::triggerReply(CorrelationId correlationId, Status status, const StructBasePtr& structBase)
{
    std::shared_ptr<FuncReply> func;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_requests.find(correlationId);
    if (it != m_requests.end())
    {
        func = it->second.func;
        assert(func);
        m_requests.erase(it);
    }
    lock.unlock();

    if (func)
    {
        (*func)(status, structBase);
    }
}


void RemoteEntity::sendReply(const ReplyContext& replyContext, const StructBase& structBase)
{
    Header header = {replyContext.entityId, "", m_entityId, MsgMode::MSG_REPLY, Status::STATUS_OK, structBase.getStructInfo().getTypeName(), replyContext.correlationId};
    RemoteEntityFormat::send(replyContext.session, header, structBase);
}

void RemoteEntity::sendReply(const ReplyContext& replyContext)
{
    Header header = {replyContext.entityId, "", m_entityId, MsgMode::MSG_REPLY, Status::STATUS_OK, "", replyContext.correlationId};
    RemoteEntityFormat::send(replyContext.session, header);
}


void RemoteEntity::sendReplyError(const ReplyContext& replyContext, remoteentity::Status status)
{
    Header header = {replyContext.entityId, "", m_entityId, MsgMode::MSG_REPLY, status, "", replyContext.correlationId};
    RemoteEntityFormat::send(replyContext.session, header);
}



PeerId RemoteEntity::connect(const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    PeerId peerId = m_nextPeerId;
    ++m_nextPeerId;
    m_peers[peerId] = {session, entityId, entityName};
    lock.unlock();

    sendRequest(peerId, EntityConnect{}, [this, peerId] (remoteentity::Status /*status*/, const StructBasePtr& structBase) {
        if (structBase && (structBase->getStructInfo().getTypeName() == EntityConnectReply::structInfo().getTypeName()))
        {
            const EntityConnectReply& entityConnectReply = static_cast<const EntityConnectReply&>(*structBase);
            std::unique_lock<std::mutex> lock(m_mutex);
            auto it = m_peers.find(peerId);
            if (it != m_peers.end())
            {
                it->second.entityId = entityConnectReply.entityid;
            }
        }
    });

    return peerId;
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
            Peer peer = it->second;
            m_peers.erase(peerId);

            // release pending calls
            std::vector<std::shared_ptr<FuncReply>> funcs;
            funcs.reserve(m_requests.size());
            for (auto it = m_requests.begin(); it != m_requests.end(); )
            {
                if (it->second.peerId == peerId)
                {
                    assert(it->second.func);
                    funcs.push_back(it->second.func);
                    it = m_requests.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            lock.unlock();
            for (size_t i = 0; i < funcs.size(); ++i)
            {
                assert(funcs[i]);
                (*funcs[i])(status, nullptr);
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


void RemoteEntity::initEntity(EntityId entityId)
{
    m_entityId = entityId;
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




std::unordered_map<PeerId, RemoteEntity::Peer>::iterator RemoteEntity::findPeer(const IProtocolSessionPtr& session, EntityId entityId)
{
    for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
    {
        Peer& peer = it->second;
        if (peer.session == session &&
            peer.entityId == entityId)
        {
            return it;
        }
    }
    return m_peers.end();
}



void RemoteEntity::receivedRequest(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase)
{
    assert(structBase);
    if (header.type == EntityConnect::structInfo().getTypeName())
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::unordered_map<PeerId, Peer>::iterator it = findPeer(session, header.srcid);
        if (it == m_peers.end())
        {
            PeerId peerId = m_nextPeerId;
            ++m_nextPeerId;
            Peer& peer = m_peers[peerId];
            peer.entityId = header.srcid;
            peer.session = session;
        }
        lock.unlock();
        sendReply({session, header.srcid, header.corrid}, EntityConnectReply(m_entityId));
    }
    else if (header.type == EntityDisconnect::structInfo().getTypeName())
    {
        PeerId peerId = PEERID_INVALID;
        std::unique_lock<std::mutex> lock(m_mutex);
        std::unordered_map<PeerId, Peer>::iterator it = findPeer(session, header.srcid);
        if (it != m_peers.end())
        {
            peerId = it->first;
        }
        lock.unlock();
        removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
    }
    else
    {

    }
}

void RemoteEntity::receivedReply(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase)
{
    triggerReply(header.corrid, header.status, structBase);

    if (header.status == Status::STATUS_ENTITY_NOT_FOUND)
    {
        PeerId peerId = PEERID_INVALID;
        std::unique_lock<std::mutex> lock(m_mutex);
        std::unordered_map<PeerId, Peer>::iterator it = findPeer(session, header.srcid);
        if (it != m_peers.end())
        {
            peerId = it->first;
        }
        lock.unlock();
        removePeer(peerId, Status::STATUS_PEER_DISCONNECTED);
    }
}

}   // namespace finalmq
