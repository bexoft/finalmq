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
#include "remoteentity/entitydata.fmq.h"


#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>


namespace finalmq {

class IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


namespace remoteentity {
    class Status;
    class Header;
}




using PeerId = std::int64_t;
static constexpr PeerId PEERID_INVALID = 0;



class ReplyContext;
typedef std::unique_ptr<ReplyContext> ReplyContextUPtr;


typedef std::function<void(PeerId peerId, remoteentity::Status status, const StructBasePtr& structBase)> FuncReply;
typedef std::function<void(ReplyContextUPtr& replyContext, const StructBasePtr& structBase)> FuncCommand;


struct IRemoteEntity
{
    virtual ~IRemoteEntity() {}

    template<class R>
    bool requestReply(const PeerId& peerId,
                      const StructBase& structBase,
                      std::function<void(PeerId peerId, remoteentity::Status status, const std::shared_ptr<R>& reply)> funcReply)
    {
        assert(funcReply);
        bool ok = sendRequest(peerId, structBase, [funcReply{std::move(funcReply)}] (PeerId peerId, remoteentity::Status status, const StructBasePtr& structBase) {
            std::shared_ptr<R> reply;
            bool typeOk = (!structBase || structBase->getStructInfo().getTypeName() == R::structInfo().getTypeName());
            if (status == remoteentity::Status::STATUS_OK && structBase && typeOk)
            {
                reply = std::static_pointer_cast<R>(structBase);
            }
            if (!typeOk)
            {
                status = remoteentity::Status::STATUS_WRONG_REPLY_TYPE;
            }
            funcReply(peerId, status, reply);
        });
        return ok;
    }

    template<class R>
    void registerCommand(std::function<void(ReplyContextUPtr& replyContext, const std::shared_ptr<R>& request)> funcCommand)
    {
        registerCommandFunction(R::structInfo().getTypeName(), reinterpret_cast<FuncCommand&>(funcCommand));
    }

    virtual bool sendEvent(const PeerId& peerId, const StructBase& structBase) = 0;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName) = 0;
    virtual PeerId connect(const IProtocolSessionPtr& session, EntityId) = 0;
    virtual void disconnect(PeerId peerId) = 0;
    virtual std::vector<PeerId> getAllPeers() const = 0;
    virtual void registerCommandFunction(const std::string& functionName, FuncCommand funcCommand) = 0;

    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) = 0;

    // methods for ReplyContext
    virtual PeerId getPeerId(const IProtocolSessionPtr& session, EntityId entityId) const = 0;

    // methods for RemoteEntityContainer
    virtual void initEntity(EntityId entityId, const std::string& entityName) = 0;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) = 0;
    virtual void receivedRequest(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) = 0;
    virtual void receivedReply(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) = 0;
};
typedef std::shared_ptr<IRemoteEntity> IRemoteEntityPtr;


class ReplyContext
{
public:
    inline ReplyContext(const std::weak_ptr<IRemoteEntity>& entity, const IProtocolSessionPtr& session, EntityId entityIdDest, EntityId entityIdSrc, CorrelationId correlationId)
        : m_entity(entity)
        , m_session(session)
        , m_entityIdDest(entityIdDest)
        , m_entityIdSrc(entityIdSrc)
        , m_correlationId(correlationId)
        , m_replySent(false)
    {
    }

    ~ReplyContext()
    {
        if (!m_replySent && m_correlationId != CORRELATIONID_NONE)
        {
            reply(remoteentity::Status::STATUS_OK);
        }
    }

    inline PeerId peerId()
    {
        if (m_peerId == PEERID_INVALID)
        {
            std::shared_ptr<IRemoteEntity> entity = m_entity.lock();
            if (entity)
            {
                entity->getPeerId(m_session, m_entityIdDest);
            }
        }
        return m_peerId;
    }

    void reply(const StructBase& structBase)
    {
        remoteentity::Header header{m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, structBase.getStructInfo().getTypeName(), m_correlationId};
        RemoteEntityFormat::send(m_session, header, structBase);
        m_replySent = true;
    }

private:
    void reply(remoteentity::Status status)
    {
        remoteentity::Header header{m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, status, "", m_correlationId};
        RemoteEntityFormat::send(m_session, header);
        m_replySent = true;
    }

    inline const IProtocolSessionPtr& session() const
    {
        return m_session;
    }
    inline EntityId entityId() const
    {
        return m_entityIdDest;
    }

    ReplyContext(const ReplyContext&) = delete;
    const ReplyContext& operator =(const ReplyContext&) = delete;
    ReplyContext(const ReplyContext&&) = delete;
    const ReplyContext& operator =(const ReplyContext&&) = delete;
private:
    std::weak_ptr<IRemoteEntity>    m_entity;
    IProtocolSessionPtr             m_session;
    EntityId                        m_entityIdDest = ENTITYID_INVALID;
    EntityId                        m_entityIdSrc = ENTITYID_INVALID;
    CorrelationId                   m_correlationId = CORRELATIONID_NONE;
    PeerId                          m_peerId = PEERID_INVALID;
    bool                            m_replySent = false;

    friend class RemoteEntity;
};



class SessionEntityIdToPeerId
{

};



class RemoteEntity : public IRemoteEntity
                   , private std::enable_shared_from_this<RemoteEntity>
{
public:
    RemoteEntity();

private:
    // IRemoteEntity
    virtual bool sendEvent(const PeerId& peerId, const StructBase& structBase) override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, EntityId) override;
    virtual void disconnect(PeerId peerId) override;
    virtual std::vector<PeerId> getAllPeers() const override;
    virtual PeerId getPeerId(const IProtocolSessionPtr& session, EntityId entityId) const override;
    virtual void registerCommandFunction(const std::string& functionName, FuncCommand funcCommand) override;

    virtual void initEntity(EntityId entityId, const std::string& entityName) override;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) override;
    virtual void receivedRequest(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) override;
    virtual void receivedReply(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) override;

    struct Peer
    {
        IProtocolSessionPtr session;
        EntityId            entityId = ENTITYID_INVALID;
        std::string         entityName;
    };

    PeerId getPeerId(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName) const;
    PeerId addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool& added);
    PeerId connectIntern(const IProtocolSessionPtr& session, const std::string& entityName, EntityId);
    void removePeer(PeerId peerId, remoteentity::Status status);
    CorrelationId getNextCorrelationId() const;
    bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId);
    void triggerReply(CorrelationId correlationId, remoteentity::Status status, const StructBasePtr& structBase);

    struct Request
    {
        PeerId      peerId = PEERID_INVALID;
        FuncReply   func;
    };

    EntityId                            m_entityId = 0;
    std::string                         m_entityName;

    std::unordered_map<PeerId, Peer>    m_peers;
    PeerId                              m_nextPeerId{1};
    std::unordered_map<CorrelationId, std::unique_ptr<Request>> m_requests;
    std::unordered_map<std::uint64_t, std::pair<std::unordered_map<EntityId, PeerId>, std::unordered_map<std::string, PeerId>>> m_sessionEntityToPeerId;
    std::unordered_map<std::string, std::shared_ptr<FuncCommand>> m_funcCommands;
    mutable std::atomic_uint64_t        m_nextCorrelationId{1};
    mutable std::mutex                  m_mutex;
};




}   // namespace finalmq
