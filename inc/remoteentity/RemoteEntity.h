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

#pragma once

#include "protocolconnection/ProtocolSessionContainer.h"
#include "remoteentity/RemoteEntityFormat.h"


#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>


namespace finalmq {

class StructBase;
typedef std::shared_ptr<StructBase> StructBasePtr;
class IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


namespace remoteentity {
    class Status;
    class Header;
}




using PeerId = std::int64_t;
static constexpr PeerId PEERID_INVALID = 0;

using CorrelationId = std::uint64_t;
static constexpr CorrelationId CORRELATIONID_NONE = 0;


struct ReplyContext
{
    IProtocolSessionPtr session;
    EntityId            entityId = ENTITYID_INVALID;
    CorrelationId       correlationId = CORRELATIONID_NONE;
};

typedef std::function<void(remoteentity::Status status, const StructBasePtr& structBase)> FuncReply;

struct IRemoteEntity
{
    virtual ~IRemoteEntity() {}

    virtual CorrelationId getNextCorrelationId() const = 0;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId) = 0;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) = 0;
    virtual void sendReply(const ReplyContext& replyContext, const StructBase& structBase) = 0;
    virtual void sendReplyError(const ReplyContext& replyContext, remoteentity::Status status) = 0;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, EntityId = ENTITYID_INVALID) = 0;
    virtual void disconnect(PeerId peerId) = 0;
    virtual std::vector<PeerId> getAllPeers() const = 0;

    virtual void initEntity(EntityId entityId) = 0;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) = 0;
    virtual void received(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) = 0;


};
typedef std::shared_ptr<IRemoteEntity> IRemoteEntityPtr;





class RemoteEntity : public IRemoteEntity
{
public:

private:
    // IRemoteEntity
    virtual CorrelationId getNextCorrelationId() const override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId) override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) override;
    virtual void sendReply(const ReplyContext& replyContext, const StructBase& structBase) override;
    virtual void sendReplyError(const ReplyContext& replyContext, remoteentity::Status status) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, EntityId = ENTITYID_INVALID) override;
    virtual void disconnect(PeerId peerId) override;
    virtual std::vector<PeerId> getAllPeers() const override;

    virtual void initEntity(EntityId entityId) override;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) override;
    virtual void received(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) override;

    struct Peer
    {
        IProtocolSessionPtr session;
        EntityId            entityId = ENTITYID_INVALID;
        std::string         entityName;
        CorrelationId       correlationId = CORRELATIONID_NONE;
    };

    std::unordered_map<PeerId, Peer>::iterator findPeer(const IProtocolSessionPtr& session, EntityId entityId);
    void removePeer(PeerId peerId);
    void triggerReply(CorrelationId correlationId, remoteentity::Status status, const StructBasePtr& structBase);


    EntityId                            m_entityId = 0;

    std::unordered_map<PeerId, Peer>    m_peers;
    PeerId                              m_nextPeerId{1};
    std::unordered_map<CorrelationId, std::shared_ptr<FuncReply>> m_requests;
    mutable std::atomic_uint64_t        m_nextCorrelationId{1};
    mutable std::mutex                  m_mutex;
};




}   // namespace finalmq
