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
#include "finalmq/remoteentity/FileTransferReply.h"
#include "finalmq/remoteentity/entitydata.fmq.h"


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




using PeerId = std::int64_t;
static constexpr PeerId PEERID_INVALID = 0;



class RequestContext;
typedef std::shared_ptr<RequestContext> RequestContextPtr;


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
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = PEER_CONNECTED;
    static const EnumInfo _enumInfo;
};


struct ReceiveData
{
    IProtocolSessionPtr         session;
    IMessagePtr                 message;
    remoteentity::Header        header;
    std::shared_ptr<StructBase> structBase;
};



typedef std::function<void(PeerId peerId, remoteentity::Status status, const StructBasePtr& structBase)> FuncReply;
typedef std::function<void(PeerId peerId, remoteentity::Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase)> FuncReplyMeta;
typedef std::function<void(RequestContextPtr& requestContext, const StructBasePtr& structBase)> FuncCommand;
typedef std::function<void(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, PeerEvent peerEvent, bool incoming)> FuncPeerEvent;
typedef std::function<bool(CorrelationId correlationId, remoteentity::Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase)> FuncReplyEvent; // return bool reply handled -> skip looking for reply lambda.
typedef std::function<void(PeerId peerId, remoteentity::Status status)> FuncReplyConnect;

struct IRemoteEntity
{
    /**
     * @brief ~IRemoteEntity is the virtual destructor of the interface.
     */
    virtual ~IRemoteEntity() {}

    /**
     * @brief requestReply sends a request to the peer and the funcReply is triggered when
     * the reply is available. The template parameter is the message type of the reply (generated code of fmq file).
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return true on success.
     */
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

    /**
     * @brief requestReply sends a request to the peer and the funcReply is triggered when
     * the reply is available. The template parameter is the message type of the reply (generated code of fmq file).
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers. You can use it
     * to exchange additional data besides the message data.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return true on success.
     */
    template<class R>
    bool requestReply(const PeerId& peerId,
        IMessage::Metainfo&& metainfo,
        const StructBase& structBase,
        std::function<void(PeerId peerId, remoteentity::Status status, IMessage::Metainfo& metainfo, const std::shared_ptr<R>& reply)> funcReply)
    {
        assert(funcReply);
        bool ok = sendRequest(peerId, std::move(metainfo), structBase, [funcReply{ std::move(funcReply) }](PeerId peerId, remoteentity::Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase) {
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

    /**
     * @brief sendEvent sends a request to the peer and does not expect a reply.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param structBase is the request message to send (generated code of fmq file).
     * @return true on success.
     */
    virtual bool sendEvent(const PeerId& peerId, const StructBase& structBase) = 0;

    /**
     * @brief sendEvent sends a request to the peer and does not expect a reply.
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers.
     * You can use it to exchange additional data besides the message data.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the request message to send (generated code of fmq file).
     * @return true on success.
     */
    virtual bool sendEvent(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase) = 0;

    /**
     * @brief registerCommand registers a callback function for executing a request or event.
     * The template parameter is the message type of the request/event (generated code of fmq file).
     * @param funcCommand is the callback function. Is will be called whenever the request/event is
     * sent by a peer. Inside the callback you can reply to the peer with requestContext->reply().
     * If you cannot reply immediatelly, then you can store the requestContext and reply later
     * whenever you would like to reply. If the command is an event, then you do not have to reply
     * at all. If a client peer calls requestReply, but you do not reply, then the client will get
     * the status = NO_REPLY. If the client peer calls sendEvent, but you reply, then the reply
     * will not be sent.
     */
    template<class R>
    void registerCommand(std::function<void(const RequestContextPtr& requestContext, const std::shared_ptr<R>& request)> funcCommand)
    {
        registerCommandFunction(R::structInfo().getTypeName(), reinterpret_cast<FuncCommand&>(funcCommand));
    }

    /**
     * @brief connect the entity with a remote entity. This entity-to-entity connection is represented by a peer ID.
     * You can connect an entity with multiple remote entities. For each connection you will get a peer ID.
     * To connect to a remote entity, you have to pass a session and the name of the remote entity (how it is
     * registered at its RemoteEntityContainer).
     * @param session is the session that is used for this entity-to-entity connection.
     * @param entityName is the name of the remote entity to which this entity shall be connected. The entityName is the name
     * how the remote entity is registered at its RemoteEntityContainer.
     * @param funcReplyConnect is the callback for this connection, it will indicate if the connection was successful.
     * @return the peer ID. Use this ID to send requests/events to the remote entity.
     */
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, FuncReplyConnect funcReplyConnect = {}) = 0;

    /**
     * @brief connect the entity with a remote entity. This entity-to-entity connection is represented by a peer ID.
     * You can connect an entity with multiple remote entities. For each connection you will get a peer ID.
     * To connect to a remote entity, you have to pass a session and the ID of the remote entity.
     * @param session is the session that is used for this entity-to-entity connection.
     * @param entityId is the ID of the remote entity to which this entity shall be connected.
     * @param funcReplyConnect is the callback for this connection, it will indicate if the connection was successful.
     * @return the peer ID. Use this ID to send requests/events to the remote entity.
     */
    virtual PeerId connect(const IProtocolSessionPtr& session, EntityId entityId, FuncReplyConnect funcReplyConnect = {}) = 0;

    /**
     * @brief disconnect releases the entity-to-entity connection. Open requests which were not answered, yet, will be
     * answered with status Status::STATUS_PEER_DISCONNECTED.
     * @param peerId to identify which entity-to-entity connection shall be released.
     */
    virtual void disconnect(PeerId peerId) = 0;

    /**
     * @brief getAllPeers gets all entity-to-entity connections of this entity.
     * @return a vector of peer IDs.
     */
    virtual std::vector<PeerId> getAllPeers() const = 0;

    /**
     * @brief registerPeerEvent registers a callback that will be triggered whenever an entity-to-entity connection
     * is established or released.
     * @param funcPeerEvent the callback.
     */
    virtual void registerPeerEvent(FuncPeerEvent funcPeerEvent) = 0;

    /**
     * @brief getEntityId returns the entity ID of this entity.
     * @return the entity ID.
     */
    virtual EntityId getEntityId() const = 0;

    /**
     * @brief getSession returns the session which is used for the entity-to-entity connection of a certain peer ID.
     * @param peerId the peer ID.
     * @return the session of the peer ID.
     */
    virtual IProtocolSessionPtr getSession(PeerId peerId) const = 0;

    // low level methods

    /**
     * @brief createPeer creates an entity-to-entity connection (peer) without a session and entity identifier.
     * Use this method, when you want to send requests without knowing to whom. As soon as you know the session
     * and remote entity, then call connect that has a peerId as a parameter.
     * @param funcReplyConnect is the callback that indicates if a later connection was successful.
     * @return the peer ID. Use it to send requests/events and to connect with a session and entity identifier.
     */
    virtual PeerId createPeer(FuncReplyConnect funcReplyConnect = {}) = 0;

    /**
     * @brief connect connects the peer that was created with createPeer().
     * @param peerId is the ID that was returned by createPeer().
     * @param session is the session that is used for this entity-to-entity connection.
     * @param entityName is the name of the remote entity to which this entity shall be connected.
     */
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName) = 0;

    /**
     * @brief connect connects the peer that was created with createPeer().
     * @param peerId is the ID that was returned by createPeer().
     * @param session is the session that is used for this entity-to-entity connection.
     * @param entityId is the ID of the remote entity to which this entity shall be connected.
     */
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId) = 0;

