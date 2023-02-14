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

    public interface IRemoteEntityContainer
    {
        /**
         * @brief init initializes the instance. Call it once after constructing the object.
         * @param executor decouples the callback thread context from the polling thread.
         * @param cycleTime is a timer interval in [ms] in which the poller thread will trigger a timer callback (see parameter funcTimer).
         * @param funcTimer is a callback that is been called every cycleTime.
         * @param storeRawDataInReceiveStruct is a flag. It is usually false. But if you wish to have the raw data inside a message struct, then you can set this flag to true.
         * @param checkReconnectInterval is the timer interval in [ms] in which the reconnect timers will be checked (the reconnect timers are not checked every cycleTime). Unit tests which test reconnection, set this parameter to 1ms to have faster tests.
         */
        void Init(IExecutor? executor = null, int cycleTime = 100, FuncTimer? funcTimer = null, bool storeRawDataInReceiveStruct = false, int checkReconnectInterval = 1000);

        /**
         * @brief bind opens a listener socket.
         * @param endpoint defines the type of socket, port or socket name, protocol and the data encoding.
         * examples:
         * The space between // and * is in the comment, because
         * "tcp://*:3000/httpserver:json" = tcp socket, port 3000, http protocol, json data encoding
         * "ipc://my_uds/delimiter_lf:json" = unix domain socket, name my_uds, delimiter linefeed protocol, json data encoding
         * "tcp://*:3000/headersize:protobuf" = tcp socket, port 3000, 4 bytes payload size in header, protobuf data encoding
         * @param bindProperties contains properties for the bind, e.g. SSL/TLS properties.
         * @return 0 on success and -1 on failure.
         */
        int Bind(string endpoint, BindProperties? bindProperties = null);

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
         * @brief Executor gets the executor (there is also an executor in case if you do not pass an executor at init()).
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
    {
        public RemoteEntityContainer()
        {
            m_protocolSessionContainer = new ProtocolSessionContainer();
            m_executor = m_protocolSessionContainer.Executor;
        }

        ~RemoteEntityContainer()
        {
            Deinit();
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



        public void Init(IExecutor? executor = null, int cycleTime = 100, FuncTimer? funcTimer = null, bool storeRawDataInReceiveStruct = false, int checkReconnectInterval = 1000)
        {

        }
        public int Bind(string endpoint, BindProperties? bindProperties = null)
        {
            return 0;
        }
        public void Unbind(string endpoint)
        {

        }
        public SessionInfo Connect(string endpoint, ConnectProperties? connectProperties = null)
        {
            return null;
        }
        public IExecutor? Executor 
        {
            get { return null; } 
        }
        public EntityId RegisterEntity(IRemoteEntity remoteEntity, string name = "")
        {
            return 0;
        }
        public void UnregisterEntity(EntityId entityId)
        {

        }
        public void RegisterConnectionEvent(FuncConnectionEvent funcConnectionEvent)
        {

        }
        public IList<EntityId> AllEntities 
        { 
            get { return null; } 
        }
        public IRemoteEntity? GetEntity(EntityId entityId)
        {
            return null;
        }
        public string GetEntityName(EntityId entityId, out bool registered)
        {
            registered = false;
            return "";
        }


        IProtocolSessionContainer m_protocolSessionContainer;
        IDictionary<string, IRemoteEntity> m_name2entity = new Dictionary<string, IRemoteEntity>();
        IDictionary<EntityId, IRemoteEntity> m_entityId2entity = new Dictionary<EntityId, IRemoteEntity>();
        IDictionary<EntityId, string> m_entityId2name = new Dictionary<EntityId, string>();
        FuncConnectionEvent m_funcConnectionEvent;
        bool m_storeRawDataInReceiveStruct = false;
        //std::chrono::time_point<std::chrono::steady_clock> m_lastCheckTime;
        readonly IExecutor m_executor;

        long m_entitiesChanged = 0; //atomic

        readonly object m_mutex = new object();
    }
}   // namespace finalmq
