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
using finalmq::remoteentity::ConnectStatus;
using finalmq::remoteentity::Header;
using finalmq::remoteentity::ErrorReply;
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


bool RemoteEntity::request(const PeerId& peerId, CorrelationId correlationId, const StructBase& structBase)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_peers.find(peerId);
    if (it != m_peers.end())
    {
        const auto& peer = it->second;
        IProtocolSessionPtr session = peer.session;
        Header header = {peer.entityId, peer.entityName, m_entityId, MsgMode::MSG_REQUEST, Status::STATUS_OK, structBase.getStructInfo().getTypeName(), correlationId};
        lock.unlock();

        bool ok = RemoteEntityFormat::send(session, header, structBase);
        if (!ok)
        {
            removePeer(peerId);
        }
    }
    return false;
}


void RemoteEntity::reply(const ReplyContext& replyContext, const StructBase& structBase)
{
    Header header = {replyContext.entityId, "", m_entityId, MsgMode::MSG_REPLY, Status::STATUS_OK, structBase.getStructInfo().getTypeName(), replyContext.correlationId};
    RemoteEntityFormat::send(replyContext.session, header, structBase);
}


void RemoteEntity::replyError(const ReplyContext& replyContext, remoteentity::Status status)
{
    Header header = {replyContext.entityId, "", m_entityId, MsgMode::MSG_REPLY, status, ErrorReply::structInfo().getTypeName(), replyContext.correlationId};
    RemoteEntityFormat::send(replyContext.session, header, ErrorReply());
}



PeerId RemoteEntity::connect(const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    PeerId peerId = m_nextPeerId;
    ++m_nextPeerId;
    Peer peer{session, entityId, entityName, getNextCorrelationId()};
    m_peers[peerId] = peer;
    lock.unlock();

    request(peerId, peer.correlationId, EntityConnect{});

    return peerId;
}


void RemoteEntity::disconnect(PeerId peerId)
{
    request(peerId, CORRELATIONID_NONE, EntityDisconnect());
    removePeer(peerId);
}


void RemoteEntity::removePeer(PeerId peerId)
{
    if (peerId != PEERID_INVALID)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_peers.find(peerId);
        if (it != m_peers.end())
        {
            Peer peer = it->second;
            m_peers.erase(peerId);
            lock.unlock();

            // TODO: release pending calls
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
        removePeer(peerIds[i]);
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



void RemoteEntity::received(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBase& structBase)
{
    if (header.mode == MsgMode::MSG_REQUEST)
    {
        if (header.type == EntityConnect::structInfo().getTypeName())
        {
            ConnectStatus connectStatus = ConnectStatus::CONNECT_ALREADY_DONE;
            std::unique_lock<std::mutex> lock(m_mutex);
            std::unordered_map<PeerId, Peer>::iterator it = findPeer(session, header.srcid);
            if (it == m_peers.end())
            {
                PeerId peerId = m_nextPeerId;
                ++m_nextPeerId;
                Peer& peer = m_peers[peerId];
                peer.entityId = header.srcid;
                peer.session = session;
                connectStatus = ConnectStatus::CONNECT_OK;
            }
            lock.unlock();
            reply({session, header.srcid, header.corrid}, EntityConnectReply(connectStatus));
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
            removePeer(peerId);
        }
        else
        {

        }
    }
    else if (header.mode == MsgMode::MSG_REPLY)
    {
        if (header.status == Status::STATUS_OK)
        {
            if (header.type == EntityConnectReply::structInfo().getTypeName())
            {
                const EntityConnectReply& connectReply = static_cast<const EntityConnectReply&>(structBase);
                if (connectReply.status == Status::STATUS_OK && header.corrid != CORRELATIONID_NONE)
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
                    {
                        Peer& peer = it->second;
                        if (peer.correlationId == header.corrid &&
                            peer.entityId == ENTITYID_INVALID &&
                            peer.session == session)
                        {
                            peer.correlationId = CORRELATIONID_NONE;
                            peer.entityId = header.srcid;
                            break;
                        }
                    }
                }
            }
            else
            {

            }
        }
        else
        {
            // status not ok
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
                removePeer(peerId);
            }
        }
    }
}


}   // namespace finalmq
