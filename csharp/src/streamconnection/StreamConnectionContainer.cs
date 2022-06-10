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

using System.Collections.Generic;
using System;
using System.Net;
using System.Net.Sockets;
using System.Net.Security;
using System.IO;
using System.Threading;


namespace finalmq {

    public delegate void FuncPollerLoopTimer();

    public interface IStreamConnectionContainer
    {
        void Init(int cycleTime = 100, FuncPollerLoopTimer funcTimer = null, int checkReconnectInterval = 1000);
        void Bind(string endpoint, IStreamConnectionCallback callback, BindProperties bindProperties = null);
        void Unbind(string endpoint);
        IStreamConnection Connect(string endpoint, IStreamConnectionCallback callback, ConnectProperties connectionProperties = null);
        IStreamConnection CreateConnection(IStreamConnectionCallback callback);
        bool Connect(string endpoint, IStreamConnection streamConnection, ConnectProperties connectionProperties = null);
        IList<IStreamConnection> GetAllConnections();
        IStreamConnection GetConnection(long connectionId);
        //void Run();
        //void terminatePollerLoop();
        IExecutor GetPollerThreadExecutor();
    };


    public class StreamConnectionContainer : IStreamConnectionContainer
    {
        public void Bind(string endpoint, IStreamConnectionCallback callback, BindProperties bindProperties = null)
        {
            if (string.IsNullOrEmpty(endpoint))
            {
                throw new ArgumentException("Endpoint is empty");
            }

            ConnectionData connectionData = AddressHelpers.Endpoint2ConnectionData(endpoint);
            connectionData.ssl = (bindProperties != null) ? bindProperties.CertificateData.ssl : false;

            IPAddress address = null;
            if (connectionData.hostname == "*")
            {
                address = IPAddress.IPv6Any;
            }
            else
            {
                IPHostEntry entry = Dns.GetHostEntry(endpoint);
                if (entry.AddressList.Length > 0)
                {
                    address = entry.AddressList[0];
                }
            }

            if (address == null)
            {
                throw new ArgumentException("Hostname error: " + endpoint);
            }

            TcpListener listener = null;
            BindData bindData = null;
            lock (m_mutex)
            {
                if (!m_endpoint2binds.ContainsKey(endpoint))
                {
                    listener = new TcpListener(address, connectionData.port);
                    bindData = new BindData(connectionData, callback, listener);
                    m_endpoint2binds.Add(endpoint, bindData);
                }
            }

            if (listener != null)
            {
                listener.Start();
                AsyncCallback callbackAccept = new AsyncCallback((IAsyncResult ar) => {
                    AsyncCallback c = (AsyncCallback)ar.AsyncState;
                    TcpClient tcpClient = listener.EndAcceptTcpClient(ar);
                    if (connectionData.ssl)
                    {
                        CertificateData certificateData = bindProperties.CertificateData;
                        SslStream sslStream = new SslStream(tcpClient.GetStream(), false, certificateData.userCertificateValidationCallback, 
                                                            certificateData.userCertificateSelectionCallback,certificateData.encryptionPolicy);
                        listener.BeginAcceptTcpClient(c, null);
                        StartIncomingSslConnection(bindData, sslStream, certificateData);
                    }
                    else
                    {
                        Stream stream = tcpClient.GetStream();
                        listener.BeginAcceptTcpClient(c, null);
                        StartIncomingConnection(bindData, stream);
                    }
                });
                listener.BeginAcceptTcpClient(callbackAccept, callbackAccept);
            }
        }

        private void StartIncomingConnection(BindData bindData, Stream stream)
        {
            ConnectionData connectionData = bindData.ConnectionData.Clone();
            connectionData.incomingConnection = true;
            connectionData.startTime = DateTime.Now;
            connectionData.connectionState = ConnectionState.CONNECTIONSTATE_CONNECTED;
            IStreamConnectionPrivate connection = AddConnection(stream, connectionData, bindData.Callback);
            connection.Connected(connection);
        }

