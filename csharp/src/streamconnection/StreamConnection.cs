// MIT License

// Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


using System.Security.Cryptography.X509Certificates;
using System.Security.Authentication;
using System.Net.Security;

namespace finalmq {

    public class SslServerOptions
    {
        public SslServerOptions(X509Certificate serverCertificate,
                                bool clientCertificateRequired = false,
                                SslProtocols enabledSslProtocols = SslProtocols.None,
                                bool checkCertificateRevocation = false,
                                RemoteCertificateValidationCallback? userCertificateValidationCallback = null,
                                LocalCertificateSelectionCallback? userCertificateSelectionCallback = null,
                                EncryptionPolicy encryptionPolicy = EncryptionPolicy.RequireEncryption)
        {
            this.serverCertificate = serverCertificate;
            this.clientCertificateRequired = clientCertificateRequired;
            this.enabledSslProtocols = enabledSslProtocols;
            this.checkCertificateRevocation = checkCertificateRevocation;
            this.userCertificateValidationCallback = userCertificateValidationCallback;
            this.userCertificateSelectionCallback = userCertificateSelectionCallback;
            this.encryptionPolicy = encryptionPolicy;
        }

        public X509Certificate ServerCertificate { get => serverCertificate; set => serverCertificate = value; }
        public bool ClientCertificateRequired { get => clientCertificateRequired; set => clientCertificateRequired = value; }
        public SslProtocols EnabledSslProtocols { get => enabledSslProtocols; set => enabledSslProtocols = value; }
        public bool CheckCertificateRevocation { get => checkCertificateRevocation; set => checkCertificateRevocation = value; }
        public RemoteCertificateValidationCallback? UserCertificateValidationCallback { get => userCertificateValidationCallback; set => userCertificateValidationCallback = value; }
        public LocalCertificateSelectionCallback? UserCertificateSelectionCallback { get => userCertificateSelectionCallback; set => userCertificateSelectionCallback = value; }
        public EncryptionPolicy EncryptionPolicy { get => encryptionPolicy; set => encryptionPolicy = value; }

        // BeginAuthenticateAsServer parameter
        private X509Certificate serverCertificate;
        private bool clientCertificateRequired;
        private SslProtocols enabledSslProtocols;
        private bool checkCertificateRevocation;

        // SslStream ctor parameter
        private RemoteCertificateValidationCallback? userCertificateValidationCallback;
        private LocalCertificateSelectionCallback? userCertificateSelectionCallback;
        private EncryptionPolicy encryptionPolicy;
    }

    public class BindProperties
    {
        public BindProperties(SslServerOptions? sslServerOptions = null)
        {
            this.sslServerOptions = sslServerOptions;
        }
        public SslServerOptions? SslServerOptions { get => sslServerOptions; set => sslServerOptions = value; }

        private SslServerOptions? sslServerOptions = null;
    }

    public class ConnectConfig
    {
        public ConnectConfig(int reconnectInterval = 5000, int totalReconnectDuration = -1)
        {
            this.reconnectInterval = reconnectInterval;
            this.totalReconnectDuration = totalReconnectDuration;
        }
        public int ReconnectInterval { get => reconnectInterval; set => reconnectInterval = value; }
        public int TotalReconnectDuration { get => totalReconnectDuration; set => totalReconnectDuration = value; }

        private int reconnectInterval = 5000;       ///< if the server is not available, you can pass a reconnection intervall in [ms]
        private int totalReconnectDuration = -1;    ///< if the server is not available, you can pass a duration in [ms] how long the reconnect shall happen. -1 means: try for ever.
    }


    public class SslClientOptions
    {
        public SslClientOptions(string targetHost,
                                RemoteCertificateValidationCallback? userCertificateValidationCallback = null,
                                X509CertificateCollection? clientCertificates = null,
                                SslProtocols enabledSslProtocols = SslProtocols.None,
                                bool checkCertificateRevocation = false,
                                LocalCertificateSelectionCallback? userCertificateSelectionCallback = null,
                                EncryptionPolicy encryptionPolicy = EncryptionPolicy.RequireEncryption)
        {
            this.targetHost = targetHost;
            this.userCertificateValidationCallback = userCertificateValidationCallback;
            this.clientCertificates = clientCertificates;
            this.enabledSslProtocols = enabledSslProtocols;
            this.checkCertificateRevocation = checkCertificateRevocation;
            this.userCertificateSelectionCallback = userCertificateSelectionCallback;
            this.encryptionPolicy = encryptionPolicy;
        }

