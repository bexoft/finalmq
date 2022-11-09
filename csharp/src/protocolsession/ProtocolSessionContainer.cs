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

namespace finalmq 
{

    public delegate void FuncTimer();

    interface IProtocolSessionContainer : IDisposable
    {
        int CheckReconnectInterval { get; set; }
        void Bind(string endpoint, IProtocolSessionCallback callback, BindProperties? bindProperties = null, int contentType = 0);
        void Unbind(string endpoint);
        IProtocolSession Connect(string endpoint, IProtocolSessionCallback callback, ConnectProperties? connectProperties = null, int contentType = 0);
        IProtocolSession CreateSession(IProtocolSessionCallback callback);
        IList<IProtocolSession> GetAllSessions();
        IProtocolSession GetSession(long sessionId);
        IExecutor? GetExecutor();
    }


    class ProtocolBind : IStreamConnectionCallback
    {
        public ProtocolBind(IProtocolSessionCallback callback, IExecutor? executor, IProtocolFactory protocolFactory, IProtocolSessionList protocolSessionList, BindProperties? bindProperties = null, int contentType = 0)
        {
            m_callback = callback;
            m_executor = executor;
            m_protocolFactory = protocolFactory;
            m_protocolSessionList = protocolSessionList;
            m_bindProperties = bindProperties;
            m_contentType = contentType;
        }

        // IStreamConnectionCallback
        public IStreamConnectionCallback? Connected(IStreamConnection connection)
        {
            IProtocol protocol = m_protocolFactory.CreateProtocol(/* todo m_bindProperties.ProtocolData */);
            IProtocolSessionPrivate protocolSession = new ProtocolSession(m_callback, m_executor, protocol, m_protocolSessionList, m_bindProperties, m_contentType);
            protocolSession.SetConnection(connection, !protocol.DoesSupportSession);
            return protocol;
        }

        public void Disconnected(IStreamConnection connection)
        {
            // should never be called, because the callback will be overriden by connected
            Debug.Assert(false);
        }

        public void Received(IStreamConnection connection, byte[] buffer, int count)
        {
            // should never be called, because the callback will be overriden by connected
            Debug.Assert(false);
        }

        readonly IProtocolSessionCallback m_callback;
        readonly IExecutor? m_executor;
        readonly IProtocolFactory m_protocolFactory;
        readonly IProtocolSessionList m_protocolSessionList;
        readonly BindProperties? m_bindProperties;
        readonly int m_contentType;
    }




    class ProtocolSessionContainer : IProtocolSessionContainer
    {
        public ProtocolSessionContainer(IExecutor? executor)
        {
            m_executor = executor;
        }

        ~ProtocolSessionContainer()
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
                IList<string> endpoints = new List<string>();
                foreach (var entry in m_endpoint2Bind)
                {
                    endpoints.Add(entry.Key);
                }
                foreach (var endpoint in endpoints)
                {
                    Unbind(endpoint);
                }
                m_streamConnectionContainer.Dispose();
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

        // IProtocolSessionContainer
        public int CheckReconnectInterval 
        {
            get => m_streamConnectionContainer.CheckReconnectInterval;
            set => m_streamConnectionContainer.CheckReconnectInterval = value;
        }
        public void Bind(string endpoint, IProtocolSessionCallback callback, BindProperties? bindProperties = null, int contentType = 0)
        {
            int ixEndpoint = endpoint.LastIndexOf(':');
            if (ixEndpoint == -1)
            {
                throw new ArgumentException("':' missing in endpoint");
            }
            string protocolName = endpoint.Substring(ixEndpoint + 1, endpoint.Length - (ixEndpoint + 1));
            IProtocolFactory protocolFactory = ProtocolRegistry.Instance.GetProtocolFactory(protocolName);

            string endpointStreamConnection = endpoint.Substring(0, ixEndpoint);

            ProtocolBind? bind = null;
            lock (m_mutex)
            {
                if (!m_endpoint2Bind.ContainsKey(endpoint))
                {
                    bind = new ProtocolBind(callback, m_executor, protocolFactory, m_protocolSessionList, bindProperties, contentType);
                    m_endpoint2Bind[endpoint] = bind;
                }
                else
                {
                    throw new ArgumentException("Endpoint already bound: " + endpoint);
                }
            }
            if (bind != null)
            {
                m_streamConnectionContainer.Bind(endpointStreamConnection, bind, bindProperties);
            }
        }
        public void Unbind(string endpoint)
        {
            lock (m_mutex)
            {
                if (m_endpoint2Bind.ContainsKey(endpoint))
                {
                    m_endpoint2Bind.Remove(endpoint);
                    m_streamConnectionContainer.Unbind(endpoint);
                }
            }
        }
        public IProtocolSession Connect(string endpoint, IProtocolSessionCallback callback, ConnectProperties? connectProperties = null, int contentType = 0)
        {
            int ixEndpoint = endpoint.LastIndexOf(':');
            if (ixEndpoint == -1)
            {
                throw new ArgumentException("':' missing in endpoint");
            }
            string protocolName = endpoint.Substring(ixEndpoint + 1, endpoint.Length - (ixEndpoint + 1));
            IProtocolFactory protocolFactory = ProtocolRegistry.Instance.GetProtocolFactory(protocolName);

            IProtocol protocol = protocolFactory.CreateProtocol(/* todo connectProperties.protocolData*/);

            string endpointStreamConnection = endpoint.Substring(0, ixEndpoint);

            IProtocolSessionPrivate protocolSession = new ProtocolSession(callback, m_executor, protocol, m_protocolSessionList, m_streamConnectionContainer, endpointStreamConnection, connectProperties, contentType);
            protocolSession.Connect();
            return protocolSession;
        }
        public IProtocolSession CreateSession(IProtocolSessionCallback callback)
        {
            IProtocolSessionPrivate protocolSession = new ProtocolSession(callback, m_executor, m_protocolSessionList, m_streamConnectionContainer);
            protocolSession.SetConnection(null, true);
            return protocolSession;
        }
        public IList<IProtocolSession> GetAllSessions()
        {
            IList<IProtocolSessionPrivate> protocolSessions = m_protocolSessionList.GetAllSessions();
            IList<IProtocolSession> result = new List<IProtocolSession>(protocolSessions);
            return result;
        }
        public IProtocolSession GetSession(long sessionId)
        {
            IProtocolSession protocolSession = m_protocolSessionList.GetSession(sessionId);
            return protocolSession;
        }
        public IExecutor? GetExecutor()
        {
            return m_executor;
        }

        private readonly IProtocolSessionList m_protocolSessionList = new ProtocolSessionList();
        private readonly IDictionary<string, ProtocolBind> m_endpoint2Bind = new Dictionary<string, ProtocolBind>();
        private readonly IStreamConnectionContainer m_streamConnectionContainer = new StreamConnectionContainer();
        private readonly IExecutor? m_executor = null;
        private bool m_disposed = false;
        private readonly object m_mutex = new object();
    }

}   // namespace finalmq