        private void StartIncomingSslConnection(BindData bindData, SslStream sslStream, CertificateData certificateData)
        {
            sslStream.BeginAuthenticateAsServer(certificateData.serverCertificate, certificateData.clientCertificateRequired, certificateData.enabledSslProtocols,
                                                certificateData.checkCertificateRevocation, (IAsyncResult ar) => {
                ConnectionData connectionData = bindData.ConnectionData.Clone();
                connectionData.incomingConnection = true;
                connectionData.startTime = DateTime.Now;
                connectionData.connectionState = ConnectionState.CONNECTIONSTATE_CONNECTED;
                IStreamConnectionPrivate connection = AddConnection(sslStream, connectionData, bindData.Callback);
                connection.Connected(connection);
            }, null);
        }

        private IStreamConnectionPrivate AddConnection(Stream stream, ConnectionData connectionData, IStreamConnectionCallback callback)
        {
            long connectionId = Interlocked.Increment(ref m_nextConnectionId);
            connectionData.connectionId = connectionId;
            IStreamConnectionPrivate connection = new StreamConnection(connectionData, stream, callback);
            lock (m_mutex)
            {
                m_connectionId2Connection[connectionId] = connection;
            }
            return connection;
        }


        public IStreamConnection Connect(string endpoint, IStreamConnectionCallback callback, ConnectProperties connectionProperties = null)
        {
            throw new System.NotImplementedException();
        }

        public bool Connect(string endpoint, IStreamConnection streamConnection, ConnectProperties connectionProperties = null)
        {
            throw new System.NotImplementedException();
        }

        public IStreamConnection CreateConnection(IStreamConnectionCallback callback)
        {
            throw new System.NotImplementedException();
        }

        public IList<IStreamConnection> GetAllConnections()
        {
            throw new System.NotImplementedException();
        }

        public IStreamConnection GetConnection(long connectionId)
        {
            throw new System.NotImplementedException();
        }

        public IExecutor GetPollerThreadExecutor()
        {
            throw new System.NotImplementedException();
        }

        public void Init(int cycleTime = 100, FuncPollerLoopTimer funcTimer = null, int checkReconnectInterval = 1000)
        {
            throw new System.NotImplementedException();
        }

        public void Unbind(string endpoint)
        {
            lock (m_mutex)
            {
                BindData bindData = null;
                bool found = m_endpoint2binds.TryGetValue(endpoint, out bindData);
                if (found)
                {
                    bindData.TcpListener.Stop();
                    lock (m_mutex)
                    {
                        m_endpoint2binds.Remove(endpoint);
                    }
                }
            }
        }

        class BindData
        {
            public BindData(ConnectionData connectionData, IStreamConnectionCallback callback, TcpListener tcpListener)
            {
                this.connectionData = connectionData;
                this.callback = callback;
                this.tcpListener = tcpListener;
            }
            public ConnectionData ConnectionData { get => connectionData; }
            public IStreamConnectionCallback Callback { get => callback; }
            public TcpListener TcpListener { get => tcpListener; }

            private ConnectionData connectionData;
            private IStreamConnectionCallback callback;
            private TcpListener tcpListener;
        }

        private IDictionary<string, BindData> m_endpoint2binds = new Dictionary<string, BindData>();
        private IDictionary<long, IStreamConnectionPrivate> m_connectionId2Connection = new Dictionary<long, IStreamConnectionPrivate>();
        private static long m_nextConnectionId = 0;
        private object m_mutex = new object();

    }


    //class SYMBOLEXP StreamConnectionContainer : public IStreamConnectionContainer
    //{
    //public:
    //    StreamConnectionContainer();
    //    ~StreamConnectionContainer();

