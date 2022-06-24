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

using System.Net;
using System.Net.Sockets;
using System.Net.Security;


namespace finalmq {

    public interface IStreamConnectionContainer
    {
        int CheckReconnectInterval { get; set; }
        void Bind(string endpoint, IStreamConnectionCallback callback, BindProperties? bindProperties = null);
        void Unbind(string endpoint);
        IStreamConnection Connect(string endpoint, IStreamConnectionCallback callback, ConnectProperties? connectionProperties = null);
        IStreamConnection CreateConnection(IStreamConnectionCallback callback);
        void Connect(string endpoint, IStreamConnection streamConnection, ConnectProperties? connectionProperties = null);
        IList<IStreamConnection> GetAllConnections();
        IStreamConnection? GetConnection(long connectionId);
        IStreamConnection? TryGetConnection(long connectionId);
    }

    internal interface IStreamConnectionContainerPrivate
    {
        void Connect(string endpoint, IStreamConnection streamConnection, ConnectProperties? connectionProperties = null);
        void Disconnect(IStreamConnectionPrivate connection);
    }


    public class StreamConnectionContainer : IStreamConnectionContainer
                                           , IStreamConnectionContainerPrivate
    {
        public StreamConnectionContainer()
        {
            m_timerReconnect.Elapsed += (Object? source, System.Timers.ElapsedEventArgs e) => { DoReconnect(); };
            m_timerReconnect.AutoReset = true;
            m_timerReconnect.Enabled = true;
        }

        public void Bind(string endpoint, IStreamConnectionCallback callback, BindProperties? bindProperties = null)
        {
            if (string.IsNullOrEmpty(endpoint))
            {
                throw new ArgumentException("Endpoint is empty");
            }

            ConnectionData connectionData = AddressHelpers.Endpoint2ConnectionData(endpoint);
            connectionData.ssl = (bindProperties != null && bindProperties.SslServerOptions != null) ? true : false;

            IPAddress? address = null;
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

            TcpListener? listener = null;
            BindData? bindData = null;
            lock (m_mutex)
            {
                if (!m_endpoint2binds.ContainsKey(endpoint))
                {
                    listener = new TcpListener(address, connectionData.port);
                    bindData = new BindData(connectionData, callback, listener);
                    m_endpoint2binds.Add(endpoint, bindData);
                }
            }

            if (listener != null && bindData != null)
            {
                listener.Start();
                AsyncCallback callbackAccept = new AsyncCallback((IAsyncResult ar) => {
                    TcpClient tcpClient = listener.EndAcceptTcpClient(ar);
                    if (bindProperties != null && bindProperties.SslServerOptions != null)
                    {
                        SslServerOptions sslServerOptions = bindProperties.SslServerOptions;
                        SslStream sslStream = new SslStream(tcpClient.GetStream(), false, sslServerOptions.UserCertificateValidationCallback,
                                                            sslServerOptions.UserCertificateSelectionCallback, sslServerOptions.EncryptionPolicy);
                        StartIncomingSslConnection(bindData, sslStream, sslServerOptions);
                    }
                    else
                    {
                        Stream stream = tcpClient.GetStream();
                        StartIncomingConnection(bindData, stream);
                    }
                    AsyncCallback? c = (AsyncCallback?)ar.AsyncState;
                    listener.BeginAcceptTcpClient(c, c);
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
            connection.Connected();
            StartReading(stream, connection);
        }

        private void StartIncomingSslConnection(BindData bindData, SslStream sslStream, SslServerOptions sslServerOptions)
        {
            sslStream.BeginAuthenticateAsServer(sslServerOptions.ServerCertificate, sslServerOptions.ClientCertificateRequired, sslServerOptions.EnabledSslProtocols,
                                                sslServerOptions.CheckCertificateRevocation, (IAsyncResult ar) => {
                                                    sslStream.EndAuthenticateAsServer(ar);
                                                    ConnectionData connectionData = bindData.ConnectionData.Clone();
                                                    connectionData.incomingConnection = true;
                                                    connectionData.startTime = DateTime.Now;
                                                    connectionData.connectionState = ConnectionState.CONNECTIONSTATE_CONNECTED;
                                                    IStreamConnectionPrivate connection = AddConnection(sslStream, connectionData, bindData.Callback);
                                                    connection.Connected();
                                                    StartReading(sslStream, connection);
                                                }, null);
        }

        private IStreamConnectionPrivate AddConnection(Stream? stream, ConnectionData connectionData, IStreamConnectionCallback callback)
        {
            long connectionId = Interlocked.Increment(ref m_nextConnectionId);
            connectionData.connectionId = connectionId;
            IStreamConnectionPrivate connection = new StreamConnection(connectionData, stream, callback, this);
            lock (m_mutex)
            {
                m_connectionId2Connection[connectionId] = connection;
            }
            return connection;
        }

        private void StartReading(Stream stream, IStreamConnectionPrivate connection)
        {
            long connectionId = connection.GetConnectionId();
            byte[] buffer = new byte[4096];
            AsyncCallback callbackRead = new AsyncCallback((IAsyncResult ar) => {
                int count = stream.EndRead(ar);
                if (count > 0)
                {
                    connection.Received(buffer, count);
                    AsyncCallback? c = (AsyncCallback?)ar.AsyncState;
                    stream.BeginRead(buffer, 0, buffer.Length, c, c);
                }
                else
                {
                    ((IStreamConnectionContainerPrivate)this).Disconnect(connection);
                }
            });
            stream.BeginRead(buffer, 0, buffer.Length, callbackRead, callbackRead);
        }

        public IStreamConnection Connect(string endpoint, IStreamConnectionCallback callback, ConnectProperties? connectProperties = null)
        {
            IStreamConnection connection = CreateConnection(callback);
            Connect(endpoint, connection, connectProperties);
            return connection;
        }

        public void Connect(string endpoint, IStreamConnection streamConnection, ConnectProperties? connectProperties = null)
        {
            long connectionId = streamConnection.GetConnectionId();
            IStreamConnectionPrivate? connection = FindConnectionById(connectionId);
            if (connection == null)
            {
                throw new ArgumentException("Connection is not part of StreamConnectionContainer");
            }

            ConnectProperties connectPropertiesNoneNull = new ConnectProperties();
            if (connectProperties != null)
            {
                connectPropertiesNoneNull = connectProperties;
            }

            ConnectionData connectionData = AddressHelpers.Endpoint2ConnectionData(endpoint);
            connectionData.connectionId = connectionId;
            connectionData.incomingConnection = false;
            connectionData.reconnectInterval = connectPropertiesNoneNull.ConnectConfig.reconnectInterval;
            connectionData.totalReconnectDuration = connectPropertiesNoneNull.ConnectConfig.totalReconnectDuration;
            connectionData.startTime = DateTime.Now;
            connectionData.ssl = connectPropertiesNoneNull.SslClientOptions != null ? true : false;
            connectionData.connectionState = ConnectionState.CONNECTIONSTATE_CONNECTING;
            connectionData.connectProperties = connectProperties;

            TcpClient tcpClient = new TcpClient();
            SslStream? sslStream = null;
            if (connectPropertiesNoneNull.SslClientOptions != null)
            {
                SslClientOptions sslClientOptions = connectPropertiesNoneNull.SslClientOptions;
                sslStream = new SslStream(tcpClient.GetStream(), false, sslClientOptions.UserCertificateValidationCallback,
                                                sslClientOptions.UserCertificateSelectionCallback, sslClientOptions.EncryptionPolicy);
                connectionData.stream = sslStream;
            }
            else
            {
                try
                {
                    connectionData.stream = tcpClient.GetStream();
                }
                catch (InvalidOperationException)
                {
                    ((IStreamConnectionContainerPrivate)this).Disconnect(connection);
                }
            }

            if (connectionData.stream != null)
            {
                lock (m_mutex)
                {
                    m_connectionId2Conns[connectionData.connectionId] = new ConnData(tcpClient);
                }

                connection.UpdateConnectionData(connectionData);

                tcpClient.BeginConnect(connectionData.hostname, connectionData.port, (IAsyncResult ar) =>
                {
                    tcpClient.EndConnect(ar);
                    if (connectPropertiesNoneNull.SslClientOptions != null && sslStream != null)
                    {
                        StartOutgoingSslConnection(connection, connectionData, sslStream, connectPropertiesNoneNull.SslClientOptions);
                    }
                    else
                    {
                        connection.Connected();
                        StartReading(connectionData.stream, connection);
                    }
                }, null);
            }
        }

        private void StartOutgoingSslConnection(IStreamConnectionPrivate streamConnection, ConnectionData connectionData, SslStream sslStream, SslClientOptions sslClientOptions)
        {
            sslStream.BeginAuthenticateAsClient(sslClientOptions.TargetHost, sslClientOptions.ClientCertificates, sslClientOptions.EnabledSslProtocols,
                                                sslClientOptions.CheckCertificateRevocation, 
                (IAsyncResult ar) => 
                {
                    sslStream.EndAuthenticateAsClient(ar);
                    streamConnection.Connected();
                    StartReading(sslStream, streamConnection);
                }, 
                null);
        }


        public IStreamConnectionPrivate? FindConnectionById(long connectionId)
        {
            IStreamConnectionPrivate? connection = null;
            lock (m_mutex)
            {
                m_connectionId2Connection.TryGetValue(connectionId, out connection);
            }
            return connection;
        }

        public IStreamConnection CreateConnection(IStreamConnectionCallback callback)
        {
            ConnectionData connectionData = new ConnectionData();
            connectionData.incomingConnection = false;
            connectionData.connectionState = ConnectionState.CONNECTIONSTATE_CREATED;
            IStreamConnectionPrivate connection = AddConnection(null, connectionData, callback);
            return connection;
        }

        public IList<IStreamConnection> GetAllConnections()
        {
            IList<IStreamConnection> connections = new List<IStreamConnection>();
            lock (m_mutex)
            {
                foreach (var entry in m_connectionId2Connection)
                {
                    connections.Add(entry.Value);
                }
            }
            return connections;
        }

        public IStreamConnection GetConnection(long connectionId)
        {
            lock (m_mutex)
            {
                IStreamConnectionPrivate? connection = null;
                m_connectionId2Connection.TryGetValue(connectionId, out connection);
                if (connection != null)
                {
                    return connection;
                }
            }
            throw new System.Collections.Generic.KeyNotFoundException("Connection ID " + connectionId.ToString() + " not found");
        }

        public IStreamConnection? TryGetConnection(long connectionId)
        {
            IStreamConnectionPrivate? connection = null;
            lock (m_mutex)
            {
                m_connectionId2Connection.TryGetValue(connectionId, out connection);
            }
            return connection;
        }

        public int CheckReconnectInterval 
        { 
            get => (int)m_timerReconnect.Interval;
            set
            {
                m_timerReconnect.Interval = 1000.0;
            }
        }

        public void Unbind(string endpoint)
        {
            lock (m_mutex)
            {
                BindData? bindData = null;
                m_endpoint2binds.TryGetValue(endpoint, out bindData);
                if (bindData != null)
                {
                    bindData.TcpListener.Stop();
                    lock (m_mutex)
                    {
                        m_endpoint2binds.Remove(endpoint);
                    }
                }
            }
        }

        private void DoReconnect()
        {
            IList<IStreamConnectionPrivate> connections = new List<IStreamConnectionPrivate>();
            lock (m_mutex)
            {
                foreach (var entry in m_connectionId2Connection)
                {
                    connections.Add(entry.Value);
                }
            }

            foreach (var connection in connections)
            {
//todo                connection.DoReconnect();
            }
        }

        void IStreamConnectionContainerPrivate.Disconnect(IStreamConnectionPrivate connection)
        {
            bool removeConn = connection.ChangeStateForDisconnect();
            long connectionId = connection.GetConnectionId();
            RemoveConnection(connectionId, removeConn);
            if (removeConn)
            {
                connection.Disconnected();
            }
        }

        private void RemoveConnection(long connectionId, bool removeConn)
        {
            IStreamConnectionPrivate? streamConnection = null;
            ConnectionData? connectionData = null;
            ConnData? connData = null;
            lock (m_mutex)
            {
                m_connectionId2Connection.TryGetValue(connectionId, out streamConnection);
                if (removeConn)
                {
                    m_connectionId2Connection.Remove(connectionId);
                }
                if (streamConnection != null)
                {
                    connectionData = streamConnection.GetConnectionData();
                    if (!connectionData.incomingConnection)
                    {
                        if (m_connectionId2Conns.TryGetValue(connectionId, out connData))
                        {
                            m_connectionId2Conns.Remove(connectionId);
                        }
                    }
                }
            }
            if (connectionData != null)
            {
                connectionData.stream?.Close();
            }
            if (connData != null)
            {
                connData.TcpClient.Close();
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

        class ConnData
        {
            public ConnData(TcpClient tcpClient)
            {
                this.tcpClient = tcpClient;
            }

            public TcpClient TcpClient
            {
                get => tcpClient;
            }
            private readonly TcpClient tcpClient;
        }

        private IDictionary<string, BindData> m_endpoint2binds = new Dictionary<string, BindData>();
        private IDictionary<long, ConnData> m_connectionId2Conns = new Dictionary<long, ConnData>();
        private IDictionary<long, IStreamConnectionPrivate> m_connectionId2Connection = new Dictionary<long, IStreamConnectionPrivate>();
        private static long m_nextConnectionId = 0;
        private System.Timers.Timer m_timerReconnect = new System.Timers.Timer(1000.0);
        private object m_mutex = new object();
    }


}   // namespace finalmq
