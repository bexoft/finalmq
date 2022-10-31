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


#include "finalmq/remoteentity/entitydata.fmq.h"
#include "finalmq/remoteentity/FileTransferReply.h"


namespace finalmq {

using PeerId = std::int64_t;
static constexpr PeerId PEERID_INVALID = 0;

class PeerEvent;
class StructBase;
class RequestContext;
typedef std::shared_ptr<RequestContext> RequestContextPtr;

using CorrelationId = std::uint64_t;
static constexpr CorrelationId CORRELATIONID_NONE = 0;

using EntityId = std::uint64_t;
static constexpr EntityId ENTITYID_DEFAULT = 0;
static constexpr EntityId ENTITYID_INVALID = 0x7fffffffffffffffull;

struct ReceiveData
{
    IProtocolSessionPtr         session;
    std::string                 virtualSessionId;
    IMessagePtr                 message;
    Header        header;
    std::shared_ptr<StructBase> structBase;
};




typedef std::function<void(PeerId peerId, Status status, const StructBasePtr& structBase)> FuncReply;
typedef std::function<void(PeerId peerId, Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase)> FuncReplyMeta;
typedef std::function<void(RequestContextPtr& requestContext, const StructBasePtr& structBase)> FuncCommand;
typedef std::function<void(PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, PeerEvent peerEvent, bool incoming)> FuncPeerEvent;
typedef std::function<bool(CorrelationId correlationId, Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase)> FuncReplyEvent; // return bool reply handled -> skip looking for reply lambda.
typedef std::function<void(PeerId peerId, Status status)> FuncReplyConnect;


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
     * @param path is the path that shall be called at the remote entity
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return if successful, valid correlation ID.
     */
    template<class R>
    CorrelationId requestReply(const PeerId& peerId,
        const std::string& path, const StructBase& structBase,
        std::function<void(PeerId peerId, Status status, const std::shared_ptr<R>& reply)> funcReply)
    {
        assert(funcReply);
        CorrelationId correlationId = sendRequest(peerId, path, structBase, [funcReply{ std::move(funcReply) }](PeerId peerId, Status status, const StructBasePtr& structBase) {
            std::shared_ptr<R> reply;
            bool typeOk = (!structBase || structBase->getStructInfo().getTypeName() == R::structInfo().getTypeName());
            if (status == Status::STATUS_OK && structBase && typeOk)
            {
                reply = std::static_pointer_cast<R>(structBase);
            }
            if (!typeOk)
            {
                status = Status::STATUS_WRONG_REPLY_TYPE;
            }
            funcReply(peerId, status, reply);
        });
        return correlationId;
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
     * @param path is the path that shall be called at the remote entity
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return if successful, valid correlation ID.
     */
    template<class R>
    CorrelationId requestReply(const PeerId& peerId,
        const std::string& path,
        IMessage::Metainfo&& metainfo,
        const StructBase& structBase,
        std::function<void(PeerId peerId, Status status, IMessage::Metainfo& metainfo, const std::shared_ptr<R>& reply)> funcReply)
    {
        assert(funcReply);
        CorrelationId correlationId = sendRequest(peerId, path, std::move(metainfo), structBase, [funcReply{ std::move(funcReply) }](PeerId peerId, Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase) {
            std::shared_ptr<R> reply;
            bool typeOk = (!structBase || structBase->getStructInfo().getTypeName() == R::structInfo().getTypeName());
            if (status == Status::STATUS_OK && structBase && typeOk)
            {
                reply = std::static_pointer_cast<R>(structBase);
            }
            if (!typeOk)
            {
                status = Status::STATUS_WRONG_REPLY_TYPE;
            }
            funcReply(peerId, status, metainfo, reply);
        });
        return correlationId;
    }

    /**
     * @brief requestReply sends a request to the peer and the funcReply is triggered when
     * the reply is available. The template parameter is the message type of the reply (generated code of fmq file).
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return if successful, valid correlation ID.
     */
    template<class R>
    CorrelationId requestReply(const PeerId& peerId,
        const StructBase& structBase,
        std::function<void(PeerId peerId, Status status, const std::shared_ptr<R>& reply)> funcReply)
    {
        assert(funcReply);
        CorrelationId correlationId = sendRequest(peerId, structBase, [funcReply{ std::move(funcReply) }](PeerId peerId, Status status, const StructBasePtr& structBase) {
            std::shared_ptr<R> reply;
            bool typeOk = (!structBase || structBase->getStructInfo().getTypeName() == R::structInfo().getTypeName());
            if (status == Status::STATUS_OK && structBase && typeOk)
            {
                reply = std::static_pointer_cast<R>(structBase);
            }
            if (!typeOk)
            {
                status = Status::STATUS_WRONG_REPLY_TYPE;
            }
            funcReply(peerId, status, reply);
        });
        return correlationId;
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
     * @return if successful, valid correlation ID.
     */
    template<class R>
    CorrelationId requestReply(const PeerId& peerId,
        IMessage::Metainfo&& metainfo,
        const StructBase& structBase,
        std::function<void(PeerId peerId, Status status, IMessage::Metainfo& metainfo, const std::shared_ptr<R>& reply)> funcReply)
    {
        assert(funcReply);
        CorrelationId correlationId = sendRequest(peerId, std::move(metainfo), structBase, [funcReply{ std::move(funcReply) }](PeerId peerId, Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase) {
            std::shared_ptr<R> reply;
            bool typeOk = (!structBase || structBase->getStructInfo().getTypeName() == R::structInfo().getTypeName());
            if (status == Status::STATUS_OK && structBase && typeOk)
            {
                reply = std::static_pointer_cast<R>(structBase);
            }
            if (!typeOk)
            {
                status = Status::STATUS_WRONG_REPLY_TYPE;
            }
            funcReply(peerId, status, metainfo, reply);
        });
        return correlationId;
    }

    /**
     * @brief sendEvent sends a request to the peer and does not expect a reply.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEvent(const PeerId& peerId, const StructBase& structBase) = 0;

    /**
     * @brief sendEvent sends a request to the peer and does not expect a reply.
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers.
     * You can use it to exchange additional data besides the message data.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEvent(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase) = 0;

    /**
     * @brief sendEvent sends a request to the peer and does not expect a reply.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param path is the path that shall be called at the remote entity
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEvent(const PeerId& peerId, const std::string& path, const StructBase& structBase) = 0;

    /**
     * @brief sendEvent sends a request to the peer and does not expect a reply.
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers.
     * You can use it to exchange additional data besides the message data.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param path is the path that shall be called at the remote entity
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEvent(const PeerId& peerId, const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase) = 0;

    /**
     * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEventToAllPeers(const StructBase& structBase) = 0;

    /**
     * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers.
     * You can use it to exchange additional data besides the message data.
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEventToAllPeers(IMessage::Metainfo&& metainfo, const StructBase& structBase) = 0;

    /**
     * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
     * @param path is the path that shall be called at the remote entity
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEventToAllPeers(const std::string& path, const StructBase& structBase) = 0;

    /**
     * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers.
     * You can use it to exchange additional data besides the message data.
     * @param path is the path that shall be called at the remote entity
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the event message to send (generated code of fmq file).
     */
    virtual void sendEventToAllPeers(const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase) = 0;

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
        registerCommandFunction(R::structInfo().getTypeName(), R::structInfo().getTypeName(), reinterpret_cast<FuncCommand&>(funcCommand));
    }

    /**
     * @brief registerCommand registers a callback function for executing a request or event.
     * The template parameter is the message type of the request/event (generated code of fmq file).
     * @param path ist the path how to access the command.
     * @param funcCommand is the callback function. Is will be called whenever the request/event is
     * sent by a peer. Inside the callback you can reply to the peer with requestContext->reply().
     * If you cannot reply immediatelly, then you can store the requestContext and reply later
     * whenever you would like to reply. If the command is an event, then you do not have to reply
     * at all. If a client peer calls requestReply, but you do not reply, then the client will get
     * the status = NO_REPLY. If the client peer calls sendEvent, but you reply, then the reply
     * will not be sent.
     */
    template<class R>
    void registerCommand(const std::string& path, std::function<void(const RequestContextPtr& requestContext, const std::shared_ptr<R>& request)> funcCommand)
    {
        registerCommandFunction(path, R::structInfo().getTypeName(), reinterpret_cast<FuncCommand&>(funcCommand));
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
     * @param path is the path of the function.
     * @param type is the name of the concrete request type.
     * @param funcCommand is the callback function. Is will be called whenever the request/event is
     * sent by a peer. Inside the callback you can reply to the peer with requestContext->reply().
     * If you cannot reply immediatelly, then you can store the requestContext and reply later
     * whenever you would like to reply. If the command is an event, then you do not have to reply
     * at all. If a client peer calls requestReply, but you do not reply, then the client will get
     * the status = NO_REPLY. If the client peer calls sendEvent, but you reply, then the reply
     * will not be sent.
     */
    virtual void registerCommandFunction(const std::string& path, const std::string& type, FuncCommand funcCommand) = 0;

    /**
     * @brief gets the type of the function, which is defined at path.
     * @param path is the path of the function. Can be adjusted by the call, if the method is relevant for the function.
     * @param method is a http method (GET,POST,...).
     * @return expected type of the function.
     */
    virtual std::string getTypeOfCommandFunction(std::string& path, const std::string* method = nullptr) = 0;

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
     * @param path is the path that shall be called at the remote entity
     * @param structBase is the request message to send (generated code of fmq file).
     * @param correlationId is an ID that can be matched at the callback, which is registered
     * by registerReplyEvent().
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     */
    virtual void sendRequest(const PeerId& peerId, const std::string& path, const StructBase& structBase, CorrelationId correlationId, IMessage::Metainfo* metainfo = nullptr) = 0;

    /**
     * @brief sendRequest sends a request to the peer and the funcReply is triggered when
     * the reply is available.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param path is the path that shall be called at the remote entity
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return if successful, valid correlation ID.
     */
    virtual CorrelationId sendRequest(const PeerId& peerId, const std::string& path, const StructBase& structBase, FuncReply funcReply) = 0;

    /**
     * @brief sendRequest sends a request to the peer and the funcReply is triggered when
     * the reply is available.
     * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers. You can use it
     * to exchange additional data besides the message data.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param path is the path that shall be called at the remote entity
     * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return if successful, valid correlation ID.
     */
    virtual CorrelationId sendRequest(const PeerId& peerId, const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) = 0;

    /**
     * @brief sendRequest sends a request to the peer and the funcReply is triggered when
     * the reply is available.
     * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
     * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
     * command execution. The peerId belongs to this entity. Because an entity can have multiple
     * connections to remote entities, a remote entity must be identified be the peerId.
     * @param structBase is the request message to send (generated code of fmq file).
     * @param funcReply is the reply callback.
     * @return if successful, valid correlation ID.
     */
    virtual CorrelationId sendRequest(const PeerId& peerId, const StructBase& structBase, FuncReply funcReply) = 0;

    /**
     * @brief sendRequest sends a request to the peer and the funcReply is triggered when
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
     * @return if successful, valid correlation ID.
     */
    virtual CorrelationId sendRequest(const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase, FuncReplyMeta funcReply) = 0;

    /**
    * @brief cancels a reply callback. After calling this function, the expected reply callback will not be called, anymore.
    * Call this function, if e.g. a timeout happened, and you are not interested in the reply, anymore.
    * @param correlationId of the request, which reply callback shall not be called, anymore
    * @return true, if the request was still pending and the reply callback was canceled. false, if the reply callback, was already called or 
    * if it is still running.
    */
    virtual bool cancelReply(CorrelationId correlationId) = 0;

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

    /**
    * @brief creates a peer at the own entity, so that the peer will publish events to the session.
    * This can be used e.g. for mqtt sessions.
    * @param entityName is the name of the destination. The mqtt topic will look like this: "/<entityName>/<message type>".
    * @return the created peer id.
    */
    virtual PeerId createPublishPeer(const IProtocolSessionPtr& session, const std::string& entityName) = 0;


private:
    // methods for RemoteEntityContainer
    virtual void initEntity(EntityId entityId, const std::string& entityName, const std::shared_ptr<FileTransferReply>& fileTransferReply, const IExecutorPtr& executorPollerThread) = 0;
    virtual void sessionDisconnected(const IProtocolSessionPtr& session) = 0;
    virtual void virtualSessionDisconnected(const IProtocolSessionPtr& session, const std::string& virtualSessionId) = 0;
    virtual void receivedRequest(ReceiveData& receiveData) = 0;
    virtual void receivedReply(const ReceiveData& receiveData) = 0;
    virtual void deinit() = 0;
    friend class RemoteEntityContainer;
};
typedef std::shared_ptr<IRemoteEntity> IRemoteEntityPtr;




}   // namespace finalmq
