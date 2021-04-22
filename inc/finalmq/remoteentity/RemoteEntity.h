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

#include "finalmq/protocolconnection/ProtocolSessionContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatRegistry.h"
#include "finalmq/remoteentity/entitydata.fmq.h"


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
typedef std::function<void(PeerId peerId, remoteentity::Status status, std::vector<std::string>& metainfo, const StructBasePtr& structBase)> FuncReplyMeta;
typedef std::function<void(ReplyContextUPtr& replyContext, const StructBasePtr& structBase)> FuncCommand;
typedef std::function<void(PeerId peerId, PeerEvent peerEvent, bool incoming)> FuncPeerEvent;
typedef std::function<bool(CorrelationId correlationId, remoteentity::Status status, std::vector<std::string>& metainfo, const StructBasePtr& structBase)> FuncReplyEvent; // return bool reply handled -> skip looking for reply lambda.
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
        bool ok = sendRequest(peerId, structBase, [funcReply{ std::move(funcReply) }](PeerId peerId, remoteentity::Status status, const StructBasePtr& structBase) {
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
    bool requestReply(const PeerId& peerId,
        std::vector<std::string>&& metainfo,
        const StructBase& structBase,
        std::function<void(PeerId peerId, remoteentity::Status status, std::vector<std::string>& metainfo, const std::shared_ptr<R>& reply)> funcReply)
    {
        assert(funcReply);
        bool ok = sendRequest(peerId, std::move(metainfo), structBase, [funcReplyMeta{ std::move(funcReply) }](PeerId peerId, remoteentity::Status status, std::vector<std::string>& metainfo, const StructBasePtr& structBase) {
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
            funcReply(peerId, status, metainfo, reply);
        });
        return ok;
    }

    template<class R>
    void registerCommand(std::function<void(ReplyContextUPtr& replyContext, const std::shared_ptr<R>& request)> funcCommand)
    {
        registerCommandFunction(R::structInfo().getTypeName(), reinterpret_cast<FuncCommand&>(funcCommand));
    }

    virtual bool sendEvent(const PeerId& peerId, const StructBase& structBase) = 0;
    virtual bool sendEvent(const PeerId& peerId, std::vector<std::string>&& metainfo, const StructBase& structBase) = 0;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, FuncReplyConnect funcReplyConnect = {}) = 0;
    virtual PeerId connect(const IProtocolSessionPtr& session, EntityId entityId, FuncReplyConnect funcReplyConnect = {}) = 0;
    virtual void disconnect(PeerId peerId) = 0;
    virtual std::vector<PeerId> getAllPeers() const = 0;
    virtual void registerPeerEvent(FuncPeerEvent funcPeerEvent) = 0;
    virtual EntityId getEntityId() const = 0;
    virtual IProtocolSessionPtr getSession(PeerId peerId) const = 0;

    // low level methods
    virtual PeerId createPeer(FuncReplyConnect funcReplyConnect = {}) = 0;
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName) = 0;
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId) = 0;
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId) = 0;
    virtual void registerCommandFunction(const std::string& functionName, FuncCommand funcCommand) = 0;
    virtual CorrelationId getNextCorrelationId() const = 0;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId) = 0;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) = 0;
    virtual bool sendRequest(const PeerId& peerId, std::vector<std::string>&& metainfo, const StructBase& structBase, CorrelationId correlationId) = 0;
    virtual bool sendRequest(const PeerId& peerId, std::vector<std::string>&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) = 0;
    virtual bool isEntityRegistered() const = 0;
    // A callback for every received reply. With this callback a match with the correlation ID can be done by the application.
    virtual void registerReplyEvent(FuncReplyEvent funcReplyEvent) = 0;

private:
    // methods for RemoteEntityContainer
    virtual void initEntity(EntityId entityId, const std::string& entityName) = 0;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) = 0;
    virtual void receivedRequest(const IProtocolSessionPtr& session, remoteentity::Header& header, const StructBasePtr& structBase) = 0;
    virtual void receivedReply(const IProtocolSessionPtr& session, remoteentity::Header& header, const StructBasePtr& structBase) = 0;
    virtual void deinit() = 0;
    friend class RemoteEntityContainer;
};
typedef std::shared_ptr<IRemoteEntity> IRemoteEntityPtr;



class SYMBOLEXP PeerManager
{
public:

    enum ReadyToSend
    {
        RTS_READY,
        RTS_SESSION_NOT_AVAILABLE,
        RTS_PEER_NOT_AVAILABLE
    };

    struct Request
    {
        StructBasePtr   structBase;
        CorrelationId   correlationId = CORRELATIONID_NONE;
    };

    PeerManager();
    std::vector<PeerId> getAllPeers() const;
    std::vector<PeerId> getAllPeersWithSession(IProtocolSessionPtr session) const;
    void updatePeer(PeerId peerId, EntityId entityId, const std::string& entityName);
    bool removePeer(PeerId peerId, bool& incoming);
    PeerId getPeerId(std::int64_t sessionId, EntityId entityId, const std::string& entityName) const;
    ReadyToSend getRequestHeader(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId, remoteentity::Header& header, IProtocolSessionPtr& session);
    std::string getEntityName(const PeerId& peerId);
    PeerId addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool incoming, bool& added);
    PeerId addPeer();
    std::deque<PeerManager::Request> connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName);
    void setEntityId(EntityId entityId);
    void setPeerEvent(const std::shared_ptr<FuncPeerEvent>& funcPeerEvent);
    IProtocolSessionPtr getSession(PeerId peerId) const;

