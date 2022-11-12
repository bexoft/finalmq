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

    [Collection("TestCollectionSocket")]
    public class TestIntegrationProtocolStreamSessionContainer : IDisposable
    {
        readonly IProtocolSessionContainer m_sessionContainer = new ProtocolSessionContainer();
        readonly Mock<IProtocolSessionCallback> m_mockClientCallback = new Mock<IProtocolSessionCallback>();
        readonly Mock<IProtocolSessionCallback> m_mockServerCallback = new Mock<IProtocolSessionCallback>();
        readonly string MESSAGE1_BUFFER = "Hello";

        public void Dispose()
        {
            m_sessionContainer.Dispose();
        }

        [Fact]
        public void TestBind()
        {
            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);
        }

        [Fact]
        public void TestUnbind()
        {
            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);
            m_sessionContainer.Unbind("tcp://*:3333:stream");
        }

        [Fact]
        public void TestBindConnect()
        {
            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);

            IProtocolSession? connConnect = null;
            CondVar connectClient = new CondVar();
            CondVar connectServer = new CondVar();

            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) => {
                    connConnect = connection;
                    connectClient.Set();
                });
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connectServer.Set();
                });


            IProtocolSession connection = m_sessionContainer.Connect("tcp://localhost:3333:stream", m_mockClientCallback.Object);

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));

            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_sessionContainer.GetSession(connection.SessionId) == connection);
        }

        [Fact]
        public void TestBindConnectSend()
        {
            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);

            IProtocolSession? connConnect = null;
            CondVar condVarReceived = new CondVar();

            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connConnect = connection;
                });
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                });
            m_mockServerCallback.Setup(x => x.Received(It.IsAny<IProtocolSession>(), It.IsAny<IMessage>()))
                .Callback((IProtocolSession connection, IMessage message) =>
                {
                    BufferRef bufferRef = message.GetReceivePayload();
                    Debug.Assert(Encoding.UTF8.GetString(bufferRef.Buffer, 0, bufferRef.Length) == MESSAGE1_BUFFER);
                    condVarReceived.Set();
                });

            IProtocolSession connection = m_sessionContainer.Connect("tcp://localhost:3333:stream", m_mockClientCallback.Object);

            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            Debug.Assert(condVarReceived.Wait(5000));

            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
        }

        [Fact]
        public void TestConnectBind()
        {
            IProtocolSession? connConnect = null;
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connConnect = connection;
                    condVarConnectClient.Set();
                });
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) => {
                    condVarConnectServer.Set();
                });


            IProtocolSession connection = m_sessionContainer.Connect("tcp://localhost:3333:stream", m_mockClientCallback.Object, new ConnectProperties(null, new ConnectConfig(1)));

            Thread.Sleep(4000);

            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_sessionContainer.GetSession(connection.SessionId) == connection);
        }

        [Fact]
        public void TestReconnectExpires()
        {
            IProtocolSession? connDisconnect = null;
            CondVar condVarDisconnectClient = new CondVar();

            m_mockClientCallback.Setup(x => x.Disconnected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connDisconnect = connection;
                    condVarDisconnectClient.Set();
                });

            IProtocolSession connection = m_sessionContainer.Connect("tcp://localhost:3333:stream", m_mockClientCallback.Object, new ConnectProperties(null, new ConnectConfig(1, 1)));

            Debug.Assert(condVarDisconnectClient.Wait(5000));

            Debug.Assert(connDisconnect != null);
            Debug.Assert(connDisconnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_DISCONNECTED);
            Debug.Assert(m_sessionContainer.TryGetSession(connection.SessionId) == null);
        }

        [Fact]
        public void TestBindConnectDisconnect()
        {
            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);

            IProtocolSession? connConnect = null;
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connConnect = connection;
                    condVarConnectClient.Set();
                });
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    condVarConnectServer.Set();
                });


            IProtocolSession connection = m_sessionContainer.Connect("tcp://localhost:3333:stream", m_mockClientCallback.Object);

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);

            IProtocolSession? connDisconnect = null;
            CondVar condVarDisconnectClient = new CondVar();
            CondVar condVarDisconnectServer = new CondVar();

            m_mockClientCallback.Setup(x => x.Disconnected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) => {
                    connDisconnect = connection;
                    condVarDisconnectClient.Set();
                });
            m_mockServerCallback.Setup(x => x.Disconnected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    condVarDisconnectServer.Set();
                });

            connection.Disconnect();

            Debug.Assert(condVarDisconnectClient.Wait(5000));
            Debug.Assert(condVarDisconnectServer.Wait(5000));

            Debug.Assert(connDisconnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_DISCONNECTED);
            Debug.Assert(m_sessionContainer.TryGetSession(connection.SessionId) == null);
        }

        [Fact]
        public void testGetAllSessions()
        {
            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);

            IProtocolSession? connBind = null;
            IProtocolSession? connConnect = null;
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connConnect = connection;
                    condVarConnectClient.Set();
                });
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) => {
                    condVarConnectServer.Set();
                    connBind = connection;
                });


            IProtocolSession connection = m_sessionContainer.Connect("tcp://localhost:3333:stream", m_mockClientCallback.Object);

            Debug.Assert(condVarConnectClient.Wait(5000));
            Debug.Assert(condVarConnectServer.Wait(5000));

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3333");

            IList<IProtocolSession> connections = m_sessionContainer.GetAllSessions();
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
            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);

            IProtocolSession? connConnect = null;
            CondVar connectClient = new CondVar();
            CondVar connectServer = new CondVar();

            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connConnect = connection;
                    connectClient.Set();
                });
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) => {
                    connectServer.Set();
                });

            IProtocolSession connection = m_sessionContainer.CreateSession(m_mockClientCallback.Object);
            connection.Connect("tcp://localhost:3333:stream", new ConnectProperties(null, new ConnectConfig(1)));

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));

            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_sessionContainer.GetSession(connection.SessionId) == connection);
        }

        [Fact]
        public void TestSendLateConnectBind()
        {
            IProtocolSession? connConnect = null;
            CondVar condVarReceived = new CondVar();

            m_mockClientCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) =>
                {
                    connConnect = connection;
                });
            m_mockServerCallback.Setup(x => x.Connected(It.IsAny<IProtocolSession>()))
                .Callback((IProtocolSession connection) => {
                });
            m_mockServerCallback.Setup(x => x.Received(It.IsAny<IProtocolSession>(), It.IsAny<IMessage>()))
                .Callback((IProtocolSession connection, IMessage message) =>
                {
                    BufferRef bufferRef = message.GetReceivePayload();
                    Debug.Assert(Encoding.UTF8.GetString(bufferRef.Buffer, 0, bufferRef.Length) == MESSAGE1_BUFFER);
                    condVarReceived.Set();
                });


            IProtocolSession connection = m_sessionContainer.CreateSession(m_mockClientCallback.Object);
            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            connection.Connect("tcp://localhost:3333:stream", new ConnectProperties(null, new ConnectConfig(1)));

            Thread.Sleep(4000);

            m_sessionContainer.Bind("tcp://*:3333:stream", m_mockServerCallback.Object);


            Debug.Assert(condVarReceived.Wait(5000));

            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
        }

        [Fact]
        public void TestCreateConnectionDisconnect()
        {
            CondVar condVarConnectClient = new CondVar();
            CondVar condVarConnectServer = new CondVar();

            IProtocolSession connection = m_sessionContainer.CreateSession(m_mockClientCallback.Object);
            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            connection.Disconnect();

            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_DISCONNECTED);
            Debug.Assert(m_sessionContainer.TryGetSession(connection.SessionId) == null);
        }

    }
}

