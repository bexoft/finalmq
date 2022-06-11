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
using System.Security.Cryptography.X509Certificates;


namespace finalmq {

    public delegate void FuncPollerLoopTimer();

    public interface IStreamConnectionContainer
    {
        void Init(int cycleTime = 100, FuncPollerLoopTimer? funcTimer = null, int checkReconnectInterval = 1000);
        void Bind(string endpoint, IStreamConnectionCallback? callback, BindProperties? bindProperties = null);
        void Unbind(string endpoint);
        IStreamConnection Connect(string endpoint, IStreamConnectionCallback callback, ConnectProperties? connectionProperties = null);
        IStreamConnection CreateConnection(IStreamConnectionCallback callback);
        void Connect(string endpoint, IStreamConnection streamConnection, ConnectProperties? connectionProperties = null);
        IList<IStreamConnection> GetAllConnections();
        IStreamConnection GetConnection(long connectionId);
        //void Run();
        //void terminatePollerLoop();
        IExecutor GetPollerThreadExecutor();
    };


    public class StreamConnectionContainer : IStreamConnectionContainer
    {
        public void Bind(string endpoint, IStreamConnectionCallback? callback, BindProperties? bindProperties = null)
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
                    AsyncCallback? c = (AsyncCallback?)ar.AsyncState;
                    TcpClient tcpClient = listener.EndAcceptTcpClient(ar);
                    if (bindProperties != null && bindProperties.SslServerOptions != null)
                    {
                        SslServerOptions sslServerOptions = bindProperties.SslServerOptions;
                        SslStream sslStream = new SslStream(tcpClient.GetStream(), false, sslServerOptions.UserCertificateValidationCallback,
                                                            sslServerOptions.UserCertificateSelectionCallback, sslServerOptions.EncryptionPolicy);
                        listener.BeginAcceptTcpClient(c, null);
                        StartIncomingSslConnection(bindData, sslStream, sslServerOptions);
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
            connection.Connected();
        }

        private void StartIncomingSslConnection(BindData bindData, SslStream sslStream, SslServerOptions sslServerOptions)
        {
            sslStream.BeginAuthenticateAsServer(sslServerOptions.ServerCertificate, sslServerOptions.ClientCertificateRequired, sslServerOptions.EnabledSslProtocols,
                                                sslServerOptions.CheckCertificateRevocation, (IAsyncResult ar) => {
                                                    ConnectionData connectionData = bindData.ConnectionData.Clone();
                                                    connectionData.incomingConnection = true;
                                                    connectionData.startTime = DateTime.Now;
                                                    connectionData.connectionState = ConnectionState.CONNECTIONSTATE_CONNECTED;
                                                    IStreamConnectionPrivate connection = AddConnection(sslStream, connectionData, bindData.Callback);
                                                    connection.Connected();
                                                }, null);
        }

        private IStreamConnectionPrivate AddConnection(Stream? stream, ConnectionData connectionData, IStreamConnectionCallback? callback)
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


        public IStreamConnection Connect(string endpoint, IStreamConnectionCallback callback, ConnectProperties? connectionProperties = null)
        {
            IStreamConnection connection = CreateConnection(callback);
            Connect(endpoint, connection, connectionProperties);
            return connection;
        }

        public void Connect(string endpoint, IStreamConnection streamConnection, ConnectProperties? connectionProperties = null)
        {
            IStreamConnectionPrivate? connection = FindConnectionById(streamConnection.GetConnectionId());
            if (connection == null)
            {
                throw new ArgumentException("Connection is not part of StreamConnectionContainer");
            }

            ConnectProperties connectionPropertiesNoneNull = new ConnectProperties();
            if (connectionProperties != null)
            {
                connectionPropertiesNoneNull = connectionProperties;
            }

            ConnectionData connectionData = AddressHelpers.Endpoint2ConnectionData(endpoint);
            connectionData.connectionId = connection.GetConnectionId();
            connectionData.incomingConnection = false;
            connectionData.reconnectInterval = connectionPropertiesNoneNull.ConnectConfig.reconnectInterval;
            connectionData.totalReconnectDuration = connectionPropertiesNoneNull.ConnectConfig.totalReconnectDuration;
            connectionData.startTime = DateTime.Now;
            connectionData.ssl = connectionPropertiesNoneNull.SslClientOptions != null ? true : false;
            connectionData.connectionState = ConnectionState.CONNECTIONSTATE_CONNECTING;

            TcpClient tcpClient = new TcpClient();
            SslStream? sslStream = null;
            if (connectionPropertiesNoneNull.SslClientOptions != null)
            {
                SslClientOptions sslClientOptions = connectionPropertiesNoneNull.SslClientOptions;
                sslStream = new SslStream(tcpClient.GetStream(), false, sslClientOptions.UserCertificateValidationCallback,
                                                sslClientOptions.UserCertificateSelectionCallback, sslClientOptions.EncryptionPolicy);
                connectionData.stream = sslStream;
            }
            else
            {
                connectionData.stream = tcpClient.GetStream();
            }

            lock (m_mutex)
            {
                m_connectionId2Conns[connectionData.connectionId] = new ConnData(tcpClient);
            }

            connection.UpdateConnectionData(connectionData);

            tcpClient.BeginConnect(connectionData.hostname, connectionData.port, (IAsyncResult ar) => {
                if (connectionPropertiesNoneNull.SslClientOptions != null && sslStream != null)
                {
                    StartOutgoingSslConnection(connection, connectionData, sslStream, connectionPropertiesNoneNull.SslClientOptions);
                }
                else 
                {
                    connection.Connected();
                }
            }, null);

        }

        private void StartOutgoingSslConnection(IStreamConnectionPrivate streamConnection, ConnectionData connectionData, SslStream sslStream, SslClientOptions sslClientOptions)
        {
            sslStream.BeginAuthenticateAsClient(sslClientOptions.TargetHost, sslClientOptions.ClientCertificates, sslClientOptions.EnabledSslProtocols,
                                                sslClientOptions.CheckCertificateRevocation, 
                (IAsyncResult ar) => 
                {
                    streamConnection.Connected();
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

        public void Init(int cycleTime = 100, FuncPollerLoopTimer? funcTimer = null, int checkReconnectInterval = 1000)
        {
            throw new System.NotImplementedException();
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

        class BindData
        {
            public BindData(ConnectionData connectionData, IStreamConnectionCallback? callback, TcpListener tcpListener)
            {
                this.connectionData = connectionData;
                this.callback = callback;
                this.tcpListener = tcpListener;
            }
            public ConnectionData ConnectionData { get => connectionData; }
            public IStreamConnectionCallback? Callback { get => callback; }
            public TcpListener TcpListener { get => tcpListener; }

            private ConnectionData connectionData;
            private IStreamConnectionCallback? callback;
            private TcpListener tcpListener;
        }

        class ConnData
        {
            public ConnData(TcpClient tcpClient)
            {
                this.tcpClient = tcpClient;
            }
            private TcpClient tcpClient;
        }

        private IDictionary<string, BindData> m_endpoint2binds = new Dictionary<string, BindData>();
        private IDictionary<long, ConnData> m_connectionId2Conns = new Dictionary<long, ConnData>();
        private IDictionary<long, IStreamConnectionPrivate> m_connectionId2Connection = new Dictionary<long, IStreamConnectionPrivate>();
        private static long m_nextConnectionId = 0;
        private object m_mutex = new object();

    }


}   // namespace finalmq
