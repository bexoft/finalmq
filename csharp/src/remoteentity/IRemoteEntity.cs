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



global using PeerId = System.Int64;
global using CorrelationId = System.UInt64;
global using EntityId = System.UInt64;

using System.Diagnostics;


namespace finalmq {


    // todo remove interface IRemoteEntityContainer
    public interface IRemoteEntityContainer
    {

    }
    public class RequestContext
    {
        public bool DoesSupportFileTransfer
        {
            get
            {
                return false;
            }
        }

        public void Reply(Variant controlData, Metainfo? metainfo = null)
        {

        }

        public void Reply(Status status)
        {

        }

        public void Reply(StructBase? structBase, Metainfo? metainfo = null)
        {
        }


    }

    public class SessionInfo
    {
        public SessionInfo(IRemoteEntityContainer entityContainer, IProtocolSession session)
        {
            m_entityContainer = entityContainer;
            m_session = session;
            Debug.Assert(m_entityContainer != null);
            Debug.Assert(m_session != null);
            m_sessionId = session.SessionId;
        }

        public SessionInfo()
        {
        }

        public IRemoteEntityContainer? EntityContainer
        {
            get
            {
                return m_entityContainer;
            }
        }
        public IProtocolSession? Session
        {
            get
            {
                return m_session;
            }
        }

        public long SessionId
        {
            get
            {
                return m_sessionId;
            }
        }

        //operator bool()
        //{
        //    return (m_session != null);
        //}

        public void Disconnect()
        {
            if (m_session != null)
            {
                m_session.Disconnect();
            }
        }

        static readonly ConnectionData EmptyConnectionData = new ConnectionData();

        public ConnectionData ConnectionData
        {
            get
            {
                if (m_session != null)
                {
                    return m_session.ConnectionData;
                }
                return EmptyConnectionData;
            }
        }

        public bool DoesSupportFileTransfer
        {
            get
            {
                if (m_session != null)
                {
                    return m_session.DoesSupportFileTransfer();
                }
                return false;
            }
        }

        IRemoteEntityContainer? m_entityContainer = null;
        IProtocolSession? m_session = null;
        long m_sessionId = 0;
    };



    public class ReceiveData
    {
        public SessionInfo? Session
        {
            get => m_session;
            set => m_session = value;
        }
        public string? VirtualSessionId
        {
            get => m_virtualSessionId;
            set => m_virtualSessionId = value;
        }
        public IMessage? Message
        {
            get => m_message;
            set => m_message = value;
        }
        public Header? Header
        {
            get => m_header;
            set => m_header = value;
        }
        public bool AutomaticConnect
        {
            get => m_automaticConnect;
            set => m_automaticConnect = value;
        }
        public StructBase? StructBase
        {
            get => m_structBase;
            set => m_structBase = value;
        }

        SessionInfo? m_session = null;
        string? m_virtualSessionId = null;
        IMessage? m_message = null;
        Header? m_header = null;
        bool m_automaticConnect = false;
        StructBase? m_structBase = null;
    };


    public enum PeerEvent
    {
        PEER_CONNECTED = 0,     ///< The entity connected to a remote entity or a remote entity connected to the entity.
        PEER_DISCONNECTED = 1   ///< The entity disconnected from a remote entity or a remote entity disconnected from the entity.
    };


    public delegate void FuncReply(PeerId peerId, Status status, StructBase? structBase);
    public delegate void FuncReplyMeta(PeerId peerId, Status status, Metainfo metainfo, StructBase? structBase);
    public delegate void FuncCommand(RequestContext requestContext, StructBase structBase);
    public delegate void FuncPeerEvent(PeerId peerId, SessionInfo sessionInfo, EntityId entityId, PeerEvent peerEvent, bool incoming);
    public delegate bool FuncReplyEvent(CorrelationId correlationId, Status status, Metainfo metainfo, StructBase structBase); // return bool reply handled -> skip looking for reply lambda.
    public delegate void FuncReplyConnect(PeerId peerId, Status status);