        public string TargetHost { get => targetHost; set => targetHost = value; }
        public RemoteCertificateValidationCallback? UserCertificateValidationCallback { get => userCertificateValidationCallback; set => userCertificateValidationCallback = value; }
        public X509CertificateCollection? ClientCertificates { get => clientCertificates; set => clientCertificates = value; }
        public SslProtocols EnabledSslProtocols { get => enabledSslProtocols; set => enabledSslProtocols = value; }
        public bool CheckCertificateRevocation { get => checkCertificateRevocation; set => checkCertificateRevocation = value; }
        public LocalCertificateSelectionCallback? UserCertificateSelectionCallback { get => userCertificateSelectionCallback; set => userCertificateSelectionCallback = value; }
        public EncryptionPolicy EncryptionPolicy { get => encryptionPolicy; set => encryptionPolicy = value; }

        // BeginAuthenticateAsClient parameter
        private string targetHost;
        private X509CertificateCollection? clientCertificates;
        private SslProtocols enabledSslProtocols;
        private bool checkCertificateRevocation;

        // SslStream ctor parameter
        private RemoteCertificateValidationCallback? userCertificateValidationCallback;
        private LocalCertificateSelectionCallback? userCertificateSelectionCallback;
        private EncryptionPolicy encryptionPolicy;
    }

    public class ConnectProperties
    {
        public ConnectProperties(SslClientOptions? sslClientOptions = null, ConnectConfig? config = null)
        {
            this.sslClientOptions = sslClientOptions;
            if (config != null)
            {
                this.config = config;
            }
        }
        public SslClientOptions? SslClientOptions { get => sslClientOptions; set => sslClientOptions = value; }
        public ConnectConfig ConnectConfig { get => config; set => config = value; }

        private SslClientOptions? sslClientOptions = null;
        private ConnectConfig config = new ConnectConfig();
    //    Variant protocolData;
    }



    public interface IStreamConnectionCallback
    {
        IStreamConnectionCallback? Connected(IStreamConnection connection);
        void Disconnected(IStreamConnection connection);
        void Received(IStreamConnection connection, byte[] buffer, int count);
    }




    public interface IStreamConnection
    {
        void SendMessage(IMessage msg);
        ConnectionData ConnectionData { get; }
        ConnectionState ConnectionState { get; }
        long ConnectionId { get; }
        void Disconnect();
    };

    public interface IStreamConnectionPrivate : IStreamConnection, IDisposable
    {
        bool Connect();
//        Socket GetSocketPrivate();
        bool DoReconnect();
        bool ChangeStateForDisconnect();
        bool GetDisconnectFlag();
        void UpdateConnectionData(ConnectionData connectionData);
        void Connected();
        void Disconnected();
        void Received(byte[] buffer, int count);
    };

    internal class StreamConnection : IStreamConnectionPrivate
    {
        bool m_disposed = false;

        public StreamConnection(ConnectionData connectionData, Stream? stream, IStreamConnectionCallback callback, IStreamConnectionContainerPrivate streamConnectionContainer)
        {
            m_connectionData = connectionData;
            m_callback = callback;
            m_streamConnectionContainer = streamConnectionContainer;
        }