    //private:
    //    // IStreamConnectionContainer
    //    void init(int cycleTime = 100, FuncPollerLoopTimer funcTimer = {}, int checkReconnectInterval = 1000) override;
    //    int bind(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const BindProperties& bindProperties = {}) override;
    //    void unbind(const std::string& endpoint) override;
    //    IStreamConnectionPtr connect(const std::string& endpoint, hybrid_ptr<IStreamConnectionCallback> callback, const ConnectProperties& connectionProperties = {}) override;
    //    IStreamConnectionPtr createConnection(hybrid_ptr<IStreamConnectionCallback> callback) override;
    //    bool connect(const std::string& endpoint, const IStreamConnectionPtr& streamConnection, const ConnectProperties& connectionProperties = {}) override;
    //    std::vector< IStreamConnectionPtr > getAllConnections() const override;
    //    IStreamConnectionPtr getConnection(std::int64_t connectionId) const override;
    //    void run() override;
    //    void terminatePollerLoop() override;
    //    IExecutorPtr getPollerThreadExecutor() const override;

    //    void pollerLoop();

    //    struct BindData
    //    {
    //        ConnectionData                          connectionData;
    //        SocketPtr                               socket;
    //        hybrid_ptr<IStreamConnectionCallback>   callback;
    //    };

    //    std::unordered_map<SOCKET, BindData>::iterator findBindByEndpoint(const std::string& endpoint);
    //    IStreamConnectionPrivatePtr findConnectionBySdOnlyForPollerLoop(SOCKET sd);
    //    IStreamConnectionPrivatePtr findConnectionById(std::int64_t connectionId);
    //    bool createSocket(const IStreamConnectionPtr& streamConnection, ConnectionData& connectionData, const ConnectProperties& connectionProperties);
    //    void removeConnection(const SocketDescriptorPtr& sd, std::int64_t connectionId);
    //    void disconnectIntern(const IStreamConnectionPrivatePtr& connectionDisconnect, const SocketDescriptorPtr& sd);
    //    IStreamConnectionPrivatePtr addConnection(const SocketPtr& socket, ConnectionData& connectionData, hybrid_ptr<IStreamConnectionCallback> callback);
    //    void handleConnectionEvents(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, const DescriptorInfo& info);
    //    void handleBindEvents(const DescriptorInfo& info);
    //    void handleReceive(const IStreamConnectionPrivatePtr& connection, const SocketPtr& socket, int bytesToRead);
    //    static bool isTimerExpired(std::chrono::time_point<std::chrono::steady_clock>& lastTime, int interval);
    //    void doReconnect();

    //    std::shared_ptr<IPoller>                                        m_poller;
    //    std::unordered_map<SOCKET, BindData>                            m_sd2binds;
    //    std::unordered_map<std::int64_t, IStreamConnectionPrivatePtr>   m_connectionId2Connection;
    //    std::unordered_map<SOCKET, IStreamConnectionPrivatePtr>         m_sd2Connection;
    //    std::unordered_map<SOCKET, IStreamConnectionPrivatePtr>         m_sd2ConnectionPollerLoop;
    //    std::atomic_flag                                                m_connectionsStable{};
    //    static std::atomic_int64_t                                      m_nextConnectionId;
    //    bool                                                            m_terminatePollerLoop = false;
    //    int                                                             m_cycleTime = 100;
    //    int                                                             m_checkReconnectInterval = 1000;
    //    FuncPollerLoopTimer                                             m_funcTimer;
    //    IExecutorPtr                                                    m_executorPollerThread;
    //    std::unique_ptr<IExecutorWorker>                                m_executorWorker;
    //    std::thread                                                     m_threadTimer;
    //    mutable std::mutex                                              m_mutex;

    //    std::chrono::time_point<std::chrono::steady_clock>              m_lastReconnectTime;
    //    std::chrono::time_point<std::chrono::steady_clock>              m_lastCycleTime;

    //#ifdef USE_OPENSSL
    //    struct SslAcceptingData
    //    {
    //        SocketPtr socket;
    //        ConnectionData connectionData;
    //        hybrid_ptr<IStreamConnectionCallback> callback;
    //    };
    //    bool sslAccepting(SslAcceptingData& sslAcceptingData);
    //    std::unordered_map<SOCKET, SslAcceptingData>                    m_sslAcceptings;
    //#endif
    //};

}   // namespace finalmq
