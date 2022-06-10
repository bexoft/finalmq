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
using System.IO;
using System.Net.Sockets;
using System.Net.Security;

namespace finalmq {

    public class CertificateData
    {
        public CertificateData(bool ssl = false,
                               X509Certificate serverCertificate = null,
                               bool clientCertificateRequired = false,
                               SslProtocols enabledSslProtocols = SslProtocols.Default,
                               bool checkCertificateRevocation = false,
                               RemoteCertificateValidationCallback userCertificateValidationCallback = null,
                               LocalCertificateSelectionCallback userCertificateSelectionCallback = null,
                               EncryptionPolicy encryptionPolicy = EncryptionPolicy.RequireEncryption) 
        {
            this.ssl = ssl;
            this.serverCertificate = serverCertificate;
            this.clientCertificateRequired = clientCertificateRequired;
            this.enabledSslProtocols = enabledSslProtocols;
            this.checkCertificateRevocation = checkCertificateRevocation;
            this.userCertificateValidationCallback = userCertificateValidationCallback;
            this.userCertificateSelectionCallback = userCertificateSelectionCallback;
            this.encryptionPolicy = encryptionPolicy;
        }

        public bool ssl = false;
        public X509Certificate serverCertificate = null;
        public bool clientCertificateRequired = false;
        public SslProtocols enabledSslProtocols = SslProtocols.Default;
        public bool checkCertificateRevocation = false;
        public RemoteCertificateValidationCallback userCertificateValidationCallback = null;
        public LocalCertificateSelectionCallback userCertificateSelectionCallback = null;
        public EncryptionPolicy encryptionPolicy = EncryptionPolicy.RequireEncryption;
    }


    public class BindProperties
    {
        public BindProperties(CertificateData certificateData = null)
        {
            this.certificateData = certificateData;
            if (this.certificateData == null)
            {
                this.certificateData = new CertificateData();
            }
        }
        public CertificateData CertificateData 
        {   
            get => certificateData;
            set
            {
                certificateData = value;
            }
        }
        private CertificateData certificateData = null;
        //private Variant protocolData;
    }

    public class ConnectConfig
    {
        public int reconnectInterval = 5000;       ///< if the server is not available, you can pass a reconnection intervall in [ms]
        public int totalReconnectDuration = -1;    ///< if the server is not available, you can pass a duration in [ms] how long the reconnect shall happen. -1 means: try for ever.
    }

    public class ConnectProperties
    {
    //    CertificateData certificateData;
        public ConnectConfig config = new ConnectConfig();
    //    Variant protocolData;
    }



    public interface IStreamConnectionCallback
    {
        IStreamConnectionCallback Connected(IStreamConnection connection);
        void Disconnected(IStreamConnection connection);
        bool Received(IStreamConnection connection, Socket socket, int bytesToRead);
    }




    public interface IStreamConnection
    {
        bool SendMessage(IMessage msg);
        ConnectionData GetConnectionData();
        ConnectionState GetConnectionState();
        long GetConnectionId();
        Socket GetSocket();
        void Disconnect();
    };

    public interface IStreamConnectionPrivate : IStreamConnection
    {
        bool Connect();
        Socket GetSocketPrivate();
        bool SendPendingMessages();
        bool CheckEdgeConnected();
        bool DoReconnect();
        bool ChangeStateForDisconnect();
        bool GetDisconnectFlag();
        void UpdateConnectionData(ConnectionData connectionData);

        void Connected(IStreamConnection connection);
        void Disconnected(IStreamConnection connection);
        bool Received(IStreamConnection connection, Socket socket, int bytesToRead);
    };

    class StreamConnection : IStreamConnectionPrivate
    {
        public StreamConnection(ConnectionData connectionData, Stream stream, IStreamConnectionCallback callback)
        {

        }

        public bool ChangeStateForDisconnect()
        {
            throw new System.NotImplementedException();
        }

        public bool CheckEdgeConnected()
        {
            throw new System.NotImplementedException();
        }

        public bool Connect()
        {
            throw new System.NotImplementedException();
        }

        public void Connected(IStreamConnection connection)
        {
            throw new System.NotImplementedException();
        }

        public void Disconnect()
        {
            throw new System.NotImplementedException();
        }

        public void Disconnected(IStreamConnection connection)
        {
            throw new System.NotImplementedException();
        }

        public bool DoReconnect()
        {
            throw new System.NotImplementedException();
        }

        public ConnectionData GetConnectionData()
        {
            throw new System.NotImplementedException();
        }

        public long GetConnectionId()
        {
            throw new System.NotImplementedException();
        }

        public ConnectionState GetConnectionState()
        {
            throw new System.NotImplementedException();
        }

        public bool GetDisconnectFlag()
        {
            throw new System.NotImplementedException();
        }

        public Socket GetSocket()
        {
            throw new System.NotImplementedException();
        }

        public Socket GetSocketPrivate()
        {
            throw new System.NotImplementedException();
        }

        public bool Received(IStreamConnection connection, Socket socket, int bytesToRead)
        {
            throw new System.NotImplementedException();
        }

        public bool SendMessage(IMessage msg)
        {
            throw new System.NotImplementedException();
        }

        public bool SendPendingMessages()
        {
            throw new System.NotImplementedException();
        }

        public void UpdateConnectionData(ConnectionData connectionData)
        {
            throw new System.NotImplementedException();
        }
    }


}   // namespace finalmq
