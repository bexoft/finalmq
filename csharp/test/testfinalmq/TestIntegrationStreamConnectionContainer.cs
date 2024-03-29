using System;
using System.Threading;
using System.Security.Cryptography.X509Certificates;
using System.Net.Sockets;
using System.Diagnostics;
using System.Text;

using Xunit;
using Moq;

using finalmq;

namespace testfinalmq
{

    [Collection("TestCollectionStreamConnectionContainer")]
    public class TestIntegrationStreamConnectionContainer : IDisposable
    {
        readonly StreamConnectionContainer m_connectionContainer = new StreamConnectionContainer();
        readonly Mock<IStreamConnectionCallback> m_mockBindCallback = new Mock<IStreamConnectionCallback>();
        readonly Mock<IStreamConnectionCallback> m_mockClientCallback = new Mock<IStreamConnectionCallback>();
        readonly Mock<IStreamConnectionCallback> m_mockServerCallback = new Mock<IStreamConnectionCallback>();
        readonly string MESSAGE1_BUFFER = "Hello";


        public TestIntegrationStreamConnectionContainer()
        {
        }

        public void Dispose()
        {
            m_connectionContainer.Dispose();
        }

        [Fact]
        public void TestBind()
        {
            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);
        }

        [Fact]
        public void TestUnbind()
        {
            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);
            m_connectionContainer.Unbind("tcp://*:3004");
        }

        [Fact]
        public void TestBindConnect()
        {
            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);

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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3004", m_mockClientCallback.Object);

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_connectionContainer.GetConnection(connection.ConnectionData.ConnectionId) == connection);
        }

        [Fact]
        public void TestBindConnectSend()
        {
            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);

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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3004", m_mockClientCallback.Object);

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
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");
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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3004", m_mockClientCallback.Object, new ConnectProperties(null, new ConnectConfig(1)));

            Thread.Sleep(4000);

            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");
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

            m_connectionContainer.CheckReconnectInterval = 1;
            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3004", m_mockClientCallback.Object, new ConnectProperties(null, new ConnectConfig(1, 1)));

            Debug.Assert(condVarDisconnectClient.Wait(10000));

            //m_mockClientCallback.Verify(x => x.Disconnected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connDisconnect != null);
            Debug.Assert(connDisconnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_DISCONNECTED);
            Debug.Assert(m_connectionContainer.TryGetConnection(connection.ConnectionData.ConnectionId) == null);
        }

        [Fact]
        public void TestBindConnectDisconnect()
        {
            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);

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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3004", m_mockClientCallback.Object);

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");

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
            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);

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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3004", m_mockClientCallback.Object);

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");

            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");
            Debug.Assert(connConnect == connection);

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
            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);

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
            m_connectionContainer.Connect("tcp://localhost:3004", connection, new ConnectProperties(null, new ConnectConfig(1)));

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_connectionContainer.GetConnection(connection.ConnectionData.ConnectionId) == connection);
        }

        [Fact]
        public void TestSendLateConnectBind()
        {
            IStreamConnection? connBind = null;
            IStreamConnection? connConnect = null;
            CondVar condVarReceived = new CondVar();
            CondVar condVarClientConnected = new CondVar();
            CondVar condVarBindServer = new CondVar();

            m_mockBindCallback.Setup(x => x.Connected(It.IsAny<IStreamConnection>()))
                .Callback((IStreamConnection connection) =>
                {
                    connBind = connection;
                    condVarBindServer.Set();
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


            IStreamConnection connection = m_connectionContainer.CreateConnection(m_mockClientCallback.Object);
            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            m_connectionContainer.Connect("tcp://localhost:3004", connection, new ConnectProperties(null, new ConnectConfig(1)));

            Thread.Sleep(4000);

            m_connectionContainer.Bind("tcp://*:3004", m_mockBindCallback.Object);


            Debug.Assert(condVarReceived.Wait(5000));
            Debug.Assert(condVarClientConnected.Wait(5000));
            Debug.Assert(condVarBindServer.Wait(5000));

            //m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            //m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3004");
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