private:
    void removePeerFromSessionEntityToPeerId(std::int64_t sessionId, EntityId entityId, const std::string& entityName);
    PeerId getPeerIdIntern(std::int64_t sessionId, EntityId entityId, const std::string& entityName) const;

    struct Peer
    {
        IProtocolSessionPtr session;
        EntityId            entityId{ENTITYID_INVALID};
        std::string         entityName;
        bool                incoming = false;
        std::deque<Request> requests;
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
    inline ReplyContext(const PeerManagerPtr& sessionIdEntityIdToPeerId, const IProtocolSessionPtr& session, EntityId entityIdSrc, remoteentity::Header& header)
        : m_peerManager(sessionIdEntityIdToPeerId)
        , m_session(session)
        , m_entityIdDest(header.srcid)
        , m_entityIdSrc(entityIdSrc)
        , m_correlationId(header.corrid)
        , m_replySent(false)
        , m_metainfo(std::move(header.meta))
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
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, structBase.getStructInfo().getTypeName(), m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, header, &structBase);
            m_replySent = true;
        }
    }

    void reply(const StructBase& structBase, const std::vector<std::string>& metainfo)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, structBase.getStructInfo().getTypeName(), m_correlationId, metainfo };
            RemoteEntityFormatRegistry::instance().send(m_session, header, &structBase);
            m_replySent = true;
        }
    }

    void reply(const StructBase& structBase, std::vector<std::string>&& metainfo)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, structBase.getStructInfo().getTypeName(), m_correlationId, std::move(metainfo) };
            RemoteEntityFormatRegistry::instance().send(m_session, header, &structBase);
            m_replySent = true;
        }
    }

    inline CorrelationId correlationId() const
    {
        return m_correlationId;
    }

    std::string* getMetainfo(const std::string& key)
    {
        for (auto it = m_metainfo.begin(); it != m_metainfo.end(); ++it)
        {
            if (*it == key)
            {
                ++it;
                if (it == m_metainfo.end())
                {
                    return nullptr;
                }
                return &*it;
            }
            else
            {
                ++it;
                if (it == m_metainfo.end())
                {
                    return nullptr;
                }
            }
        }
        return nullptr;
    }

    std::vector<std::string>& getAllMetainfo()
    {
        return m_metainfo;
    }


private:
    void reply(remoteentity::Status status)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, status, "", m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, header);
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
    std::vector<std::string>        m_metainfo;

    friend class RemoteEntity;
};






class SYMBOLEXP RemoteEntity : public IRemoteEntity
{
public:
    RemoteEntity();

public:
    // IRemoteEntity
    virtual bool sendEvent(const PeerId& peerId, const StructBase& structBase) override;
    virtual bool sendEvent(const PeerId& peerId, std::vector<std::string>&& metainfo, const StructBase& structBase) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, FuncReplyConnect funcReplyConnect = {}) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, EntityId entityId, FuncReplyConnect funcReplyConnect = {}) override;
    virtual void disconnect(PeerId peerId) override;
    virtual std::vector<PeerId> getAllPeers() const override;
    virtual void registerPeerEvent(FuncPeerEvent funcPeerEvent) override;
    virtual EntityId getEntityId() const override;
    virtual IProtocolSessionPtr getSession(PeerId peerId) const override;
    virtual PeerId createPeer(FuncReplyConnect funcReplyConnect = {}) override;
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName) override;
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId) override;
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId) override;
    virtual void registerCommandFunction(const std::string& functionName, FuncCommand funcCommand) override;
    virtual CorrelationId getNextCorrelationId() const override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId) override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) override;
    virtual bool sendRequest(const PeerId& peerId, std::vector<std::string>&& metainfo, const StructBase& structBase, CorrelationId correlationId) override;
    virtual bool sendRequest(const PeerId& peerId, std::vector<std::string>&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) override;
    virtual bool isEntityRegistered() const override;
    virtual void registerReplyEvent(FuncReplyEvent funcReplyEvent) override;

private:
    virtual void initEntity(EntityId entityId, const std::string& entityName) override;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) override;
    virtual void receivedRequest(const IProtocolSessionPtr& session, remoteentity::Header& header, const StructBasePtr& structBase) override;
    virtual void receivedReply(const IProtocolSessionPtr& session, remoteentity::Header& header, const StructBasePtr& structBase) override;
    virtual void deinit() override;

    PeerId addPeer(const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName, bool incoming, bool& added);
    PeerId connectIntern(const IProtocolSessionPtr& session, const std::string& entityName, EntityId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect);
    void connectIntern(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId);
    void removePeer(PeerId peerId, remoteentity::Status status);
    void replyReceived(CorrelationId correlationId, remoteentity::Status status, std::vector<std::string>& metainfo, const StructBasePtr& structBase);
    void sendConnectEntity(PeerId peerId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect);

    struct Request
    {
        inline Request(PeerId peerId_, const std::shared_ptr<FuncReply>& func_)
            : peerId(peerId_)
            , func(func_)
        {
        }
        inline Request(PeerId peerId_, const std::shared_ptr<FuncReplyMeta>& func_)
            : peerId(peerId_)
            , funcMeta(func_)
        {
        }
        PeerId                          peerId = PEERID_INVALID;
        std::shared_ptr<FuncReply>      func;
        std::shared_ptr<FuncReplyMeta>  funcMeta;
    };

    EntityId                            m_entityId = 0;
    std::string                         m_entityName;

    FuncReplyEvent                      m_funcReplyEvent;
    std::unordered_map<CorrelationId, std::unique_ptr<Request>> m_requests;
    std::unordered_map<std::string, std::shared_ptr<FuncCommand>> m_funcCommands;
    std::shared_ptr<PeerManager>        m_peerManager;
    mutable std::atomic_uint64_t        m_nextCorrelationId{1};
    mutable std::mutex                  m_mutex;
};




}   // namespace finalmq
