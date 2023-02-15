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

using System.Diagnostics;

namespace finalmq {

    class RemoteEntity : IRemoteEntity
    {
        public static readonly string FMQ_VIRTUAL_SESSION_ID = "fmq_virtsessid";

        public RemoteEntity()
        {
            m_entityId = Interlocked.Increment(ref m_entityIdNext);

            m_peerManager.SetEntityId(m_entityId);
            RegisterCommand<ConnectEntity>((RequestContext requestContext, ConnectEntity request) => {
                bool added;
                string virtualSessionId = requestContext.VirtualSessionId;
                m_peerManager.AddPeer(requestContext.Session, virtualSessionId, requestContext.EntityId, request.entityName, true, out added, () => {
                    // send reply before the connect peer event is triggered. So that the peer gets first the connect reply and afterwards a possible greeting message maybe triggered by the connect peer event.
                    // no lock for m_entityId and m_entityName, because they are already set in initEntity, they will not be changed, anymore.
                    string ownEntityName;
                    IRemoteEntityContainer ec = requestContext.Session.EntityContainer;
                    if (ec != null)
                    {
                        bool registered = false;
                        ownEntityName = ec.GetEntityName(m_entityId, out registered);
                        Debug.Assert(registered);   // entity is already registered, otherwise the request call could not be received
                        requestContext.Reply(new ConnectEntityReply(m_entityId, ownEntityName));
                    }
                    else
                    {
                        requestContext.Reply(Status.STATUS_ENTITY_NOT_FOUND);
                    }
                });
            });

            RegisterCommand<DisconnectEntity>((RequestContext requestContext, DisconnectEntity request) => {
                PeerId peerId = requestContext.PeerId;
                RemovePeer(peerId, Status.STATUS_PEER_DISCONNECTED);
            });
        }

        ~RemoteEntity()
        {
            Deinit();
        }