    /**
     * @brief connect connects the peer that was created with createPeer().
     * @param peerId is the ID that was returned by createPeer().
     * @param session is the session that is used for this entity-to-entity connection.
     * @param entityName is the name of the remote entity to which this entity shall be connected. If
     * the name is empty then the entityId will be used to identify the repote entity.
     * @param entityId is the ID of the remote entity to which this entity shall be connected.
     */
    virtual void connect(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId) = 0;

    /**
     * @brief registerCommandFunction registers a callback function for executing a request or event.
     * The request message is received as StructBase. Use registerCommand() to have the concrete request type.
     * @param functionName is the name of the concrete request type.
     * @param funcCommand is the callback function. Is will be called whenever the request/event is
     * sent by a peer. Inside the callback you can reply to the peer with requestContext->reply().
     * If you cannot reply immediatelly, then you can store the requestContext and reply later
     * whenever you would like to reply. If the command is an event, then you do not have to reply
     * at all. If a client peer calls requestReply, but you do not reply, then the client will get
     * the status = NO_REPLY. If the client peer calls sendEvent, but you reply, then the reply
     * will not be sent.
     */
    virtual void registerCommandFunction(const std::string& functionName, FuncCommand funcCommand) = 0;

    /**
     * @brief getNextCorrelationId creates a correlation ID that is unique inside the entity.
     * @return the correlation ID.
     */
    virtual CorrelationId getNextCorrelationId() const = 0;

    /**
     * @brief sendRequest sends a request to the peer. The reply will be received by the callback,
     * which is registered by registerReplyEvent().
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param correlationId is an ID that can be matched at the callback, which is registered
     * by registerReplyEvent().
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @return true on success.
     */
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId, IMessage::Metainfo* metainfo = nullptr) = 0;

    /**
     * @brief requestReply sends a request to the peer and the funcReply is triggered when
     * the reply is available.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return true on success.
     */
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) = 0;

    /**
     * @brief requestReply sends a request to the peer and the funcReply is triggered when
     * the reply is available.
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers. You can use it
     * to exchange additional data besides the message data.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return true on success.
     */
    virtual bool sendRequest(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) = 0;

    /**
     * @brief isEntityRegistered returns the information, if the entity was registered at a RemoteEntityContainer.
     * @return true, if the entity was registered, otherwise false.
     */
    virtual bool isEntityRegistered() const = 0;

    /**
     * @brief registerReplyEvent registers a callback, which is triggered for every received reply.
     * With this callback a match with the correlation ID can be done by the application.
     * @param funcReplyEvent is the callback function.
     */
    virtual void registerReplyEvent(FuncReplyEvent funcReplyEvent) = 0;

    /**
     * @brief getExecutor returns the executor. Even if no executor was passed at RemoteEntityContainer::init(),
     * it will return an executor that executes actions inside the poller thread.
     * @return the executor. With this executor, you can execute actions inside the executor's thread context.
     */
    virtual IExecutorPtr getExecutor() const = 0;

