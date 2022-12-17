using System;
using System.Threading;
using System.Security.Cryptography.X509Certificates;
using System.Net.Sockets;
using System.Diagnostics;
using System.Text;

using Xunit;
using Moq;

using finalmq;
using System.Net.Security;
using System.Security.Authentication;

namespace testfinalmq
{


    [Collection("TestCollectionStreamConnectionContainerSsl")]
    public class TestIntegrationStreamConnectionContainerSsl : IDisposable
    {
        readonly StreamConnectionContainer m_connectionContainer = new StreamConnectionContainer();
        readonly Mock<IStreamConnectionCallback> m_mockBindCallback = new Mock<IStreamConnectionCallback>();
        readonly Mock<IStreamConnectionCallback> m_mockClientCallback = new Mock<IStreamConnectionCallback>();
        readonly Mock<IStreamConnectionCallback> m_mockServerCallback = new Mock<IStreamConnectionCallback>();
        readonly string MESSAGE1_BUFFER = "Hello";

        public TestIntegrationStreamConnectionContainerSsl()
        {
        }

        public void Dispose()
        {
            m_connectionContainer.Dispose();
        }

        public static bool ValidateServerCertificate(
                     object sender,
                     X509Certificate? certificate,
                     X509Chain? chain,
                     SslPolicyErrors sslPolicyErrors)
        {
            sslPolicyErrors &= ~SslPolicyErrors.RemoteCertificateChainErrors;
            sslPolicyErrors &= ~SslPolicyErrors.RemoteCertificateNameMismatch;
            if (sslPolicyErrors == SslPolicyErrors.None)
            {
                return true;
            }

            Console.WriteLine("Certificate error: {0}", sslPolicyErrors);

            // Do not allow this client to communicate with unauthenticated servers.
            return false;
        }

