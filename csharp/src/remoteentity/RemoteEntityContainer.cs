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

    public enum ConnectionEvent
    {
        CONNECTIONEVENT_CONNECTED = 0,
        CONNECTIONEVENT_DISCONNECTED = 1,
        CONNECTIONEVENT_SOCKET_CONNECTED = 2,
        CONNECTIONEVENT_SOCKET_DISCONNECTED = 3,
    };

    public delegate void FuncConnectionEvent(SessionInfo session, ConnectionEvent connectionEvent);

    public interface IRemoteEntityContainer : IDisposable
    {
        int CheckReconnectInterval { get; set; }

        /**
         * @brief bind opens a listener socket.
         * @param endpoint defines the type of socket, port or socket name, protocol and the data encoding.
         * examples:
         * The space between // and * is in the comment, because
         * "tcp://*:3000/httpserver:json" = tcp socket, port 3000, http protocol, json data encoding
         * "ipc://my_uds/delimiter_lf:json" = unix domain socket, name my_uds, delimiter linefeed protocol, json data encoding
         * "tcp://*:3000/headersize:protobuf" = tcp socket, port 3000, 4 bytes payload size in header, protobuf data encoding
         * @param bindProperties contains properties for the bind, e.g. SSL/TLS properties.
         */
        void Bind(string endpoint, BindProperties? bindProperties = null);

        /**
         * @brief unbind removes the listening socket
         * @param endpoint must match with the endpoint from the bind call.
         */
        void Unbind(string endpoint);

        /**
         * @brief connect to a peer endpoint.
         * @param endpoint defines the type of socket, hostname or IP address, port or socket name, protocol and the data encoding.
         * examples:
         * The space between // and * is in the comment, because
         * "tcp://localhost:3000/httpserver:json" = tcp socket, port 3000, http protocol, json data encoding
         * "ipc://my_uds/delimiter_lf:json" = unix domain socket, name my_uds, delimiter linefeed protocol, json data encoding
         * "tcp://127.0.0.1:3000/headersize:protobuf" = tcp socket, port 3000, 4 bytes payload size in header, protobuf data encoding
         * @param connectProperties contains properties for the connect, e.g. SSL/TLS properties.
         * @return the session.
         */
        SessionInfo Connect(string endpoint, ConnectProperties? connectProperties = null);

        /**
         * @brief Executor gets the executor.
         * Use the executor to run code in the executor context.
         * Example:
         * entityContainer.Executor.AddAction([]() {
         *     // put here application code that will be executed in the context of the executor
         * });
         * @return the executor.
         */
        IExecutor? Executor { get; }

        /**
         * @brief registerEntity registers an entity. Multiple entities can be registered.
         * @param remoteEntity the entity that shall be registered.
         * @param name is the name, how a peer can find the entity.
         * @return the entity ID.
         */
        EntityId RegisterEntity(IRemoteEntity remoteEntity, string name = "");

        /**
         * @brief unregisterEntity unregisters an entity.
         * @param entityId is the entity ID that shall be unregistered.
         */
        void UnregisterEntity(EntityId entityId);

        /**
         * @brief registerConnectionEvent registers a function that will be called whenever a connection state has changed.
         * @param funcConnectionEvent is the callback function.
         */
        void RegisterConnectionEvent(FuncConnectionEvent funcConnectionEvent);

        /**
         * @brief getAllEntities gets all entities as IDs.
         * @return a list of entities.
         */
        IList<EntityId> AllEntities { get; } 

        /**
         * @brief getEntity gets an entity object by its ID.
         * @return the entity.
         */
        IRemoteEntity? GetEntity(EntityId entityId);

        /**
         * @brief gets the name of an entity
         * @param entityId the ID of the entity
         * @param registered outputs true, if the entity was registered
         * @return the name of the entity
         */
        string GetEntityName(EntityId entityId, out bool registered);
    }

    public class RemoteEntityContainer : IRemoteEntityContainer
                                       , IProtocolSessionCallback
    {
        public RemoteEntityContainer(IExecutor? executor = null, bool storeRawDataInReceiveStruct = false)
        {
            m_storeRawDataInReceiveStruct = storeRawDataInReceiveStruct;
            m_protocolSessionContainer = new ProtocolSessionContainer(executor);
        }

        ~RemoteEntityContainer()
        {
            Dispose(false);
        }

        private void Dispose(bool disposing)
        {
            if (m_disposed)
            {
                return;
            }
            m_disposed = true;

            if (disposing)
            {
                Deinit();
                m_protocolSessionContainer.Dispose();
            }
        }

        public void Dispose()
        {
            lock (m_mutex)
            {
                Dispose(true);
            }
            GC.SuppressFinalize(this);
        }

        void Deinit()
        {
            IList<IRemoteEntity> entities = new List<IRemoteEntity>();
            lock (m_mutex)
            {
                foreach (var entry in m_entityId2entity)
                {
                    entities.Add(entry.Value);
                }
                m_name2entity.Clear();
                m_entityId2entity.Clear();
                Interlocked.Increment(ref m_entitiesChanged);
            }

            foreach (var entity in entities)
            {
                entity.Deinit();
            }
        }

        // IRemoteEntityContainer
        public int CheckReconnectInterval 
        { 
            get => m_protocolSessionContainer.CheckReconnectInterval;
            set => m_protocolSessionContainer.CheckReconnectInterval = value; 
        }

        static string EndpointToProtocolEndpoint(string endpoint, out string contentTypeName)
        {
            int ixEndpoint = endpoint.IndexOf(':');
            if (ixEndpoint == -1)
            {
                contentTypeName = "";
                return "";
            }
            contentTypeName = endpoint.Substring(ixEndpoint + 1, endpoint.Length - (ixEndpoint + 1));
            return endpoint.Substring(0, ixEndpoint);
        }

        void SubscribeEntityNames(IProtocolSession session)
        {
            IList<string> subscribtions = new List<string>();
            lock (m_mutex)
            {
                foreach (var entry in m_name2entity)
                {
                    string subscribtion = entry.Key;
                    if (subscribtion.Length >= 1 && subscribtion[subscribtion.Length - 1] != '*')
                    {
                        subscribtions.Add(entry.Key + "/#");
                    }
                }
            }

            session.Subscribe(subscribtions);
        }

        void SubscribeSessions(string name)
        {
            if (name.Length == 0)
            {
                return;
            }

            if (name[name.Length - 1] == '*')
            {
                return;
            }

            name = name + "/#";

            IList<IProtocolSession> sessions = m_protocolSessionContainer.AllSessions;
            foreach (var session in sessions)
            {
                session.Subscribe(new List<string>{ name });
            }
        }


        public void Bind(string endpoint, BindProperties? bindProperties = null)
        {
            string contentTypeName;
            string endpointProtocol = EndpointToProtocolEndpoint(endpoint, out contentTypeName);
            if (endpointProtocol.Length == 0)
            {
                throw new ArgumentException("Wrong syntax in endpoint: " + endpoint);
            }
            int contentType = RemoteEntityFormatRegistry.Instance.GetContentType(contentTypeName);
            if (contentType == 0)
            {
                throw new ArgumentException("ContentType not found: " + contentTypeName);
            }

            m_protocolSessionContainer.Bind(endpointProtocol, this, bindProperties, contentType);
        }
        public void Unbind(string endpoint)
        {
            string contentTypeName;
            string endpointProtocol = EndpointToProtocolEndpoint(endpoint, out contentTypeName);
            m_protocolSessionContainer.Unbind(endpointProtocol);
        }
        public SessionInfo Connect(string endpoint, ConnectProperties? connectProperties = null)
        {
            int ixEndpoint = endpoint.IndexOf(':');
            if (ixEndpoint == -1)
            {
                throw new ArgumentException("Wrong syntax in endpoint: " + endpoint);
            }
            string contentTypeName = endpoint.Substring(ixEndpoint + 1, endpoint.Length - (ixEndpoint + 1));
            int contentType = RemoteEntityFormatRegistry.Instance.GetContentType(contentTypeName);
            if (contentType == 0)
            {
                throw new ArgumentException("ContentType not found: " + contentTypeName);
            }
            string endpointProtocol = endpoint.Substring(0, ixEndpoint);

            IProtocolSession session = m_protocolSessionContainer.Connect(endpointProtocol, this, connectProperties, contentType);
            SubscribeEntityNames(session);
            return new SessionInfo(this, session);
        }
        public IExecutor? Executor 
        {
            get => m_protocolSessionContainer.Executor;
        }
        public EntityId RegisterEntity(IRemoteEntity remoteEntity, string name = "")
        {
            // remove last '/', if available
            if (name.Length != 0 && name[name.Length - 1] == '/')
            {
                name = name.Substring(0, name.Length - 1);
            }

            IRemoteEntity re = remoteEntity;
            if (re == null)
            {
                throw new ArgumentException("entity is invalid");
            }

            EntityId entityId = IRemoteEntity.ENTITYID_INVALID;
            lock (m_mutex)
            {
                if ((name.Length != 0) && m_name2entity.ContainsKey(name))
                {
                    throw new ArgumentException("Entity with name " + name + " was already registered");
                }

                entityId = re.EntityId;

                m_entityId2name[entityId] = name;

                if (name.Length != 0)
                {
                    m_name2entity[name] = remoteEntity;
                }

                m_entityId2entity[entityId] = remoteEntity;
                Interlocked.Increment(ref m_entitiesChanged);
            }

            if (name.Length != 0)
            {
                SubscribeSessions(name);
            }

            return entityId;
        }
        public void UnregisterEntity(EntityId entityId)
        {
            lock (m_mutex)
            {
                IList<string> entitiesToRemove = new List<string>();
                foreach (var entry in m_name2entity)
                {
                    IRemoteEntity remoteEntity = entry.Value;
                    if (remoteEntity.EntityId == entityId)
                    {
                        entitiesToRemove.Add(entry.Key);
                    }
                }
                foreach (var name in entitiesToRemove)
                {
                    m_name2entity.Remove(name);
                }
                m_entityId2entity.Remove(entityId);
            }
        }
        public void RegisterConnectionEvent(FuncConnectionEvent funcConnectionEvent)
        {
            lock (m_mutex)
            {
                m_funcConnectionEvent = funcConnectionEvent;
            }
        }
        public IList<EntityId> AllEntities 
        { 
            get 
            {
                IList<EntityId> entities = new List<EntityId>();
                lock (m_mutex)
                {
                    foreach (var entry in m_entityId2entity)
                    {
                        entities.Add(entry.Key);
                    }
                }
                return entities;
            }
        }
        public IRemoteEntity? GetEntity(EntityId entityId)
        {
            lock (m_mutex)
            {
                m_entityId2entity.TryGetValue(entityId, out var entity);
                return entity;
            }
        }
        public string GetEntityName(EntityId entityId, out bool registered)
        {
            registered = false;
            lock (m_mutex)
            {
                if (m_entityId2name.TryGetValue(entityId, out var name))
                {
                    registered = true;
                    return name;
                }
            }
            return "";
        }

        // IProtocolSessionCallback

        public void Connected(IProtocolSession session)
        {
            TriggerConnectionEvent(new SessionInfo(this, session), ConnectionEvent.CONNECTIONEVENT_CONNECTED);
        }
        public void Disconnected(IProtocolSession session)
        {
            TriggerConnectionEvent(new SessionInfo(this, session), ConnectionEvent.CONNECTIONEVENT_DISCONNECTED);

            IList<IRemoteEntity> entities = new List<IRemoteEntity>();
            lock (m_mutex)
            {
                foreach (var entry in m_entityId2entity)
                {
                    entities.Add(entry.Value);
                }
            }

            foreach (var entity in entities)
            {
                entity.SessionDisconnected(session);
            }
        }
        public void DisconnectedVirtualSession(IProtocolSession session, string virtualSessionId)
        {
            IList<IRemoteEntity> entities = new List<IRemoteEntity>();
            lock (m_mutex)
            {
                foreach (var entry in m_entityId2entity)
                {
                    entities.Add(entry.Value);
                }
            }

            foreach (var entity in entities)
            {
                entity.VirtualSessionDisconnected(session, virtualSessionId);
            }
        }


        class ThreadLocalDataEntities
        {
            public long changeId = 0;
            public IDictionary<string, IRemoteEntity> name2entityNoLock = new Dictionary<string, IRemoteEntity>();
            public IDictionary<EntityId, IRemoteEntity> entityId2entityNoLock = new Dictionary<EntityId, IRemoteEntity>();
        };
        [ThreadStatic]
        static ThreadLocalDataEntities? t_threadLocalDataEntities = null;


        public void Received(IProtocolSession session, IMessage message)
        {
            if (t_threadLocalDataEntities == null)
            {
                t_threadLocalDataEntities = new ThreadLocalDataEntities();
            }

            ThreadLocalDataEntities threadLocalDataEntities = t_threadLocalDataEntities;
            long changeId = Interlocked.Read(ref m_entitiesChanged);
            if (changeId != threadLocalDataEntities.changeId)
            {
                threadLocalDataEntities.changeId = changeId;
                lock (m_mutex)
                {
                    threadLocalDataEntities.name2entityNoLock = new Dictionary<string, IRemoteEntity>(m_name2entity);
                    threadLocalDataEntities.entityId2entityNoLock = new Dictionary<EntityId, IRemoteEntity>(m_entityId2entity);
                }
            }
            IDictionary<string, IRemoteEntity> name2entityNoLock = threadLocalDataEntities.name2entityNoLock;
            IDictionary<EntityId, IRemoteEntity> entityId2entityNoLock = threadLocalDataEntities.entityId2entityNoLock;

            int formatStatus = 0;
            ReceiveData receiveData = new ReceiveData(new SessionInfo(this, session), message);
            if (!session.DoesSupportMetainfo())
            {
                receiveData.StructBase = RemoteEntityFormatRegistry.Instance.Parse(session, message, m_storeRawDataInReceiveStruct, name2entityNoLock, receiveData.Header, out formatStatus);
            }
            else
            {
                receiveData.StructBase = RemoteEntityFormatRegistry.Instance.ParseHeaderInMetainfo(session, message, m_storeRawDataInReceiveStruct, name2entityNoLock, receiveData.Header, out formatStatus);
            }

            EntityId entityId = receiveData.Header.destid;
            IRemoteEntity? remoteEntity = null;
            if (entityId == IRemoteEntity.ENTITYID_INVALID || (entityId == IRemoteEntity.ENTITYID_DEFAULT && (receiveData.Header.destname.Length != 0)))
            {
                name2entityNoLock.TryGetValue(receiveData.Header.destname, out remoteEntity);
                if (remoteEntity == null)
                {
                    name2entityNoLock.TryGetValue("*", out remoteEntity);
                }
            }
            if (remoteEntity == null && entityId != IRemoteEntity.ENTITYID_INVALID)
            {
                entityId2entityNoLock.TryGetValue(entityId, out remoteEntity);
            }

            string type = receiveData.Header.type;

            if (receiveData.Header.mode == MsgMode.MSG_REQUEST)
            {
                Metainfo metainfo = receiveData.Message.AllMetainfo;
                if (metainfo.Count != 0)
                {
                    metainfo.TryGetValue(RemoteEntity.FMQ_VIRTUAL_SESSION_ID, out var virtualSessionId);
                    if (virtualSessionId != null)
                    {
                        receiveData.VirtualSessionId = virtualSessionId;
                    }
                }
                receiveData.AutomaticConnect = ((formatStatus & (int)FormatStatus.FORMATSTATUS_AUTOMATIC_CONNECT) != 0);
                Status replyStatus = Status.STATUS_OK;
                if ((formatStatus & (int)FormatStatus.FORMATSTATUS_SYNTAX_ERROR) == 0)
                {
                    if (remoteEntity != null)
                    {
                        remoteEntity.ReceivedRequest(receiveData);
                    }
                    else
                    {
                        replyStatus = Status.STATUS_ENTITY_NOT_FOUND;
                    }
                }
                else
                {
                    replyStatus = Status.STATUS_SYNTAX_ERROR;
                }
                if (replyStatus != Status.STATUS_OK)
                {
                    if (remoteEntity != null && (entityId == IRemoteEntity.ENTITYID_INVALID || entityId == IRemoteEntity.ENTITYID_DEFAULT))
                    {
                        entityId = remoteEntity.EntityId;
                    }
                    Header headerReply = new Header(receiveData.Header.srcid, "", entityId, MsgMode.MSG_REPLY, replyStatus, "", "", receiveData.Header.corrid, Array.Empty<string>() );
                    RemoteEntityFormatRegistry.Instance.Send(session, receiveData.VirtualSessionId, headerReply, message.EchoData);
                }
            }
            else if (receiveData.Header.mode == MsgMode.MSG_REPLY)
            {
                if (remoteEntity != null)
                {
                    if ((receiveData.StructBase != null) && (receiveData.Header.status == Status.STATUS_OK) && (type.Length != 0))
                    {
                        receiveData.Header.status = Status.STATUS_REPLYTYPE_NOT_KNOWN;
                    }
                    remoteEntity.ReceivedReply(receiveData);
                }
            }
        }
        public void SocketConnected(IProtocolSession session)
        {
            TriggerConnectionEvent(new SessionInfo(this, session), ConnectionEvent.CONNECTIONEVENT_SOCKET_CONNECTED);
        }
        public void SocketDisconnected(IProtocolSession session)
        {
            TriggerConnectionEvent(new SessionInfo(this, session), ConnectionEvent.CONNECTIONEVENT_SOCKET_DISCONNECTED);
        }


        void TriggerConnectionEvent(SessionInfo session, ConnectionEvent connectionEvent)
        {
            FuncConnectionEvent? funcConnectionEvent = m_funcConnectionEvent;
            if (funcConnectionEvent != null)
            {
                funcConnectionEvent(session, connectionEvent);
            }
        }

        IProtocolSessionContainer m_protocolSessionContainer;
        IDictionary<string, IRemoteEntity> m_name2entity = new Dictionary<string, IRemoteEntity>();
        IDictionary<EntityId, IRemoteEntity> m_entityId2entity = new Dictionary<EntityId, IRemoteEntity>();
        IDictionary<EntityId, string> m_entityId2name = new Dictionary<EntityId, string>();
        FuncConnectionEvent? m_funcConnectionEvent = null;
        bool m_storeRawDataInReceiveStruct = false;

        long m_entitiesChanged = 0; //atomic
        bool m_disposed = false;

        readonly object m_mutex = new object();
    }
}   // namespace finalmq