private:
    // methods for RemoteEntityContainer
    virtual void initEntity(EntityId entityId, const std::string& entityName, const std::shared_ptr<FileTransferReply>& fileTransferReply, const IExecutorPtr& executorPollerThread) = 0;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) = 0;
    virtual void receivedRequest(ReceiveData& receiveData) = 0;
    virtual void receivedReply(ReceiveData& receiveData) = 0;
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
    void updatePeer(PeerId peerId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName);
    bool removePeer(PeerId peerId, bool& incoming);
    PeerId getPeerId(std::int64_t sessionId, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName) const;
    ReadyToSend getRequestHeader(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId, remoteentity::Header& header, IProtocolSessionPtr& session);
    std::string getEntityName(const PeerId& peerId);
    PeerId addPeer(const IProtocolSessionPtr& session, const std::string& virtualSessionId, EntityId entityId, const std::string& entityName, bool incoming, bool& added, const std::function<void()>& funcBeforeFirePeerEvent);
    PeerId addPeer();
    std::deque<PeerManager::Request> connect(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, const std::string& entityName);
    void setEntityId(EntityId entityId);
    void setPeerEvent(const std::shared_ptr<FuncPeerEvent>& funcPeerEvent);
    IProtocolSessionPtr getSession(PeerId peerId) const;
    bool isVirtualSessionAvailable(const IProtocolSessionPtr& session, const std::string& virtualSessionId) const;

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
            const std::string& virtualSessionId = m_metainfo[FMQ_VIRTUAL_SESSION_ID];
            m_peerId = m_peerManager->getPeerId(m_session->getSessionId(), virtualSessionId, m_entityIdDest, "");
        }
        return m_peerId;
    }

    void reply(const StructBase& structBase, IMessage::Metainfo* metainfo = nullptr)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, structBase.getStructInfo().getTypeName(), m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, header, std::move(m_echoData), &structBase, metainfo);
            m_replySent = true;
        }
    }

    void reply(Variant& controlData, IMessage::Metainfo* metainfo = nullptr)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, remoteentity::Status::STATUS_OK, {}, m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, header, std::move(m_echoData), nullptr, metainfo, &controlData);
            m_replySent = true;
        }
    }

    void reply(remoteentity::Status status)
    {
        if (!m_replySent)
        {
            remoteentity::Header header{ m_entityIdDest, "", m_entityIdSrc, remoteentity::MsgMode::MSG_REPLY, status, "", m_correlationId, {} };
            RemoteEntityFormatRegistry::instance().send(m_session, header, std::move(m_echoData));
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
        remoteentity::Bytes replyBytes;
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
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, CorrelationId correlationId, IMessage::Metainfo* metainfo = nullptr) override;
    virtual bool sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) override;
    virtual bool sendRequest(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) override;
    virtual bool isEntityRegistered() const override;
    virtual void registerReplyEvent(FuncReplyEvent funcReplyEvent) override;
    virtual IExecutorPtr getExecutor() const override;

private:
    virtual void initEntity(EntityId entityId, const std::string& entityName, const std::shared_ptr<FileTransferReply>& fileTransferReply, const IExecutorPtr& executor) override;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) override;
    virtual void receivedRequest(ReceiveData& receiveData) override;
    virtual void receivedReply(ReceiveData& receiveData) override;
    virtual void deinit() override;

    PeerId connectIntern(const IProtocolSessionPtr& session, const std::string& virtualSessionId, const std::string& entityName, EntityId, const std::shared_ptr<FuncReplyConnect>& funcReplyConnect);
    void connectIntern(PeerId peerId, const IProtocolSessionPtr& session, const std::string& entityName, EntityId entityId);
    void removePeer(PeerId peerId, remoteentity::Status status);
    void replyReceived(ReceiveData& receiveData);
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

    EntityId                            m_entityId{ ENTITYID_INVALID };
    std::string                         m_entityName;

    FuncReplyEvent                      m_funcReplyEvent;
    std::unordered_map<CorrelationId, std::unique_ptr<Request>> m_requests;
    std::unordered_map<std::string, std::shared_ptr<FuncCommand>> m_funcCommands;
    std::shared_ptr<PeerManager>        m_peerManager;
    std::shared_ptr<FileTransferReply>  m_fileTransferReply;
    IExecutorPtr                        m_executor;
    mutable std::atomic_uint64_t        m_nextCorrelationId{1};
    mutable std::mutex                  m_mutex;
};




}   // namespace finalmq