        [Fact]
        public void TestBind()
        {
            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));
        }

        [Fact]
        public void TestUnbind()
        {
            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));
            m_connectionContainer.Unbind("tcp://*:3005");
        }

        [Fact]
        public void TestBindConnect()
        {
            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));

            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar connectClient = new CondVar();
            CondVar connectServer = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                })
                .Returns(m_mockServerCallback.Object);
            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connConnect = connection;
                    connectClient.Set();
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connectServer.Set();
                })
                .Returns((IStreamConnectionCallback?)null);

            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3005", m_mockClientCallback.Object, 
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate))));

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3005");
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_connectionContainer.GetConnection(connection.ConnectionData.ConnectionId) == connection);
        }

        [Fact]
        public void TestBindConnectSend()
        {
            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));

            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar condVarReceived = new CondVar();
            CondVar condVarClientConnected = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                })
                .Returns(m_mockServerCallback.Object);
            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connConnect = connection;
                    condVarClientConnected.Set();
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Received(It.IsAny<IStreamConnection>(), It.IsAny<byte[]>(), It.IsAny<int>()))
                .Callback((IStreamConnection connection, byte[] buffer, int count) =>
                {
                    Debug.Assert(Encoding.UTF8.GetString(buffer, 0, count) == MESSAGE1_BUFFER);
                    condVarReceived.Set();
                });


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3005", m_mockClientCallback.Object,
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate))));
            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            Debug.Assert(condVarReceived.Wait(5000));
            Debug.Assert(condVarClientConnected.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3005");
        }

        [Fact]
        public void TestConnectBind()
        {
            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                })
                .Returns(m_mockServerCallback.Object);
            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connConnect = connection;
                    condVarConnectClient.Set();
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    condVarConnectServer.Set();
                })
                .Returns((IStreamConnectionCallback?)null);


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3005", m_mockClientCallback.Object, 
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate)), new ConnectConfig(1)));

            Thread.Sleep(4000);

            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3005");
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_connectionContainer.GetConnection(connection.ConnectionData.ConnectionId) == connection);
        }

        [Fact]
        public void TestReconnectExpires()
        {
            IStreamConnection? connDisconnect = null;
            CondVar condVarDisconnectClient = new CondVar();

            m_mockClientCallback.Setup(x => x.Disconnected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connDisconnect = connection;
                    condVarDisconnectClient.Set();
                });

            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3005", m_mockClientCallback.Object, 
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate)), new ConnectConfig(1, 1)));

            Debug.Assert(condVarDisconnectClient.Wait(5000));

            //m_mockClientCallback.Verify(x => x.Disconnected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connDisconnect != null);
            Debug.Assert(connDisconnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_DISCONNECTED);
            Debug.Assert(m_connectionContainer.TryGetConnection(connection.ConnectionData.ConnectionId) == null);
        }

        [Fact]
        public void TestBindConnectDisconnect()
        {
            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));

            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                })
                .Returns(m_mockServerCallback.Object);
            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connConnect = connection;
                    condVarConnectClient.Set();
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    condVarConnectServer.Set();
                })
                .Returns((IStreamConnectionCallback?)null);


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3005", m_mockClientCallback.Object,
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate))));

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3005");

            IStreamConnection? connDisconnect = null;
            CondVar condVarDisconnectClient = new CondVar();
            CondVar condVarDisconnectServer = new CondVar();

            m_mockClientCallback.Setup(x => x.Disconnected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connDisconnect = connection;
                    condVarDisconnectClient.Set();
                });
            m_mockServerCallback.Setup(x => x.Disconnected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    condVarDisconnectServer.Set();
                });

            connection.Disconnect();

            Debug.Assert(condVarDisconnectClient.Wait(5000));
            Debug.Assert(condVarDisconnectServer.Wait(5000));

            Debug.Assert(connDisconnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_DISCONNECTED);
            Debug.Assert(m_connectionContainer.TryGetConnection(connection.ConnectionData.ConnectionId) == null);
        }

        [Fact]
        public void testGetAllConnections()
        {
            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));

            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                })
                .Returns(m_mockServerCallback.Object);
            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connConnect = connection;
                    condVarConnectClient.Set();
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    condVarConnectServer.Set();
                })
                .Returns((IStreamConnectionCallback?)null);


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3005", m_mockClientCallback.Object,
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate))));

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3005");

            IList<IStreamConnection> connections = m_connectionContainer.GetAllConnections();
            Debug.Assert(connections.Count() == 2);
            if (connections[0] == connBind)
            {
                Debug.Assert(connections[1] == connConnect);
            }
            else
            {
                Debug.Assert(connections[0] == connConnect);
                Debug.Assert(connections[1] == connBind);
            }
        }

        [Fact]
        public void TestBindLateConnect()
        {
            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));

            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar connectClient = new CondVar();
            CondVar connectServer = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                })
                .Returns(m_mockServerCallback.Object);
            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connConnect = connection;
                    connectClient.Set();
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connectServer.Set();
                })
                .Returns((IStreamConnectionCallback?)null);

            IStreamConnection connection = m_connectionContainer.CreateConnection(m_mockClientCallback.Object);
            m_connectionContainer.Connect("tcp://localhost:3005", connection, 
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate)), new ConnectConfig(1)));

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3005");
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_connectionContainer.GetConnection(connection.ConnectionData.ConnectionId) == connection);
        }

        [Fact]
        public void TestSendLateConnectBind()
        {
            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar condVarReceived = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                })
                .Returns(m_mockServerCallback.Object);
            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connConnect = connection;
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                })
                .Returns((IStreamConnectionCallback?)null);
            m_mockServerCallback.Setup(x => x.Received(It.IsAny<IStreamConnection>(), It.IsAny<byte[]>(), It.IsAny<int>()))
                .Callback((IStreamConnection connection, byte[] buffer, int count) =>
                {
                    Debug.Assert(Encoding.UTF8.GetString(buffer, 0, count) == MESSAGE1_BUFFER);
                    condVarReceived.Set();
                });


            IStreamConnection connection = m_connectionContainer.CreateConnection(m_mockClientCallback.Object);
            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            m_connectionContainer.Connect("tcp://localhost:3005", connection, 
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate)), new ConnectConfig(1)));

            Thread.Sleep(4000);

            m_connectionContainer.Bind("tcp://*:3005", m_mockBindCallback.Object, new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));


            Debug.Assert(condVarReceived.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3005");
        }

        [Fact]
        public void TestCreateConnectionDisconnect()
        {
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            IStreamConnection connection = m_connectionContainer.CreateConnection(m_mockClientCallback.Object);
            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            IStreamConnection? connDisconnect = null;
            CondVar condVarDisconnectClient = new CondVar();

            m_mockClientCallback.Setup(x => x.Disconnected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) => {
                    connDisconnect = connection;
                    condVarDisconnectClient.Set();
                });

            connection.Disconnect();

            Debug.Assert(condVarDisconnectClient.Wait(5000));

            Debug.Assert(connDisconnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_DISCONNECTED);
            Debug.Assert(m_connectionContainer.TryGetConnection(connection.ConnectionData.ConnectionId) == null);
        }
    }
}

