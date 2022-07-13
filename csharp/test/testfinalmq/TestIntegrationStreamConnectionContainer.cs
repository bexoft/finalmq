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

    class StreamConnectionCallback : IStreamConnectionCallback
    {
        public IStreamConnectionCallback? Connected(IStreamConnection connection)
        {
            return null;
        }
        public void Disconnected(IStreamConnection connection)
        {

        }
        public void Received(IStreamConnection connection, byte[] buffer, int count)
        {
        }
    }



    [Collection("TestCollectionSocket")]
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
            m_connectionContainer.Bind("tcp://*:3333", m_mockBindCallback.Object);
        }

        [Fact]
        public void TestUnbind()
        {
            m_connectionContainer.Bind("tcp://*:3333", m_mockBindCallback.Object);
            m_connectionContainer.Unbind("tcp://*:3333");
        }

        [Fact]
        public void TestBindConnect()
        {
            m_connectionContainer.Bind("tcp://*:3333", m_mockBindCallback.Object);

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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3333", m_mockClientCallback.Object);

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));
            
            m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3333");
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_connectionContainer.GetConnection(connection.ConnectionData.ConnectionId) == connection);
        }

        [Fact]
        public void TestBindConnectSend()
        {
            m_connectionContainer.Bind("tcp://*:3333", m_mockBindCallback.Object);

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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3333", m_mockClientCallback.Object);

            IMessage message = new ProtocolMessage(0);
            message.AddSendPayload(Encoding.UTF8.GetBytes(MESSAGE1_BUFFER));
            connection.SendMessage(message);

            Debug.Assert(condVarReceived.Wait(5000));

            m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3333");
        }

        [Fact]
        public void TestConnectBind()
        {
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


            IStreamConnection connection = m_connectionContainer.Connect("tcp://localhost:3333", m_mockClientCallback.Object, new ConnectProperties(null, new ConnectConfig(1)));

            Thread.Sleep(4000);

            m_connectionContainer.Bind("tcp://*:3333", m_mockBindCallback.Object);

            Debug.Assert(connectClient.Wait(5000));
            Debug.Assert(connectServer.Wait(5000));

            m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3333");
            Debug.Assert(connection.ConnectionData.ConnectionState == ConnectionState.CONNECTIONSTATE_CONNECTED);
            Debug.Assert(m_connectionContainer.GetConnection(connection.ConnectionData.ConnectionId) == connection);
        }

        [Fact]
        public void TestSendConnectBind()
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

            m_connectionContainer.Connect("tcp://localhost:3333", connection, new ConnectProperties(null, new ConnectConfig(1)));

            Thread.Sleep(5000);

            m_connectionContainer.Bind("tcp://*:3333", m_mockBindCallback.Object);


            Debug.Assert(condVarReceived.Wait(4000));

            m_mockBindCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockClientCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);
            m_mockServerCallback.Verify(x => x.Connected(It.IsAny<IStreamConnection>()), Times.Once);

            Debug.Assert(connBind != null);
            Debug.Assert(connConnect != null);
            Debug.Assert(connConnect == connection);
            Debug.Assert(connBind.ConnectionData.Endpoint == "tcp://*:3333");
        }

    }
}


