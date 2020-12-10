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

struct IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


namespace remoteentity {
    class Status;
    class Header;
}




using PeerId = std::int64_t;
static constexpr PeerId PEERID_INVALID = 0;



class ReplyContext;
typedef std::unique_ptr<ReplyContext> ReplyContextUPtr;


class SYMBOLEXP PeerEvent
{
public:
    enum Enum : std::int32_t {
        PEER_CONNECTING = 0,
        PEER_CONNECTED = 1,
        PEER_DISCONNECTED = 2
    };

    PeerEvent();
    PeerEvent(Enum en);
    operator const Enum&() const;
    operator Enum&();
    const PeerEvent& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = PEER_CONNECTED;
    static const EnumInfo _enumInfo;
};



typedef std::function<void(PeerId peerId, remoteentity::Status status, const StructBasePtr& structBase)> FuncReply;
typedef std::function<void(ReplyContextUPtr& replyContext, const StructBasePtr& structBase)> FuncCommand;
typedef std::function<void(PeerId peerId, PeerEvent peerEvent, bool incoming)> FuncPeerEvent;
typedef std::function<void(PeerId peerId, remoteentity::Status status)> FuncReplyConnect;

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
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, FuncReplyConnect funcReplyConnect = {}) = 0;
    virtual PeerId connect(const IProtocolSessionPtr& session, EntityId, FuncReplyConnect funcReplyConnect = {}) = 0;
    virtual void disconnect(PeerId peerId) = 0;
    virtual std::vector<PeerId> getAllPeers() const = 0;
    virtual void registerPeerEvent(FuncPeerEvent funcPeerEvent) = 0;

    // low level methods
    virtual void registerCommandFunction(const std::string& functionName, FuncCommand funcCommand) = 0;
    virtual CorrelationId getNextCorrelationId() const = 0;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId) = 0;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) = 0;
    // Can be overriden by the application. Will be called for every reply.
    virtual void replyReceived(CorrelationId correlationId, remoteentity::Status status, const StructBasePtr& structBase) = 0;

private:
    // methods for RemoteEntityContainer
    virtual void initEntity(EntityId entityId, const std::string& entityName) = 0;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) = 0;
    virtual void receivedRequest(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) = 0;
    virtual void receivedReply(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) = 0;
    friend class RemoteEntityContainer;
};
typedef std::shared_ptr<IRemoteEntity> IRemoteEntityPtr;



class PeerManager
{
public:
    PeerManager();
    std::vector<PeerId> getAllPeers() const;
    std::vector<PeerId> getAllPeersWithSession(IProtocolSessionPtr session) const;
    void updatePeer(PeerId peerId, EntityId entityId, const std::string& entityName);
    bool removePeer(PeerId peerId, bool& incoming);
    PeerId getPeerId(std::int64_t sessionId, EntityId entityId, const std::string& entityName) const;
    IProtocolSessionPtr getRequestHeader(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId, remoteentity::Header& header);
    std::string getEntityName(const PeerId& peerId);
    PeerId addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool incoming, bool& added);
    void setEntityId(EntityId entityId);
    void setPeerEvent(const std::shared_ptr<FuncPeerEvent>& funcPeerEvent);

private:
    void removePeerFromSessionEntityToPeerId(std::int64_t sessionId, EntityId entityId, const std::string& entityName);
    PeerId getPeerIdIntern(std::int64_t sessionId, EntityId entityId, const std::string& entityName) const;

    struct Peer
    {
        IProtocolSessionPtr session;
        EntityId            entityId{ENTITYID_INVALID};
        std::string         entityName;
        bool                incoming = false;
    };


    EntityId                            m_entityId{ENTITYID_INVALID};
    std::shared_ptr<FuncPeerEvent>      m_funcPeerEvent;
    std::unordered_map<PeerId, Peer>    m_peers;
    PeerId                              m_nextPeerId{1};
    std::unordered_map<std::uint64_t, std::pair<std::unordered_map<EntityId, PeerId>, std::unordered_map<std::string, PeerId>>> m_sessionEntityToPeerId;
    mutable std::mutex  m_mutex;
};
typedef std::shared_ptr<PeerManager> PeerManagerPtr;



