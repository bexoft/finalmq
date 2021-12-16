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

#include "finalmq/protocolsession/ProtocolSessionContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatRegistry.h"
#include "finalmq/remoteentity/IRemoteEntity.h"


#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>


namespace finalmq {

static const std::string FMQ_VIRTUAL_SESSION_ID = "fmq_virtsessid";


struct IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


namespace remoteentity {
    class Status;
    class Header;
}



/**
 * @brief The PeerEvent class is an enum with possible entity connection states.
 */
class SYMBOLEXP PeerEvent
{
public:
    enum Enum : std::int32_t {
        PEER_CONNECTED = 0,     ///< The entity connected to a remote entity or a remote entity connected to the entity.
        PEER_DISCONNECTED = 1   ///< The entity disconnected from a remote entity or a remote entity disconnected from the entity.
    };

    PeerEvent();
    PeerEvent(Enum en);
    operator const Enum&() const;
    operator Enum&();
    const PeerEvent& operator =(Enum en);
    const std::string& toName() const;
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = PEER_CONNECTED;
    static const EnumInfo _enumInfo;
};




class SYMBOLEXP PeerManager
{
public:

    enum ReadyToSend
    {
        RTS_READY,
        RTS_SESSION_NOT_AVAILABLE,
        RTS_PEER_NOT_AVAILABLE,
    };

    struct Request
    {
        StructBasePtr   structBase;
        CorrelationId   correlationId = CORRELATIONID_NONE;
    };

    PeerManager();
    std::vector<PeerId> getAllPeers() const;
    std::vector<PeerId> getAllPeersWithSession(IProtocolSessionPtr session) const;
    std::vector<PeerId> getAllPeersWithVirtualSession(IProtocolSessionPtr session, const std::string& virtualSessionId) const;
    void updatePeer(PeerId peerId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName);
    bool removePeer(PeerId peerId, bool& incoming);
    PeerId getPeerId(std::int64_t sessionId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName) const;
    ReadyToSend getRequestHeader(const PeerId& peerId, const std::string& path, const StructBase& structBase, CorrelationId correlationId, remoteentity::Header& header, IProtocolSessionPtr& session, std::string& virtualSessionId);
    std::string getEntityName(const PeerId& peerId);
    PeerId addPeer(const IProtocolSessionPtr& session, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName, bool incoming, bool& added, const std::function<void()>& funcBeforeFirePeerEvent);
    PeerId addPeer();
    std::deque<PeerManager::Request> connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName);
    void setEntityId(EntityId entityId);
    void setPeerEvent(const std::shared_ptr<FuncPeerEvent>& funcPeerEvent);
    IProtocolSessionPtr getSession(PeerId peerId) const;

private:
    struct Peer
    {
        IProtocolSessionPtr session;
        std::string         virtualSessionId;
        EntityId            entityId{ ENTITYID_INVALID };
        std::string         entityName;
        bool                incoming = false;
        std::deque<Request> requests;
        IMessagePtr         requestsMessage;
    };


    void removePeerFromSessionEntityToPeerId(std::int64_t sessionId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName);
    PeerId getPeerIdIntern(std::int64_t sessionId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName) const;
    std::shared_ptr<PeerManager::Peer> getPeer(const PeerId& peerId) const;


    EntityId                            m_entityId{ENTITYID_INVALID};
    std::shared_ptr<FuncPeerEvent>      m_funcPeerEvent;
    std::unordered_map<PeerId, std::shared_ptr<Peer>>    m_peers;
    PeerId                              m_nextPeerId{1};
    std::unordered_map<std::uint64_t, std::unordered_map<std::string, std::pair<std::unordered_map<EntityId, PeerId>, std::unordered_map<std::string, PeerId>>>> m_sessionEntityToPeerId;
    mutable std::mutex  m_mutex;
};
typedef std::shared_ptr<PeerManager> PeerManagerPtr;