        ~StreamConnection()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            lock (m_mutex)
            {
                Dispose(true);
            }
            GC.SuppressFinalize(this);
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
                m_connectionData.Stream?.DisposeAsync();
            }
        }

        public bool ChangeStateForDisconnect()
        {
            bool removeConnection = false;
            lock (m_mutex)
            {
                if (!m_disposed)
                {
                    bool reconnectExpired = false;
                    if (!GetDisconnectFlag() && (m_connectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTING))
                    {
                        TimeSpan dur = DateTime.Now - m_connectionData.StartTime;
                        int delta = dur.Milliseconds;
                        if (m_connectionData.TotalReconnectDuration >= 0 && (delta < 0 || delta >= m_connectionData.TotalReconnectDuration))
                        {
                            reconnectExpired = true;
                        }
                        else
                        {
                            m_connectionData.ConnectionState = ConnectionState.CONNECTIONSTATE_CONNECTING_FAILED;
                        }
                    }

                    if (GetDisconnectFlag() || (m_connectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED) || reconnectExpired)
                    {
                        removeConnection = true;
                        m_connectionData.ConnectionState = ConnectionState.CONNECTIONSTATE_DISCONNECTED;
                    }
                }
                else
                {
                    removeConnection = true;
                }
            }
            return removeConnection;
        }

        public bool Connect()
        {
            bool connecting = false;
            ConnectionData connectionData;
            lock (m_mutex)
            {
                if (m_disposed)
                {
                    throw new ObjectDisposedException(GetType().FullName);
                }
                connectionData = m_connectionData;
            }
            if (connectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CREATED ||
                connectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTING_FAILED)
            {
                m_streamConnectionContainer.Connect(connectionData.Endpoint, this, connectionData.ConnectProperties);
                connecting = true;
            }
            return connecting;
        }

        public void Connected()
        {
            IStreamConnectionCallback callback;
            lock (m_mutexCallback)
            {
                callback = m_callback;
            }
            if (callback != null)
            {
                IStreamConnectionCallback? callbackOverride = callback.Connected(this);
                if (callbackOverride != null)
                {
                    lock (m_mutexCallback)
                    {
                        m_callback = callbackOverride;
                    }
                    callbackOverride.Connected(this);
                }
            }

        }

        public void Disconnect()
        {
            Interlocked.Exchange(ref m_disconnectFlag, 1);
            m_streamConnectionContainer.Disconnect(this);
        }

        public void Disconnected()
        {
            IStreamConnectionCallback callback;
            lock (m_mutexCallback)
            {
                callback = m_callback;
            }
            if (callback != null)
            {
                callback.Disconnected(this);
            }
        }

        public bool DoReconnect()
        {
            bool reconnecting = false;
            ConnectionData connectionData;
            lock (m_mutex)
            {
                connectionData = m_connectionData;
            }
            if (!connectionData.IncomingConnection &&
                connectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTING_FAILED &&
                connectionData.ReconnectInterval >= 0)
            {
                DateTime now = DateTime.Now;
                TimeSpan dur = now - m_lastReconnectTime;
                int delta = dur.Milliseconds;
                if (delta < 0 || delta >= m_connectionData.ReconnectInterval)
                {
                    m_lastReconnectTime = now;
                    reconnecting = Connect();
                }
            }
            return reconnecting;
        }

        public ConnectionData ConnectionData
        {
            get
            {
                lock (m_mutex)
                {
                    return m_connectionData.Clone();
                }
            }
        }

        public long ConnectionId
        {
            get
            {
                lock (m_mutex)
                {
                    return m_connectionData.ConnectionId;
                }
            }
        }

        public ConnectionState ConnectionState
        {
            get
            {
                lock (m_mutex)
                {
                    return m_connectionData.ConnectionState;
                }
            }
        }

        public bool GetDisconnectFlag()
        {
            return (Interlocked.Read(ref m_disconnectFlag) != 0);
        }

        public void Received(byte[] buffer, int count)
        {
            m_callback.Received(this, buffer, count);
        }

        public void SendMessage(IMessage msg)
        {
            IList<BufferRef> buffers = msg.GetAllSendBuffers();
            lock (m_mutex)
            {
                if (m_disposed)
                {
                    throw new ObjectDisposedException(GetType().FullName);
                }
                var connectionState = m_connectionData.ConnectionState;
                if (connectionState == ConnectionState.CONNECTIONSTATE_CREATED ||
                    connectionState == ConnectionState.CONNECTIONSTATE_CONNECTING ||
                    connectionState == ConnectionState.CONNECTIONSTATE_CONNECTED)
                {
                    Stream? stream = m_connectionData.Stream;
                    if (stream != null)
                    {
                        SendBuffers(stream, buffers);
                    }
                    else
                    {
                        foreach (BufferRef buffer in buffers)
                        {
                            m_pendingBuffers.Add(buffer);
                        }
                    }
                }
            }
        }

        void IStreamConnectionPrivate.UpdateConnectionData(ConnectionData connectionData)
        {
            lock (m_mutex)
            {
                if (!m_disposed)
                {
                    m_connectionData = connectionData;
                    Stream? stream = m_connectionData.Stream;
                    if (stream != null)
                    {
                        SendBuffers(stream, m_pendingBuffers);
                        m_pendingBuffers.Clear();
                    }
                }
            }
        }

        void SendBuffers(Stream stream, IList<BufferRef> buffers)
        {
            foreach (BufferRef buffer in buffers)
            {
                stream.BeginWrite(buffer.Buffer, buffer.Offset, buffer.Length,
                    (IAsyncResult ar) =>
                    {
                        try
                        {
                            stream.EndWrite(ar);
                        }
                        catch (Exception)
                        {
                            m_streamConnectionContainer.Disconnect(this);
                        }
                    }, null);
            }
        }

        ConnectionData m_connectionData;
        IStreamConnectionCallback m_callback;
        readonly object m_mutex = new object();
        readonly object m_mutexCallback = new object();
        long m_disconnectFlag = 0;  // atomic  
        readonly IStreamConnectionContainerPrivate m_streamConnectionContainer;
        DateTime m_lastReconnectTime = DateTime.Now;
        IList<BufferRef> m_pendingBuffers = new List<BufferRef>();
    }


}   // namespace finalmq
