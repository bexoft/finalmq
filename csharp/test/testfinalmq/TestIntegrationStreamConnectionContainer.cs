using System;
using System.Threading;
using System.Security.Cryptography.X509Certificates;
using System.Net.Sockets;
using System.Diagnostics;

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
        public bool Received(IStreamConnection connection, byte[] buffer, int count)
        {
            return false;
        }
    }



    [Collection("TestCollectionSocket")]
    public class TestIntegrationStreamConnectionContainer : IDisposable
    {
        readonly StreamConnectionContainer m_connectionContainer = new StreamConnectionContainer();
        readonly Mock<IStreamConnectionCallback> m_mockBindCallback = new Mock<IStreamConnectionCallback>();
        readonly Mock<IStreamConnectionCallback> m_mockClientCallback = new Mock<IStreamConnectionCallback>();
        readonly Mock<IStreamConnectionCallback> m_mockServerCallback = new Mock<IStreamConnectionCallback>();

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
            Debug.Assert(connBind.GetConnectionData().endpoint == "tcp://*:3333");
        }

    }
}