class RequestContext : public std::enable_shared_from_this<RequestContext>
{
public:
    inline RequestContext(const PeerManagerPtr& sessionIdEntityIdToPeerId, EntityId entityIdSrc, ReceiveData& receiveData, const std::shared_ptr<FileTransferReply>& fileTransferReply)
        : m_peerManager(sessionIdEntityIdToPeerId)
        , m_session(receiveData.session)
        , m_virtualSessionId(std::move(receiveData.virtualSessionId))
        , m_entityIdDest(receiveData.header.srcid)
        , m_entityIdSrc(entityIdSrc)
        , m_correlationId(receiveData.header.corrid)
        , m_replySent(false)
        , m_metainfo(std::move(receiveData.message->getAllMetainfo()))
        , m_echoData(std::move(receiveData.message->getEchoData()))
        , m_fileTransferReply(fileTransferReply)
    {
    }

    ~RequestContext()
    {
        if (!m_replySent)
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
            m_peerId = m_peerManager->getPeerId(m_session->getSessionId(), m_virtualSessionId, m_entityIdDest, "");
        }
        return m_peerId;
    }

    void reply(const StructBase& structBase, IMessage::Metainfo* metainfo = nullptr)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, {}, structBase.getStructInfo().getTypeName(), m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, m_virtualSessionId, header, std::move(m_echoData), &structBase, metainfo);
            m_replySent = true;
        }
    }

    void reply(Variant& controlData, IMessage::Metainfo* metainfo = nullptr)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, {}, {}, m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, m_virtualSessionId, header, std::move(m_echoData), nullptr, metainfo, &controlData);
            m_replySent = true;
        }
    }

    void reply(remoteentity::Status status)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, status, {}, {}, m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, m_virtualSessionId, header, std::move(m_echoData));
            m_replySent = true;
        }
    }

    bool replyFile(const std::string& filename, IMessage::Metainfo* metainfo = nullptr)
    {
        bool handeled = m_fileTransferReply->replyFile(shared_from_this(), filename, metainfo);
        return handeled;
    }

    void replyMemory(const char* buffer, size_t size, IMessage::Metainfo* metainfo = nullptr)
    {
        remoteentity::RawBytes replyBytes;
        replyBytes.data.resize(size);
        memcpy(const_cast<BytesElement*>(replyBytes.data.data()), buffer, size);
        reply(replyBytes, metainfo);
    }

    inline CorrelationId correlationId() const
    {
        return m_correlationId;
    }

    std::string* getMetainfo(const std::string& key)
    {
        auto it = m_metainfo.find(key);
        if (it != m_metainfo.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    IMessage::Metainfo& getAllMetainfo()
    {
        return m_metainfo;
    }

    bool doesSupportFileTransfer() const
    {
        return m_session->doesSupportFileTransfer();
    }

    const std::string& getVirtualSessionId() const
    {
        return m_virtualSessionId;
    }

private:
    inline const IProtocolSessionPtr& session() const
    {
        return m_session;
    }
    inline EntityId entityId() const
    {
        return m_entityIdDest;
    }

    RequestContext(const RequestContext&) = delete;
    const RequestContext& operator =(const RequestContext&) = delete;
    RequestContext(const RequestContext&&) = delete;
    const RequestContext& operator =(const RequestContext&&) = delete;
private:
    PeerManagerPtr                  m_peerManager;
    IProtocolSessionPtr             m_session;
    std::string                     m_virtualSessionId;
    EntityId                        m_entityIdDest = ENTITYID_INVALID;
    EntityId                        m_entityIdSrc = ENTITYID_INVALID;
    CorrelationId                   m_correlationId = CORRELATIONID_NONE;
    PeerId                          m_peerId = PEERID_INVALID;
    bool                            m_replySent = false;
    IMessage::Metainfo              m_metainfo;
    Variant                         m_echoData;
    std::shared_ptr<FileTransferReply>  m_fileTransferReply;

    friend class RemoteEntity;
};






class SYMBOLEXP RemoteEntity : public IRemoteEntity
{
public:
    RemoteEntity();
    ~RemoteEntity();

public:
    // IRemoteEntity
    virtual bool sendEvent(const PeerId& peerId, const StructBase& structBase) override;
    virtual bool sendEvent(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase) override;
    virtual bool sendEvent(const PeerId& peerId, const std::string& path, const StructBase& structBase) override;
    virtual bool sendEvent(const PeerId& peerId, const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase) override;
    virtual bool sendEventToAllPeers(const StructBase& structBase) override;
    virtual bool sendEventToAllPeers(IMessage::Metainfo&& metainfo, const StructBase& structBase) override;
    virtual bool sendEventToAllPeers(const std::string& path, const StructBase& structBase) override;
    virtual bool sendEventToAllPeers(const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase) override;
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
    virtual void registerCommandFunction(const std::string& path, const std::string& type, FuncCommand funcCommand) override;
    virtual std::string getTypeOfCommandFunction(std::string& path, const std::string* method = nullptr) override;
    virtual CorrelationId getNextCorrelationId() const override;
    virtual bool sendRequest(const PeerId& peerId, const std::string& path, const StructBase& structBase, CorrelationId correlationId, IMessage::Metainfo* metainfo = nullptr) override;
    virtual bool sendRequest(const PeerId& peerId, const std::string& path, const StructBase& structBase, FuncReply funcReply) override;
    virtual bool sendRequest(const PeerId& peerId, const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) override;
    virtual bool sendRequest(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) override;
    virtual bool isEntityRegistered() const override;
    virtual void registerReplyEvent(FuncReplyEvent funcReplyEvent) override;
    virtual IExecutorPtr getExecutor() const override;
    virtual PeerId createPublishPeer(const IProtocolSessionPtr& session, const std::string& entityName) override;

private:
    virtual void initEntity(EntityId entityId, const std::string& entityName, const std::shared_ptr<FileTransferReply>& fileTransferReply, const IExecutorPtr& executor) override;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) override;
    virtual void virtualSessionDisconnected(const IProtocolSessionPtr& session, const std::string& virtualSessionId) override;
    virtual void receivedRequest(ReceiveData& receiveData) override;
    virtual void receivedReply(ReceiveData& receiveData) override;
    virtual void deinit() override;

    PeerId connectIntern(const IProtocolSessionPtr& session, const std::string& virtualSessionId, const std::string& entityName, EntityId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect);
    void connectIntern(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId);
    void removePeer(PeerId peerId, remoteentity::Status status);
    void replyReceived(ReceiveData& receiveData);
    void sendConnectEntity(PeerId peerId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect);

    struct Function
    {
        std::string                  type;
        std::shared_ptr<FuncCommand> func;
    };
    struct FunctionVar : public Function
    {
        std::vector<std::string>     pathEntries;
    };

    const Function* getFunction(const std::string& path, IMessage::Metainfo* keys = nullptr) const;

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


    EntityId                            m_entityId{ ENTITYID_INVALID };
    std::string                         m_entityName;

    FuncReplyEvent                      m_funcReplyEvent;
    std::unordered_map<CorrelationId, std::unique_ptr<Request>> m_requests;
    std::unordered_map<std::string, Function> m_funcCommandsStatic;
    std::list<FunctionVar>              m_funcCommandsVar;
    std::list<FunctionVar>              m_funcCommandsVarStar;
    std::shared_ptr<PeerManager>        m_peerManager;
    std::shared_ptr<FileTransferReply>  m_fileTransferReply;
    IExecutorPtr                        m_executor;
    mutable std::atomic_uint64_t        m_nextCorrelationId{1};
    mutable std::mutex                  m_mutex;
    mutable std::mutex                  m_mutexFunctions;
};




}   // namespace finalmq