class ReplyContext
{
public:
    inline ReplyContext(const PeerManagerPtr& sessionIdEntityIdToPeerId, const IProtocolSessionPtr& session, EntityId entityIdDest, EntityId entityIdSrc, CorrelationId correlationId)
        : m_peerManager(sessionIdEntityIdToPeerId)
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
            reply(remoteentity::Status::STATUS_NO_REPLY);
        }
    }

    inline PeerId peerId()
    {
        if (m_peerId == PEERID_INVALID)
        {
            assert(m_peerManager);
            assert(m_session);
            // get peer
            m_peerId = m_peerManager->getPeerId(m_session->getSessionId(), m_entityIdDest, "");
        }
        return m_peerId;
    }

    void reply(const StructBase& structBase)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, structBase.getStructInfo().getTypeName(), m_correlationId};
            RemoteEntityFormat::send(m_session, header, structBase);
            m_replySent = true;
        }
    }

private:
    void reply(remoteentity::Status status)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, status, "", m_correlationId};
            RemoteEntityFormat::send(m_session, header);
            m_replySent = true;
        }
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
    PeerManagerPtr                  m_peerManager;
    IProtocolSessionPtr             m_session;
    EntityId                        m_entityIdDest = ENTITYID_INVALID;
    EntityId                        m_entityIdSrc = ENTITYID_INVALID;
    CorrelationId                   m_correlationId = CORRELATIONID_NONE;
    PeerId                          m_peerId = PEERID_INVALID;
    bool                            m_replySent = false;

    friend class RemoteEntity;
};






class SYMBOLEXP RemoteEntity : public IRemoteEntity
{
public:
    RemoteEntity();

public:
    // IRemoteEntity
    virtual bool sendEvent(const PeerId& peerId, const StructBase& structBase) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, FuncReplyConnect funcReplyConnect = {}) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, EntityId, FuncReplyConnect funcReplyConnect = {}) override;
    virtual void disconnect(PeerId peerId) override;
    virtual std::vector<PeerId> getAllPeers() const override;
    virtual void registerPeerEvent(FuncPeerEvent funcPeerEvent) override;
    virtual void registerCommandFunction(const std::string& functionName, FuncCommand funcCommand) override;
    CorrelationId getNextCorrelationId() const override;
    bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId) override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) override;
    virtual void replyReceived(CorrelationId correlationId, remoteentity::Status status, const StructBasePtr& structBase) override;

private:
    virtual void initEntity(EntityId entityId, const std::string& entityName) override;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) override;
    virtual void receivedRequest(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) override;
    virtual void receivedReply(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBasePtr& structBase) override;

    PeerId addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool incoming, bool& added);
    PeerId connectIntern(const IProtocolSessionPtr& session, const std::string& entityName, EntityId, FuncReplyConnect funcReplyConnect);
    void removePeer(PeerId peerId, remoteentity::Status status);

    struct Request
    {
        inline Request(PeerId peerId_, FuncReply&& func_)
            : peerId(peerId_)
            , func(std::move(func_))
        {
        }
        PeerId      peerId = PEERID_INVALID;
        FuncReply   func;
    };

    EntityId                            m_entityId = 0;
    std::string                         m_entityName;

    std::unordered_map<CorrelationId, std::unique_ptr<Request>> m_requests;
    std::unordered_map<std::string, std::shared_ptr<FuncCommand>> m_funcCommands;
    std::shared_ptr<PeerManager>        m_peerManager;
    mutable std::atomic_uint64_t        m_nextCorrelationId{1};
    mutable std::mutex                  m_mutex;
};




}   // namespace finalmq