    public delegate void FuncReplyR<R>(PeerId peerId, Status status, R? reply) where R : StructBase;
    public delegate void FuncReplyMetaR<R>(PeerId peerId, Status status, Metainfo metainfo, R? structBase) where R : StructBase;
    public delegate void FuncCommandR<R>(RequestContext requestContext, R structBase) where R : StructBase;

    public interface IRemoteEntity
    {
        public static readonly PeerId PEERID_INVALID = 0;
        public static CorrelationId CORRELATIONID_NONE = 0;

        public static EntityId ENTITYID_DEFAULT = 0;
        public static EntityId ENTITYID_INVALID = 0x7fffffffffffffff;

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
        CorrelationId RequestReply<R>(PeerId peerId,
            string path, StructBase structBase, FuncReplyR<R> funcReply) where R : StructBase
        {
            Debug.Assert(funcReply != null);
            CorrelationId correlationId = SendRequest(peerId, path, structBase, (PeerId peerId, Status status, StructBase? structBase) => {
                R? reply = null;
                if (status == Status.STATUS_OK && structBase != null)
                {
                    reply = structBase as R;
                    if (reply == null)
                    {
                        status = Status.STATUS_WRONG_REPLY_TYPE;
                    }
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
        CorrelationId RequestReply<R>(PeerId peerId,
            string path, Metainfo metainfo, StructBase structBase, FuncReplyMetaR<R> funcReply) where R : StructBase
        {
            Debug.Assert(funcReply != null);
            CorrelationId correlationId = SendRequest(peerId, path, metainfo, structBase, (PeerId peerId, Status status, Metainfo metainfo, StructBase? structBase) => {
                R? reply = null;
                if (status == Status.STATUS_OK && structBase != null)
                {
                    reply = structBase as R;
                    if (reply == null)
                    {
                        status = Status.STATUS_WRONG_REPLY_TYPE;
                    }
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
        CorrelationId RequestReply<R>(PeerId peerId,
            StructBase structBase, FuncReplyR<R> funcReply) where R : StructBase
        {
            Debug.Assert(funcReply != null);
            CorrelationId correlationId = SendRequest(peerId, structBase, (PeerId peerId, Status status, StructBase? structBase) => {
                R? reply = null;
                if (status == Status.STATUS_OK && structBase != null)
                {
                    reply = structBase as R;
                    if (reply == null)
                    {
                        status = Status.STATUS_WRONG_REPLY_TYPE;
                    }
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
        CorrelationId RequestReply<R>(PeerId peerId,
            Metainfo metainfo, StructBase structBase, FuncReplyMetaR<R> funcReply) where R : StructBase
        {
            Debug.Assert(funcReply != null);
            CorrelationId correlationId = SendRequest(peerId, metainfo, structBase, (PeerId peerId, Status status, Metainfo metainfo, StructBase? structBase) => {
                R? reply = null;
                if (status == Status.STATUS_OK && structBase != null)
                {
                    reply = structBase as R;
                    if (reply == null)
                    {
                        status = Status.STATUS_WRONG_REPLY_TYPE;
                    }
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
        void SendEvent(PeerId peerId, StructBase structBase);

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
        void SendEvent(PeerId peerId, Metainfo metainfo, StructBase structBase);

        /**
         * @brief sendEvent sends a request to the peer and does not expect a reply.
         * @param peerId is the id of the peer. You can get it when you connect() to a peer, when you
         * call getAllPeers(), inside a peer event or by calling requestContext->peerId() inside a
         * command execution. The peerId belongs to this entity. Because an entity can have multiple
         * connections to remote entities, a remote entity must be identified be the peerId.
         * @param path is the path that shall be called at the remote entity
         * @param structBase is the event message to send (generated code of fmq file).
         */
        void SendEvent(PeerId peerId, string path, StructBase structBase);

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
        void SendEvent(PeerId peerId, string path, Metainfo metainfo, StructBase structBase);

        /**
         * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
         * @param structBase is the event message to send (generated code of fmq file).
         */
        void SendEventToAllPeers(StructBase structBase);

        /**
         * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
         * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers.
         * You can use it to exchange additional data besides the message data.
         * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
         * @param structBase is the event message to send (generated code of fmq file).
         */
        void SendEventToAllPeers(Metainfo metainfo, StructBase structBase);

        /**
         * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
         * @param path is the path that shall be called at the remote entity
         * @param structBase is the event message to send (generated code of fmq file).
         */
        void SendEventToAllPeers(string path, StructBase structBase);

        /**
         * @brief sendEventToAllPeers sends an event to all connected peers and does not expect a reply.
         * This method allows message exchange with metainfo. Metainfo is very similar to HTTP headers.
         * You can use it to exchange additional data besides the message data.
         * @param path is the path that shall be called at the remote entity
         * @param metainfo is a key/value map of additional data besides the request data. Metainfo is very similar to HTTP headers.
         * @param structBase is the event message to send (generated code of fmq file).
         */
        void SendEventToAllPeers(string path, Metainfo metainfo, StructBase structBase);

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
        void RegisterCommand<R>(FuncCommandR<R> funcCommand) where R : StructBase
        {
            //FuncCommand f = new FuncCommand((RequestContext requestContext, StructBase structBase) => {
            //    R? request = structBase as R;
            //    funcCommand(requestContext, request!); 
            //});
            FuncCommand? f = funcCommand as FuncCommand;
            RegisterCommandFunction(typeof(R).FullName!, typeof(R).FullName!, f!);
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
        void RegisterCommand<R>(string path, FuncCommandR<R> funcCommand) where R : StructBase
        {
            //FuncCommand f = new FuncCommand((RequestContext requestContext, StructBase structBase) => {
            //    R? request = structBase as R;
            //    funcCommand(requestContext, request!); 
            //});
            FuncCommand? f = funcCommand as FuncCommand;
            RegisterCommandFunction(path, typeof(R).FullName!, f!);
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
        PeerId Connect(SessionInfo session, string entityName, FuncReplyConnect? funcReplyConnect = null);

        /**
         * @brief connect the entity with a remote entity. This entity-to-entity connection is represented by a peer ID.
         * You can connect an entity with multiple remote entities. For each connection you will get a peer ID.
         * To connect to a remote entity, you have to pass a session and the ID of the remote entity.
         * @param session is the session that is used for this entity-to-entity connection.
         * @param entityId is the ID of the remote entity to which this entity shall be connected.
         * @param funcReplyConnect is the callback for this connection, it will indicate if the connection was successful.
         * @return the peer ID. Use this ID to send requests/events to the remote entity.
         */
        PeerId Connect(SessionInfo session, EntityId entityId, FuncReplyConnect? funcReplyConnect = null);

        /**
         * @brief disconnect releases the entity-to-entity connection. Open requests which were not answered, yet, will be
         * answered with status Status::STATUS_PEER_DISCONNECTED.
         * @param peerId to identify which entity-to-entity connection shall be released.
         */
        void Disconnect(PeerId peerId);

        /**
         * @brief getAllPeers gets all entity-to-entity connections of this entity.
         * @return a vector of peer IDs.
         */
        IList<PeerId> AllPeers { get; }

        /**
         * @brief registerPeerEvent registers a callback that will be triggered whenever an entity-to-entity connection
         * is established or released.
         * @param funcPeerEvent the callback.
         */
        void RegisterPeerEvent(FuncPeerEvent funcPeerEvent);

        /**
         * @brief getEntityId returns the entity ID of this entity.
         * @return the entity ID.
         */
        EntityId EntityId { get; }

        /**
         * @brief getSession returns the session which is used for the entity-to-entity connection of a certain peer ID.
         * @param peerId the peer ID.
         * @return the session of the peer ID.
         */
        SessionInfo GetSession(PeerId peerId);

        // low level methods

        /**
         * @brief createPeer creates an entity-to-entity connection (peer) without a session and entity identifier.
         * Use this method, when you want to send requests without knowing to whom. As soon as you know the session
         * and remote entity, then call connect that has a peerId as a parameter.
         * @param entityContainer an instance of IRemoteEntityContainer 
         * @param funcReplyConnect is the callback that indicates if a later connection was successful.
         * @return the peer ID. Use it to send requests/events and to connect with a session and entity identifier.
         */
        PeerId CreatePeer(IRemoteEntityContainer entityContainer, FuncReplyConnect? funcReplyConnect = null);

        /**
         * @brief connect connects the peer that was created with createPeer().
         * @param peerId is the ID that was returned by createPeer().
         * @param session is the session that is used for this entity-to-entity connection.
         * @param entityName is the name of the remote entity to which this entity shall be connected.
         */
        void Connect(PeerId peerId, SessionInfo session, string entityName);

        /**
         * @brief connect connects the peer that was created with createPeer().
         * @param peerId is the ID that was returned by createPeer().
         * @param session is the session that is used for this entity-to-entity connection.
         * @param entityId is the ID of the remote entity to which this entity shall be connected.
         */
        void Connect(PeerId peerId, SessionInfo session, EntityId entityId);

        /**
         * @brief connect connects the peer that was created with createPeer().
         * @param peerId is the ID that was returned by createPeer().
         * @param session is the session that is used for this entity-to-entity connection.
         * @param entityName is the name of the remote entity to which this entity shall be connected. If
         * the name is empty then the entityId will be used to identify the repote entity.
         * @param entityId is the ID of the remote entity to which this entity shall be connected.
         */
        void Connect(PeerId peerId, SessionInfo session, string entityName, EntityId entityId);

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
        void RegisterCommandFunction(string path, string type, FuncCommand funcCommand);

        /**
         * @brief gets the type of the function, which is defined at path.
         * @param path is the path of the function. Can be adjusted by the call, if the method is relevant for the function.
         * @param method is a http method (GET,POST,...).
         * @return expected type of the function.
         */
        string GetTypeOfCommandFunction(ref string path, string? method = null);

        /**
         * @brief getNextCorrelationId creates a correlation ID that is unique inside the entity.
         * @return the correlation ID.
         */
        CorrelationId NextCorrelationId { get; }

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
        void SendRequest(PeerId peerId, string path, StructBase structBase, CorrelationId correlationId, Metainfo? metainfo = null);

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
        CorrelationId SendRequest(PeerId peerId, string path, StructBase structBase, FuncReply funcReply);

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
        CorrelationId SendRequest(PeerId peerId, string path, Metainfo metainfo, StructBase structBase, FuncReplyMeta funcReply);

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
        CorrelationId SendRequest(PeerId peerId, StructBase structBase, FuncReply funcReply);

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
        CorrelationId SendRequest(PeerId peerId, Metainfo metainfo, StructBase structBase, FuncReplyMeta funcReply);

        /**
        * @brief cancels a reply callback. After calling this function, the expected reply callback will not be called, anymore.
        * Call this function, if e.g. a timeout happened, and you are not interested in the reply, anymore.
        * @param correlationId of the request, which reply callback shall not be called, anymore
        * @return true, if the request was still pending and the reply callback was canceled. false, if the reply callback, was already called or 
        * if it is still running.
        */
        bool cancelReply(CorrelationId correlationId);

        /**
         * @brief registerReplyEvent registers a callback, which is triggered for every received reply.
         * With this callback a match with the correlation ID can be done by the application.
         * @param funcReplyEvent is the callback function.
         */
        void registerReplyEvent(FuncReplyEvent funcReplyEvent);

        /**
        * @brief creates a peer at the own entity, so that the peer will publish events to the session.
        * This can be used e.g. for mqtt sessions.
        * @param entityName is the name of the destination. The mqtt topic will look like this: "/<entityName>/<message type>".
        * @return the created peer id.
        */
        PeerId createPublishPeer(SessionInfo session, string entityName);


        // methods for RemoteEntityContainer
        IRemoteEntityPrivate InterfacePrivate { get; }
    };

    public interface IRemoteEntityPrivate
    {
        // methods for RemoteEntityContainer
        void SessionDisconnected(IProtocolSession session);
        void VirtualSessionDisconnected(IProtocolSession session, string virtualSessionId);
        void ReceivedRequest(ref ReceiveData receiveData);
        void ReceivedReply(ReceiveData receiveData);
        void Deinit();
    }


}   // namespace finalmq
