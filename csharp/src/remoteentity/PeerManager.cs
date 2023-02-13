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

    internal class PeerManager
    {
        public enum ReadyToSend
        {
            RTS_READY,
            RTS_SESSION_NOT_AVAILABLE,
            RTS_PEER_NOT_AVAILABLE,
        };

        class Request
        {
            public Request(StructBase? structBase, CorrelationId correlationId)
            {
                m_structBase = structBase;
                m_correlationId = correlationId;
            }
            public StructBase? StructBase
            { 
                get { return m_structBase; }
            }
            public CorrelationId CorrelationId
            {
                get { return m_correlationId; }
            }
            StructBase? m_structBase = null;
            CorrelationId m_correlationId = IRemoteEntity.CORRELATIONID_NONE;
        };

        public PeerManager()
        {

        }
        public IList<PeerId> AllPeers
        {
            get 
            {
                IList<PeerId> peers = new List<PeerId>();
                lock (m_mutex)
                {
                    foreach (var peer in m_peers)
                    {
                        peers.Add(peer.Key);
                    }
                }
                return peers;
            }
        }
        public IList<PeerId> GetAllPeersWithSession(IProtocolSession session)
        {
            IList<PeerId> peers = new List<PeerId>();
            lock (m_mutex)
            {
                foreach (var peer in m_peers)
                {
                    SessionInfo? sessionInfo = peer.Value.Session;
                    if (sessionInfo != null && sessionInfo.Session == session)
                    {
                        peers.Add(peer.Key);
                    }
                }
            }
            return peers;

        }
        public IList<PeerId> GetAllPeersWithVirtualSession(IProtocolSession session, string virtualSessionId)
        {
            IList<PeerId> peers = new List<PeerId>();
            lock (m_mutex)
            {
                foreach (var peer in m_peers)
                {
                    SessionInfo? sessionInfo = peer.Value.Session;
                    if (sessionInfo != null && sessionInfo.Session == session && peer.Value.VirtualSessionId == virtualSessionId)
                    {
                        peers.Add(peer.Key);
                    }
                }
            }
            return peers;
        }

        void UpdatePeer(PeerId peerId, string virtualSessionId, EntityId entityId, string entityName)
        {
            FuncPeerEvent? funcPeerEvent = null;
            Peer? peer = null;
            lock (m_mutex)
            {
                peer = GetPeer(peerId);
                if (peer != null)
                {
                    peer.EntityId = entityId;
                    peer.EntityName = entityName;
                    Debug.Assert(peer.Session != null);
                    long sessionId = peer.Session.SessionId;

                    if (peer.VirtualSessionId != virtualSessionId)
                    {
                        Debug.Assert(peer.VirtualSessionId.Length == 0);
                        m_sessionEntityToPeerId[sessionId][virtualSessionId] = m_sessionEntityToPeerId[sessionId][peer.VirtualSessionId];
                        m_sessionEntityToPeerId[sessionId].Remove(peer.VirtualSessionId);
                        peer.VirtualSessionId = virtualSessionId;
                    }

                    if (entityId != IRemoteEntity.ENTITYID_INVALID)
                    {
                        m_sessionEntityToPeerId[sessionId][peer.VirtualSessionId].Item1[entityId] = peerId;
                    }
                    if (entityName.Length != 0)
                    {
                        m_sessionEntityToPeerId[sessionId][peer.VirtualSessionId].Item2[entityName] = peerId;
                    }
                    funcPeerEvent = m_funcPeerEvent;
                }
            }
            // fire peer event CONNECTED
            if (funcPeerEvent != null && peer != null)
            {
                Debug.Assert(peer.Session != null);
                funcPeerEvent(peerId, peer.Session, entityId, PeerEvent.PEER_CONNECTED, false);
            }
        }

        public bool RemovePeer(PeerId peerId, out bool incoming)
        {
            bool found = false;
            incoming = false;
            FuncPeerEvent? funcPeerEvent = null;
            Peer? peer = null;
            SessionInfo? sessionInfo = null;
            lock (m_mutex)
            {
                peer = GetPeer(peerId);
                if (peer != null)
                {
                    sessionInfo = peer.Session;
                    if (sessionInfo != null)
                    {
                        found = true;
                        RemovePeerFromSessionEntityToPeerId(sessionInfo.SessionId, peer.VirtualSessionId, peer.EntityId, peer.EntityName);
                        incoming = peer.Incoming;
                        m_peers.Remove(peerId);

                        funcPeerEvent = m_funcPeerEvent;
                    }
                }
            }
            // fire peer event DISCONNECTED
            if (funcPeerEvent != null && peer != null && sessionInfo != null)
            {
                funcPeerEvent(peerId, sessionInfo, peer.EntityId, PeerEvent.PEER_DISCONNECTED, incoming);
            }
            return found;
        }

        public PeerId GetPeerId(long sessionId, string virtualSessionId, EntityId entityId, string entityName)
        {
            lock (m_mutex)
            {
                return GetPeerIdIntern(sessionId, virtualSessionId, entityId, entityName);
            }
        }

        public ReadyToSend GetRequestHeader(PeerId peerId, string path, StructBase structBase, CorrelationId correlationId, Header header, out IProtocolSession? session, out string? virtualSessionId)
        {
            ReadyToSend readyToSend = ReadyToSend.RTS_PEER_NOT_AVAILABLE;
            session = null;
            virtualSessionId = null;

            lock (m_mutex)
            {
                Peer? peer = GetPeer(peerId);
                if (peer != null)
                {
                    if (peer.Session != null)
                    {
                        session = peer.Session.Session;
                        virtualSessionId = peer.VirtualSessionId;
                        string? typeName = structBase.GetRawType();
                        if (typeName == null)
                        {
                            typeName = structBase.GetType().FullName;
                        }
                        Debug.Assert(typeName != null);
                        header.destid = peer.EntityId;
                        header.destname = (peer.EntityId == IRemoteEntity.ENTITYID_INVALID) ? peer.EntityName : "";
                        header.srcid = m_entityId;
                        header.mode = MsgMode.MSG_REQUEST;
                        header.status = Status.STATUS_OK;
                        header.path = path;
                        header.type = typeName;
                        header.corrid = correlationId;

                        readyToSend = ReadyToSend.RTS_READY;
                    }
                    else
                    {
                        readyToSend = ReadyToSend.RTS_SESSION_NOT_AVAILABLE;
                        IList<Request>? requests = peer.Requests;
                        if (requests != null)
                        {
                            requests.Add(new Request(structBase, correlationId));
                        }
                    }
                }
            }
            return readyToSend;
        }

        string? GetEntityName(PeerId peerId)
        {
            string? entityName = null;
            lock (m_mutex)
            {
                Peer? peer = GetPeer(peerId);
                if (peer != null)
                {
                    entityName = peer.EntityName;
                }
            }
            return entityName;
        }

        public delegate void FuncAddPeerCallback();

        PeerId AddPeer(SessionInfo session, string virtualSessionId, EntityId entityId, string entityName, bool incoming, bool added, FuncAddPeerCallback? funcBeforeFirePeerEvent)
        {
            added = false;

            PeerId peerId;
            FuncPeerEvent? funcPeerEvent = null;
            lock (m_mutex)
            {
                peerId = GetPeerIdIntern(session.SessionId, virtualSessionId, entityId, entityName);

                if (peerId == IRemoteEntity.PEERID_INVALID)
                {
                    peerId = m_nextPeerId;
                    ++m_nextPeerId;
                    Peer peer = new Peer(session, virtualSessionId, entityId, entityName, incoming);
                    m_peers[peerId] = peer;
                    added = true;
                    long sessionId = session.SessionId;
                    if (entityId != IRemoteEntity.ENTITYID_INVALID)
                    {
                        m_sessionEntityToPeerId[sessionId][peer.VirtualSessionId].Item1[entityId] = peerId;
                    }
                    if (entityName.Length != 0)
                    {
                        m_sessionEntityToPeerId[sessionId][peer.VirtualSessionId].Item2[entityName] = peerId;
                    }

                    funcPeerEvent = m_funcPeerEvent;
                }
            }

            if (funcBeforeFirePeerEvent != null)
            {
                funcBeforeFirePeerEvent();
            }

            // fire peer event CONNECTED
            if (incoming)
            {
                if (funcPeerEvent != null)
                {
                    funcPeerEvent(peerId, session, entityId, PeerEvent.PEER_CONNECTED, incoming);
                }
            }

            return peerId;
        }

        PeerId AddPeer()
        {
            PeerId peerId;
            lock (m_mutex)
            {
                peerId = m_nextPeerId;
                ++m_nextPeerId;
                Peer peer = new Peer();
                m_peers[peerId] = peer;
            }

            return peerId;

        }

        IList<Request>? Connect(PeerId peerId, SessionInfo session, EntityId entityId, string entityName)
        {
            IList<Request>? requests = null;
            lock (m_mutex)
            {
                Peer? peer = GetPeer(peerId);
                if (peer != null)
                {
                    peer.Session = session;
                    peer.EntityId = entityId;
                    peer.EntityName = entityName;
                    long sessionId = session.SessionId;
                    if (entityId != IRemoteEntity.ENTITYID_INVALID)
                    {
                        m_sessionEntityToPeerId[sessionId][peer.VirtualSessionId].Item1[entityId] = peerId;
                    }
                    if (entityName.Length != 0)
                    {
                        m_sessionEntityToPeerId[sessionId][peer.VirtualSessionId].Item2[entityName] = peerId;
                    }

                    requests = peer.Requests;
                    peer.Requests = null;
                }
            }

            return requests;
        }

        void SetEntityId(EntityId entityId)
        {
            m_entityId = entityId;
        }

        void SetPeerEvent(FuncPeerEvent funcPeerEvent)
        {
            lock (m_mutex)
            {
                m_funcPeerEvent = funcPeerEvent;
            }
        }

        //readonly SessionInfo EMPTY_SESSION = new SessionInfo();

        SessionInfo? GetSession(PeerId peerId)
        {
            lock (m_mutex)
            {
                Peer? peer = GetPeer(peerId);
                if (peer != null)
                {
                    return peer.Session;
                }
            }
            return null;
        }

        PeerId GetPeerIdIntern(long sessionId, string virtualSessionId, EntityId entityId, string entityName)
        {
            // mutex already locked
            m_sessionEntityToPeerId.TryGetValue(sessionId, out var session);
            if (session != null)
            {
                session.TryGetValue(virtualSessionId, out var entry);
                if (entry != null)
                {
                    if (entityId != IRemoteEntity.ENTITYID_INVALID)
                    {
                        var entityIdContainer = entry.Item1;
                        if (entityIdContainer.TryGetValue(entityId, out var entity))
                        {
                            return entity;
                        }
                    }
                    else
                    {
                        var entityNameContainer = entry.Item2;
                        if (entityNameContainer.TryGetValue(entityName, out var entity))
                        {
                            return entity;
                        }
                    }
                }
            }
            return IRemoteEntity.PEERID_INVALID;
        }

        void RemovePeerFromSessionEntityToPeerId(long sessionId, string virtualSessionId, EntityId entityId, string entityName)
        {
            // mutex already locked
            // remove from m_sessionEntityToPeerId

            m_sessionEntityToPeerId.TryGetValue(sessionId, out var session);
            if (session != null)
            {
                session.TryGetValue(virtualSessionId, out var entry);
                if (entry != null)
                {
                    var entityIdContainer = entry.Item1;
                    entityIdContainer.Remove(entityId);
                    var entityNameContainer = entry.Item2;
                    entityNameContainer.Remove(entityName);
                    if (entityIdContainer.Count == 0 && entityNameContainer.Count == 0)
                    {
                        session.Remove(virtualSessionId);
                    }
                }
                if (session.Count == 0)
                {
                    m_sessionEntityToPeerId.Remove(sessionId);
                }
            }
        }

        Peer? GetPeer(PeerId peerId)
        {
            m_peers.TryGetValue(peerId, out var peer);
            return peer;
        }

        class Peer
        {
            public Peer()
            {
                m_session = null;
                m_virtualSessionId = "";
                m_entityId = IRemoteEntity.ENTITYID_INVALID;
                m_entityName = "";
                m_incoming = false;
            }
            public Peer(SessionInfo session, string virtualSessionId, EntityId entityId, string entityName, bool incoming)
            {
                m_session = session;
                m_virtualSessionId = virtualSessionId;
                m_entityId = entityId;
                m_entityName = entityName;
                m_incoming = incoming;
            }

            public SessionInfo? Session 
            {
                get { return m_session; }
                set { m_session = value; }
            }
            public string VirtualSessionId 
            {
                get { return m_virtualSessionId; }
                set { m_virtualSessionId = value; }
            }
            public EntityId EntityId 
            {
                get { return m_entityId; }
                set { m_entityId = value; }
            }
            public string EntityName 
            {
                get { return m_entityName; }
                set { m_entityName = value; }
            }
            public bool Incoming { get { return m_incoming;  } }

            public IList<Request>? Requests
            {
                get { return m_requests; }
                set { m_requests = value; }
            }

            SessionInfo? m_session;
            string m_virtualSessionId;
            EntityId m_entityId;
            string m_entityName;
            readonly bool m_incoming;
            IList<Request>? m_requests = new List<Request>();
        };

        EntityId m_entityId = IRemoteEntity.ENTITYID_INVALID;
        FuncPeerEvent? m_funcPeerEvent = null;
        IDictionary<PeerId, Peer> m_peers = new Dictionary<PeerId, Peer>();
        PeerId m_nextPeerId = 1;
        IDictionary<long, IDictionary<string, Tuple<IDictionary<EntityId, PeerId>, IDictionary<string, PeerId>>>> m_sessionEntityToPeerId = new Dictionary<long, IDictionary<string, Tuple<IDictionary<EntityId, PeerId>, IDictionary<string, PeerId>>>>();
        readonly object m_mutex = new object();
    }

}   // namespace finalmq