        public override void SendEvent(PeerId peerId, StructBase structBase)
        {
            SendRequest(peerId, String.Empty, structBase, IRemoteEntity.CORRELATIONID_NONE);
        }
        public override void SendEvent(PeerId peerId, Metainfo metainfo, StructBase structBase)
        {
            SendRequest(peerId, String.Empty, structBase, IRemoteEntity.CORRELATIONID_NONE, metainfo);
        }
        public override void SendEvent(PeerId peerId, string path, StructBase structBase)
        {
            SendRequest(peerId, path, structBase, IRemoteEntity.CORRELATIONID_NONE);
        }
        public override void SendEvent(PeerId peerId, string path, Metainfo metainfo, StructBase structBase)
        {
            SendRequest(peerId, path, structBase, IRemoteEntity.CORRELATIONID_NONE, metainfo);
        }
        public override void SendEventToAllPeers(StructBase structBase)
        {
            IList<PeerId> peers = AllPeers;
            foreach (var peer in peers)
            {
                SendRequest(peer, String.Empty, structBase, IRemoteEntity.CORRELATIONID_NONE);
            }
        }
        public override void SendEventToAllPeers(Metainfo metainfo, StructBase structBase)
        {
            IList<PeerId> peers = AllPeers;
            foreach (var peer in peers)
            {
                SendRequest(peer, String.Empty, structBase, IRemoteEntity.CORRELATIONID_NONE, metainfo);
            }
        }
        public override void SendEventToAllPeers(string path, StructBase structBase)
        {
            IList<PeerId> peers = AllPeers;
            foreach (var peer in peers)
            {
                SendRequest(peer, path, structBase, IRemoteEntity.CORRELATIONID_NONE);
            }
        }
        public override void SendEventToAllPeers(string path, Metainfo metainfo, StructBase structBase)
        {
            IList<PeerId> peers = AllPeers;
            foreach (var peer in peers)
            {
                SendRequest(peer, path, structBase, IRemoteEntity.CORRELATIONID_NONE, metainfo);
            }
        }
        public override PeerId Connect(SessionInfo session, string entityName, FuncReplyConnect? funcReplyConnect = null)
        {
            return ConnectIntern(session, "", entityName, IRemoteEntity.ENTITYID_INVALID, funcReplyConnect);
        }
        public override PeerId Connect(SessionInfo session, EntityId entityId, FuncReplyConnect? funcReplyConnect = null)
        {
            return ConnectIntern(session, "", "", entityId, funcReplyConnect);
        }
        public override void Disconnect(PeerId peerId)
        {
            SendRequest(peerId, String.Empty, new DisconnectEntity(), IRemoteEntity.CORRELATIONID_NONE);
            RemovePeer(peerId, Status.STATUS_PEER_DISCONNECTED);
        }
        public override IList<PeerId> AllPeers
        {
            get => m_peerManager.AllPeers;
        }
        public override void RegisterPeerEvent(FuncPeerEvent funcPeerEvent)
        {
            m_peerManager.SetPeerEvent(funcPeerEvent);
        }
        public override EntityId EntityId
        {
            get
            {
                return m_entityId;
            }
        }
        public override SessionInfo? GetSession(PeerId peerId)
        {
            return m_peerManager.GetSession(peerId);
        }
        public override PeerId CreatePeer(IRemoteEntityContainer entityContainer, FuncReplyConnect? funcReplyConnect = null)
        {
            PeerId peerId = m_peerManager.AddPeer();
            SendConnectEntity(peerId, entityContainer, funcReplyConnect);
            return peerId;
        }
        public override void Connect(PeerId peerId, SessionInfo session, string entityName)
        {
            ConnectIntern(peerId, session, entityName, IRemoteEntity.ENTITYID_INVALID);
        }
        public override void Connect(PeerId peerId, SessionInfo session, EntityId entityId)
        {
            ConnectIntern(peerId, session, "", entityId);
        }
        public override void Connect(PeerId peerId, SessionInfo session, string entityName, EntityId entityId)
        {
            ConnectIntern(peerId, session, entityName, entityId);
        }
        public override void RegisterCommandFunction(string path, string type, FuncCommand funcCommand)
        {
            FuncCommand func = funcCommand;
            lock (m_mutexFunctions)
            {
                if (path.IndexOf('{') != -1)
                {
                    FunctionVar funcVar = new FunctionVar(type, func, path.Split('/'));
                    m_funcCommandsVar.Add(funcVar);
                }
                else if (path.IndexOf('*') != -1)
                {
                    FunctionVar funcVar = new FunctionVar(type, func, path.Split('/'));
                    m_funcCommandsVarStar.Add(funcVar);
                }
                else
                {
                    m_funcCommandsStatic[path] = new Function(type, func);
                }
            }
        }
        public override string GetTypeOfCommandFunction(ref string path, string? method = null)
        {
            Function? function = GetFunction(path);
            if (function != null && method != null)
            {
                string pathWithMethod = path;
                pathWithMethod += '/';
                pathWithMethod += method;
                function = GetFunction(pathWithMethod);
                if (function != null)
                {
                    path = pathWithMethod;
                }
            }
            if (function != null)
            {
                return function.type;
            }
            return string.Empty;
        }
        public override CorrelationId NextCorrelationId
        {
            get
            {
                CorrelationId id = Interlocked.Increment(ref m_nextCorrelationId);
                return id;
            }
        }
        public override void SendRequest(PeerId peerId, string path, StructBase structBase, CorrelationId correlationId, Metainfo? metainfo = null)
        {
            Status status = Status.STATUS_OK;
            Header header = new Header();
            IProtocolSession? session;
            string? virtualSessionId;
            PeerManager.ReadyToSend readyToSend;

            // the mutex lock is important for RTS_CONNECT_NOT_AVAILABLE / RTS_READY handling. See connectIntern(PeerId ...)
            lock (m_mutex)
            {
                readyToSend = m_peerManager.GetRequestHeader(peerId, path, structBase, correlationId, header, out session, out virtualSessionId);
            }

            if (readyToSend == PeerManager.ReadyToSend.RTS_READY)
            {
                Debug.Assert(session != null);
                Debug.Assert(virtualSessionId != null);
                RemoteEntityFormatRegistry.Instance.Send(session, virtualSessionId, header, new Variant(), structBase, metainfo);
            }
            else if (readyToSend == PeerManager.ReadyToSend.RTS_SESSION_NOT_AVAILABLE)
            {
                // message was stored in getRequestHeader
            }
            else
            {
                status = Status.STATUS_PEER_DISCONNECTED;
            }

            if (status != Status.STATUS_OK)
            {
                ReceiveData receiveData = new ReceiveData();
                receiveData.Header.corrid = correlationId;
                receiveData.Header.status = status;
                if (session != null)
                {
                    IExecutor? executor = session.Executor;
                    if (executor != null)
                    {
                        executor.AddAction(() => {
                            ReceivedReply(receiveData);
                        });
                    }
                    else
                    {
                        ReceivedReply(receiveData);
                    }
                }
                else
                {
                    ReceivedReply(receiveData);
                }
            }
        }
        public override CorrelationId SendRequest(PeerId peerId, string path, StructBase structBase, FuncReply funcReply)
        {
            CorrelationId correlationId = NextCorrelationId;
            lock (m_mutexRequests)
            {
                m_requests.Add(correlationId, new Request(peerId, funcReply));
            }
            SendRequest(peerId, path, structBase, correlationId);
            return correlationId;
        }
        public override CorrelationId SendRequest(PeerId peerId, string path, Metainfo metainfo, StructBase structBase, FuncReplyMeta funcReply)
        {
            CorrelationId correlationId = NextCorrelationId;
            lock (m_mutexRequests)
            {
                m_requests.Add(correlationId, new Request(peerId, funcReply));
            }
            SendRequest(peerId, path, structBase, correlationId, metainfo);
            return correlationId;
        }
        public override CorrelationId SendRequest(PeerId peerId, StructBase structBase, FuncReply funcReply)
        {
            CorrelationId correlationId = NextCorrelationId;
            lock (m_mutexRequests)
            {
                m_requests.Add(correlationId, new Request(peerId, funcReply));
            }
            SendRequest(peerId, string.Empty, structBase, correlationId);
            return correlationId;
        }
        public override CorrelationId SendRequest(PeerId peerId, Metainfo metainfo, StructBase structBase, FuncReplyMeta funcReply)
        {
            CorrelationId correlationId = NextCorrelationId;
            lock (m_mutexRequests)
            {
                m_requests.Add(correlationId, new Request(peerId, funcReply));
            }
            SendRequest(peerId, string.Empty, structBase, correlationId, metainfo);
            return correlationId;
        }
        public override bool CancelReply(CorrelationId correlationId)
        {
            if (correlationId == IRemoteEntity.CORRELATIONID_NONE)
            {
                return false;
            }
            lock (m_mutexRequests)
            {
                if (m_requests.ContainsKey(correlationId))
                {
                    m_requests.Remove(correlationId);
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        public override void RegisterReplyEvent(FuncReplyEvent funcReplyEvent)
        {
            lock (m_mutex)
            {
                m_funcsReplyEvent.Add(funcReplyEvent);
                Interlocked.Increment(ref m_funcsReplyEventChanged);    // todo check memory order
            }
        }
        public override PeerId CreatePublishPeer(SessionInfo session, string entityName)
        {
            bool added = false;
            PeerId peerId = IRemoteEntity.PEERID_INVALID;
            peerId = m_peerManager.AddPeer(session, "", ENTITYID_INVALID, entityName, true, out added, null);
            return peerId;
        }


        // methods for RemoteEntityContainer

        internal override void SessionDisconnected(IProtocolSession session)
        {
            IList<PeerId> peerIds = m_peerManager.GetAllPeersWithSession(session);

            foreach (var peerId in peerIds)
            {
                RemovePeer(peerId, Status.STATUS_SESSION_DISCONNECTED);
            }
        }
        internal override void VirtualSessionDisconnected(IProtocolSession session, string virtualSessionId)
        {
            IList<PeerId> peerIds = m_peerManager.GetAllPeersWithVirtualSession(session, virtualSessionId);

            foreach (var peerId in peerIds)
            {
                RemovePeer(peerId, Status.STATUS_SESSION_DISCONNECTED);
            }
        }
        static readonly string STR_DUMMY = "dummy";
        internal override void ReceivedRequest(ReceiveData receiveData)
        {
            if (receiveData.AutomaticConnect)
            {
                bool added;
                ulong srcid = receiveData.Header.srcid;
                if (srcid == 0)
                {
                    srcid = ENTITYID_INVALID;
                }
                m_peerManager.AddPeer(receiveData.Session!, receiveData.VirtualSessionId, srcid, STR_DUMMY, true, out added, null);
            }

            FuncCommand? func = null;
            RemoteEntity.Function? funcData = GetFunction(receiveData.Header.path, receiveData.Message!.AllMetainfo);
            if (funcData != null)
            {
                func = funcData.func;
                Debug.Assert(func != null);
            }

            RequestContext requestContext = new RequestContext(m_peerManager, m_entityId, receiveData);
            Debug.Assert(requestContext != null);

            if (func != null)
            {
                func(requestContext, receiveData.StructBase!);
            }
            else
            {
                requestContext.Reply(Status.STATUS_REQUEST_NOT_FOUND);
            }
        }

        class ThreadLocalDataReplyEvent
        {
            public long changeId = 0;
            public IList<FuncReplyEvent> funcsReplyEventNoLock = new List<FuncReplyEvent>();
        };
        
        [ThreadStatic]
        static ThreadLocalDataReplyEvent? t_threadLocalDataReplyEvent = null;

        internal override void ReceivedReply(ReceiveData receiveData)
        {
            if (t_threadLocalDataReplyEvent == null)
            {
                t_threadLocalDataReplyEvent = new ThreadLocalDataReplyEvent();
            }
            ThreadLocalDataReplyEvent threadLocalDataReplyEvent = t_threadLocalDataReplyEvent;
            long changeId = Interlocked.Read(ref m_funcsReplyEventChanged);
            if (changeId != threadLocalDataReplyEvent.changeId)
            {
                threadLocalDataReplyEvent.changeId = changeId;
                lock (m_mutex)
                {
                    threadLocalDataReplyEvent.funcsReplyEventNoLock = m_funcsReplyEvent.ToList();
                }
            }
            IList<FuncReplyEvent> funcsReplyEventNoLock = threadLocalDataReplyEvent.funcsReplyEventNoLock;

            bool replyHandled = false;
            foreach (var funcReply in funcsReplyEventNoLock)
            {
                replyHandled = funcReply(receiveData.Header.corrid, receiveData.Header.status, receiveData.Message.AllMetainfo, receiveData.StructBase!);
                if (replyHandled)
                {
                    break;
                }
            }
            if (!replyHandled)
            {
                ReplyReceived(receiveData);
            }

            if (receiveData.Header.status == Status.STATUS_ENTITY_NOT_FOUND &&
                receiveData.Header.srcid != IRemoteEntity.ENTITYID_INVALID)
            {
                Debug.Assert(m_peerManager != null);
                Debug.Assert(receiveData.Session != null);
                string virtualSessionId = receiveData.Message.AllMetainfo[FMQ_VIRTUAL_SESSION_ID];
                PeerId peerId = m_peerManager.GetPeerId(receiveData.Session.SessionId, virtualSessionId, receiveData.Header.srcid, "");
                RemovePeer(peerId, Status.STATUS_PEER_DISCONNECTED);
            }
        }
        internal override void Deinit()
        {
            IList<PeerId> peers = AllPeers;
            foreach (var peer in peers)
            {
                RemovePeer(peer, Status.STATUS_PEER_DISCONNECTED);
            }
        }

        //////////////////////////////////////

        PeerId ConnectIntern(SessionInfo session, string virtualSessionId, string entityName, EntityId entityId, FuncReplyConnect? funcReplyConnect)
        {
            bool added;
            PeerId peerId = m_peerManager.AddPeer(session, virtualSessionId, entityId, entityName, false, out added, null);
            if (added)
            {
                var ec = session.EntityContainer;
                if (ec != null)
                {
                    SendConnectEntity(peerId, ec, funcReplyConnect);
                }
                else
                {
                    if (funcReplyConnect != null)
                    {
                        funcReplyConnect(peerId, Status.STATUS_PEER_DISCONNECTED);
                    }
                }
            }

            return peerId;
        }

        void ConnectIntern(PeerId peerId, SessionInfo session, string entityName, EntityId entityId)
        {
            bool ok = true;
            // the mutex lock is important for RTS_CONNECT_NOT_AVAILABLE / RTS_READY handling. See connectIntern(PeerId ...)
            lock (m_mutex)
            {
                IList<PeerManager.Request>? pendingRequests = m_peerManager.Connect(peerId, session, entityId, entityName);

                if (pendingRequests != null)
                {
                    foreach (var request in pendingRequests)
                    {
                        Header header = new Header();
                        IProtocolSession? sessionRet;
                        string? virtualSessionIdRet;
                        Debug.Assert(request.StructBase != null);
                        PeerManager.ReadyToSend readyToSend = m_peerManager.GetRequestHeader(peerId, "", request.StructBase, request.CorrelationId, header, out sessionRet, out virtualSessionIdRet);

                        if (readyToSend == PeerManager.ReadyToSend.RTS_READY)
                        {
                            Debug.Assert(session != null);
                            Debug.Assert(sessionRet != null);
                            Debug.Assert(virtualSessionIdRet != null);
                            RemoteEntityFormatRegistry.Instance.Send(sessionRet, virtualSessionIdRet, header, new Variant(), request.StructBase);
                        }
                        else
                        {
                            ok = false;
                            break;
                        }
                    }
                }
            }

            if (!ok)
            {
                RemovePeer(peerId, Status.STATUS_SESSION_DISCONNECTED);
            }
        }

        void ReplyReceived(ReceiveData receiveData)
        {
            Request? request;
            lock (m_mutexRequests)
            {
                if (m_requests.TryGetValue(receiveData.Header.corrid, out request))
                {
                    m_requests.Remove(receiveData.Header.corrid);
                }
            }

            if (request != null)
            {
                if (request.FuncReply != null)
                {
                    request.FuncReply(request.PeerId, receiveData.Header.status, receiveData.StructBase);
                }
                else if (request.FuncReplyMeta != null)
                {
                    request.FuncReplyMeta(request.PeerId, receiveData.Header.status, receiveData.Message.AllMetainfo, receiveData.StructBase);
                }
            }
        }

        void RemovePeer(PeerId peerId, Status status)
        {
            if (peerId != IRemoteEntity.PEERID_INVALID)
            {
                bool incoming = false;
                bool found = m_peerManager.RemovePeer(peerId, out incoming);

                if (found)
                {
                    IList<Request> requests = new List<Request>();
                    IList<CorrelationId> requestsToDelete = new List<CorrelationId>();
                    // get pending calls
                    lock (m_mutexRequests)
                    {
                        foreach (var entry in m_requests)
                        {
                            Request request = entry.Value;
                            if (request.PeerId == peerId)
                            {
                                requests.Add(request);
                                requestsToDelete.Add(entry.Key);
                            }
                        }
                        foreach (var key in requestsToDelete)
                        {
                            m_requests.Remove(key);
                        }
                    }

                    // release pending calls
                    foreach (var request in requests)
                    {
                        if (request.FuncReply != null)
                        {
                            request.FuncReply(request.PeerId, status, null);
                        }
                        else if (request.FuncReplyMeta != null)
                        {
                            Metainfo metainfoEmty = new Metainfo();
                            request.FuncReplyMeta(request.PeerId, status, metainfoEmty, null);
                        }
                    }
                }
            }
        }

        void SendConnectEntity(PeerId peerId, IRemoteEntityContainer entityContainer, FuncReplyConnect? funcReplyConnect)
        {
            bool registered = false;
            string ownEntityName = entityContainer.GetEntityName(m_entityId, out registered);

            if (!registered)
            {
                entityContainer.RegisterEntity(this);
            }

            RequestReply<ConnectEntityReply>(peerId, new Metainfo(), new ConnectEntity(ownEntityName), 
                (PeerId peerId, Status status, Metainfo metainfo, ConnectEntityReply? replyReceived) => {
                    if (funcReplyConnect != null)
                    {
                        funcReplyConnect(peerId, status);
                    }
                    if (replyReceived != null)
                    {
                        string virtualSessionId = metainfo[FMQ_VIRTUAL_SESSION_ID];
                        m_peerManager.UpdatePeer(peerId, virtualSessionId, replyReceived.entityId, replyReceived.entityName);
                    }
                    else if (status == Status.STATUS_ENTITY_NOT_FOUND)
                    {
//todo                  string? entityName = m_peerManager.GetEntityName(peerId);
//                      if (entityName != null)
//                      {
//                          streamError << "Entity not found: " << entityName;
//                      }
                        RemovePeer(peerId, status);
                    }
            });
        }

        static readonly string PATH_PREFIX = "PATH_";

        Function? GetFunction(string path, Metainfo? keys = null)
        {
            lock (m_mutexFunctions)
            {
                m_funcCommandsStatic.TryGetValue(path, out var func);
                if (func != null)
                {
                    return func;
                }

                string[] pathEntries = path.Split('/');
                foreach (var funcVar in m_funcCommandsVar)
                {
                    if (funcVar.pathEntries.Length == pathEntries.Length)
                    {
                        bool match = true;
                        int i = 0;
                        foreach (var entry in pathEntries)
                        {
                            string funcVarEntry = funcVar.pathEntries[i];
                            if (funcVarEntry.Length >= 2 && funcVarEntry[0] == '{')
                            {
                                if (keys != null && funcVarEntry.Length >= 3)
                                {
                                    string key = PATH_PREFIX + funcVarEntry.Substring(1, funcVarEntry.Length - 2);
                                    keys[key] = entry;
                                }
                            }
                            else
                            {
                                if (funcVarEntry != entry)
                                {
                                    match = false;
                                    break;
                                }
                            }
                            ++i;
                        }
                        if (match)
                        {
                            return funcVar;
                        }
                    }
                }

                foreach (var funcVar in m_funcCommandsVarStar)
                {
                    bool match = true;
                    int j = 0;
                    for (int i = 0; i < funcVar.pathEntries.Length && match; ++i)
                    {
                        string funcVarEntry = funcVar.pathEntries[i];
                        if (funcVarEntry[0] == '*')
                        {
                            string nextEntry = string.Empty;
                            ++i;
                            bool nextEntryAvailable = (i < funcVar.pathEntries.Length);
                            if (nextEntryAvailable)
                            {
                                nextEntry = funcVar.pathEntries[i];
                            }
                            string value = String.Empty;
                            match = false;
                            for (; j < pathEntries.Length && !match; ++j)
                            {
                                bool matchNextEntry = (nextEntryAvailable && pathEntries[j] == nextEntry);
                                bool matchLastEntry = (!nextEntryAvailable && (j == pathEntries.Length - 1));

                                if (!matchNextEntry)
                                {
                                    if (value.Length != 0)
                                    {
                                        value += '/';
                                    }
                                    value += pathEntries[j];
                                }

                                if (matchNextEntry || matchLastEntry)
                                {
                                    if (keys != null && funcVarEntry.Length >= 3)
                                    {
                                        string key = PATH_PREFIX + funcVarEntry.Substring(1, funcVarEntry.Length - 2);
                                        keys[key] = value;
                                    }
                                    match = true;
                                }
                            }
                        }
                        else
                        {
                            if (funcVarEntry != pathEntries[j])
                            {
                                match = false;
                            }
                            ++j;
                        }
                    }
                    if (match && j == pathEntries.Length)
                    {
                        return funcVar;
                    }
                }
            }

            return null;
        }

        class Function
        {
            public Function(string t, FuncCommand f)
            {
                type = t;
                func = f;
            }
            public string type = String.Empty;
            public readonly FuncCommand func;
        };

        class FunctionVar : Function
        {
            public FunctionVar(string t, FuncCommand f, string[] e)
                : base(t, f)
            {
                pathEntries = e;
            }
            public readonly string[] pathEntries;
        };

        class Request
        {
            public Request(PeerId peerId, FuncReply func)
            {
                m_peerId = peerId;
                m_func = func;
                m_funcMeta = null;
            }
            public Request(PeerId peerId, FuncReplyMeta func)
            {
                m_peerId = peerId;
                m_func = null;
                m_funcMeta = func;
            }

            public PeerId PeerId
            {
                get => m_peerId;
            }

            public FuncReply? FuncReply
            {
                get => m_func;
            }

            public FuncReplyMeta? FuncReplyMeta
            {
                get => m_funcMeta;
            }

            readonly PeerId m_peerId;
            readonly FuncReply? m_func;
            readonly FuncReplyMeta? m_funcMeta;
        };

        EntityId m_entityId =  IRemoteEntity.ENTITYID_INVALID;
        static EntityId m_entityIdNext = 0; // atomic

        IList<FuncReplyEvent> m_funcsReplyEvent = new List<FuncReplyEvent>();
        long m_funcsReplyEventChanged = 0;  // atomic
        IDictionary<CorrelationId, Request> m_requests = new Dictionary<CorrelationId, Request>();
        IDictionary<string, Function> m_funcCommandsStatic = new Dictionary<string, Function>();
        IList<FunctionVar> m_funcCommandsVar = new List<FunctionVar>();
        IList<FunctionVar> m_funcCommandsVarStar = new List<FunctionVar>();
        readonly PeerManager m_peerManager = new PeerManager();
        ulong m_nextCorrelationId = 1;   // atomic
        readonly object m_mutex = new object();
        readonly object m_mutexRequests = new object();
        readonly object m_mutexFunctions = new object();
    }
}   // namespace finalmq
