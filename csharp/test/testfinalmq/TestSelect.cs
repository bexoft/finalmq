using System;
using System.Runtime;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

using Xunit;
using Moq;

using finalmq;

namespace testfinalmq
{
    [Collection("TestCollectionPoller")]
    public class TestSelect : IDisposable
    {
        static int MILLITOMICRO = 1000;
        static int TIMEOUT = 10;
        static int NUMBER_OF_BYTES_TO_READ = 20;


        IPoller m_poller = new PollerImplSelect();
        Socket? m_controlSocketRead = new Socket(SocketType.Stream, ProtocolType.Tcp);
        Socket? m_controlSocketWrite = new Socket(SocketType.Stream, ProtocolType.Tcp);
        Mock<IPlatform> m_mockPlatform = new Mock<IPlatform>();

        public TestSelect()
        {
            Platform.Instance = m_mockPlatform.Object;

            m_mockPlatform.Setup(x => x.MakeSocketPair(out m_controlSocketWrite, out m_controlSocketRead));
            m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(),0,1,SocketFlags.None)).Returns(1);

            m_poller.Init();

            Socket? socketAny = It.IsAny<Socket>();
            m_mockPlatform.Verify(x => x.MakeSocketPair(out socketAny, out socketAny), Times.Once);
            m_mockPlatform.Verify(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None), Times.Once);
        }

        public void Dispose()
        {
            if (m_controlSocketRead != null)
            {
                m_controlSocketRead.Dispose();
            }
            if (m_controlSocketWrite != null)
            {
                m_controlSocketWrite.Dispose();
            }
            Platform.Instance = new PlatformImpl();
        }

        [Fact]
        public void TestTimeout()
        {
            int timeout = TIMEOUT * MILLITOMICRO;

            m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                               It.IsAny<IList<Socket>>(),
                                               It.IsAny<IList<Socket>>(),
                                               timeout))
                .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) => {
                    Assert.Equal(1, checkRead.Count);
                    Assert.Equal(0, checkWrite.Count);
                    Assert.Equal(1, checkError.Count);
                    Assert.Equal(m_controlSocketRead, checkRead[0]);
                    Assert.Equal(m_controlSocketRead, checkError[0]);
                    checkRead.Clear();
                    checkWrite.Clear();
                    checkError.Clear();
                });

            PollerResult result = m_poller.Wait(TIMEOUT);

            m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                It.IsAny<IList<Socket>>(),
                                                It.IsAny<IList<Socket>>(),
                                                timeout), Times.Once);

            Assert.True(result.Timeout);
            Assert.Equal(0, result.DescriptorInfos.Count);

        }


        [Fact]
        public void TestAddSocketReadableWait()
        {
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, m_controlSocketWrite);
            if (m_controlSocketWrite == null)
            {
                return;
            }
            using (Socket socket = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                m_poller.AddSocketEnableRead(socket);

                int timeout = TIMEOUT * MILLITOMICRO;

                m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   timeout))
                    .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) =>
                    {
                        Assert.Equal(2, checkRead.Count);
                        Assert.Equal(0, checkWrite.Count);
                        Assert.Equal(2, checkError.Count);
                        Assert.Equal(m_controlSocketRead, checkRead[0]);
                        Assert.Equal(m_controlSocketRead, checkError[0]);
                        Assert.Equal(socket, checkRead[1]);
                        Assert.Equal(socket, checkError[1]);
                        checkRead.Clear();
                        checkWrite.Clear();
                        checkError.Clear();
                        checkRead.Add(socket);
                    });

                m_mockPlatform.Setup(x => x.GetAvailable(socket)).Returns(NUMBER_OF_BYTES_TO_READ);

                PollerResult result = m_poller.Wait(TIMEOUT);

                m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    timeout), Times.Once);

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.True(result.DescriptorInfos[socket].socket == socket);
                Assert.False(result.DescriptorInfos[socket].disconnected);
                Assert.True(result.DescriptorInfos[socket].readable);
                Assert.False(result.DescriptorInfos[socket].writable);
                Assert.Equal(NUMBER_OF_BYTES_TO_READ, result.DescriptorInfos[socket].bytesToRead);
            }
        }

        [Fact]
        public void TestError()
        {
            int timeout = TIMEOUT * MILLITOMICRO;

            m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                               It.IsAny<IList<Socket>>(),
                                               It.IsAny<IList<Socket>>(),
                                               timeout))
                .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) => {
                    throw new System.Net.Sockets.SocketException();
                });

            Assert.Throws<System.Net.Sockets.SocketException>(() => m_poller.Wait(TIMEOUT));

            m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                It.IsAny<IList<Socket>>(),
                                                It.IsAny<IList<Socket>>(),
                                                timeout), Times.Once);
        }

        [Fact]
        public void TestAddSocketReleaseByControlSocket()
        {
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, m_controlSocketWrite);
            Assert.NotEqual(nullSocket, m_controlSocketRead);
            if (m_controlSocketWrite == null || m_controlSocketRead == null)
            {
                return;
            }
            using (Socket socket = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                m_poller.AddSocketEnableRead(socket);

                int timeout = TIMEOUT * MILLITOMICRO;

                int sequence = 0;
                m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   timeout))
                    .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) =>
                    {
                        Assert.Equal(2, checkRead.Count);
                        Assert.Equal(0, checkWrite.Count);
                        Assert.Equal(2, checkError.Count);
                        Assert.Equal(m_controlSocketRead, checkRead[0]);
                        Assert.Equal(m_controlSocketRead, checkError[0]);
                        Assert.Equal(socket, checkRead[1]);
                        Assert.Equal(socket, checkError[1]);
                        checkRead.Clear();
                        checkWrite.Clear();
                        checkError.Clear();

                        if (sequence == 0)
                        {
                            checkRead.Add(m_controlSocketRead);
                            sequence = 1;
                        }
                        else if (sequence == 1)
                        {
                        }
                    });

                m_mockPlatform.Setup(x => x.GetAvailable(m_controlSocketRead)).Returns(1);

                PollerResult result = m_poller.Wait(TIMEOUT);

                m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    timeout), Times.Exactly(2));

                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);
            }
        }

        [Fact]
        public void TestAddSocketReadableWaitSocketDescriptorsChanged()
        {
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, m_controlSocketWrite);
            if (m_controlSocketWrite == null)
            {
                return;
            }
            using (Socket socket = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                m_poller.AddSocketEnableRead(socket);

                int timeout = TIMEOUT * MILLITOMICRO;

                m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   timeout))
                    .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) =>
                    {
                        Assert.Equal(2, checkRead.Count);
                        Assert.Equal(0, checkWrite.Count);
                        Assert.Equal(2, checkError.Count);
                        Assert.Equal(m_controlSocketRead, checkRead[0]);
                        Assert.Equal(m_controlSocketRead, checkError[0]);
                        Assert.Equal(socket, checkRead[1]);
                        Assert.Equal(socket, checkError[1]);
                        checkRead.Clear();
                        checkWrite.Clear();
                        checkError.Clear();
                        checkRead.Add(socket);

                        m_poller.RemoveSocket(socket);
                    });

                m_mockPlatform.Setup(x => x.GetAvailable(socket)).Returns(NUMBER_OF_BYTES_TO_READ);

                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                PollerResult result = m_poller.Wait(TIMEOUT);

                m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    timeout), Times.Exactly(1));

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.True(result.DescriptorInfos[socket].socket == socket);
                Assert.False(result.DescriptorInfos[socket].disconnected);
                Assert.True(result.DescriptorInfos[socket].readable);
                Assert.False(result.DescriptorInfos[socket].writable);
                Assert.Equal(NUMBER_OF_BYTES_TO_READ, result.DescriptorInfos[socket].bytesToRead);
            }
        }

        [Fact]
        public void TestAddSocketDisconnect()
        {
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, m_controlSocketWrite);
            if (m_controlSocketWrite == null)
            {
                return;
            }
            using (Socket socket = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                m_poller.AddSocketEnableRead(socket);

                int timeout = TIMEOUT * MILLITOMICRO;

                m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   timeout))
                    .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) =>
                    {
                        Assert.Equal(2, checkRead.Count);
                        Assert.Equal(0, checkWrite.Count);
                        Assert.Equal(2, checkError.Count);
                        Assert.Equal(m_controlSocketRead, checkRead[0]);
                        Assert.Equal(m_controlSocketRead, checkError[0]);
                        Assert.Equal(socket, checkRead[1]);
                        Assert.Equal(socket, checkError[1]);
                        checkRead.Clear();
                        checkWrite.Clear();
                        checkError.Clear();
                        checkRead.Add(socket);
                    });

                m_mockPlatform.Setup(x => x.GetAvailable(socket)).Returns(0);

                PollerResult result = m_poller.Wait(TIMEOUT);

                m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    timeout), Times.Once);

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.True(result.DescriptorInfos[socket].socket == socket);
                Assert.False(result.DescriptorInfos[socket].disconnected);
                Assert.True(result.DescriptorInfos[socket].readable);
                Assert.False(result.DescriptorInfos[socket].writable);
                Assert.Equal(0, result.DescriptorInfos[socket].bytesToRead);
            }
        }

        [Fact]
        public void TestAddSocketAvailableError()
        {
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, m_controlSocketWrite);
            if (m_controlSocketWrite == null)
            {
                return;
            }
            using (Socket socket = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                m_poller.AddSocketEnableRead(socket);

                int timeout = TIMEOUT * MILLITOMICRO;

                m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   timeout))
                    .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) =>
                    {
                        Assert.Equal(2, checkRead.Count);
                        Assert.Equal(0, checkWrite.Count);
                        Assert.Equal(2, checkError.Count);
                        Assert.Equal(m_controlSocketRead, checkRead[0]);
                        Assert.Equal(m_controlSocketRead, checkError[0]);
                        Assert.Equal(socket, checkRead[1]);
                        Assert.Equal(socket, checkError[1]);
                        checkRead.Clear();
                        checkWrite.Clear();
                        checkError.Clear();
                        checkRead.Add(socket);
                    });

                m_mockPlatform.Setup(x => x.GetAvailable(socket)).Callback((Socket socket) =>
                {
                    throw new System.Net.Sockets.SocketException();
                });

                PollerResult result = m_poller.Wait(TIMEOUT);

                m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    timeout), Times.Once);

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.True(result.DescriptorInfos[socket].socket == socket);
                Assert.False(result.DescriptorInfos[socket].disconnected);
                Assert.True(result.DescriptorInfos[socket].readable);
                Assert.False(result.DescriptorInfos[socket].writable);
                Assert.Equal(0, result.DescriptorInfos[socket].bytesToRead);
            }
        }

        [Fact]
        public void TestAddSocketWritableWait()
        {
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, m_controlSocketWrite);
            if (m_controlSocketWrite == null)
            {
                return;
            }
            using (Socket socket = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                m_poller.AddSocketEnableRead(socket);
                m_poller.EnableWrite(socket);

                int timeout = TIMEOUT * MILLITOMICRO;

                m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   timeout))
                    .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) =>
                    {
                        Assert.Equal(2, checkRead.Count);
                        Assert.Equal(1, checkWrite.Count);
                        Assert.Equal(2, checkError.Count);
                        Assert.Equal(m_controlSocketRead, checkRead[0]);
                        Assert.Equal(m_controlSocketRead, checkError[0]);
                        Assert.Equal(socket, checkRead[1]);
                        Assert.Equal(socket, checkWrite[0]);
                        Assert.Equal(socket, checkError[1]);
                        checkRead.Clear();
                        checkWrite.Clear();
                        checkError.Clear();
                        checkWrite.Add(socket);
                    });

                PollerResult result = m_poller.Wait(TIMEOUT);

                m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    timeout), Times.Once);

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.True(result.DescriptorInfos[socket].socket == socket);
                Assert.False(result.DescriptorInfos[socket].disconnected);
                Assert.False(result.DescriptorInfos[socket].readable);
                Assert.True(result.DescriptorInfos[socket].writable);
                Assert.Equal(0, result.DescriptorInfos[socket].bytesToRead);
            }
        }

        [Fact]
        public void TestAddSocketDisableWritableWait()
        {
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, m_controlSocketWrite);
            if (m_controlSocketWrite == null)
            {
                return;
            }
            using (Socket socket = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None)).Returns(1);

                m_poller.AddSocketEnableRead(socket);
                m_poller.EnableWrite(socket);
                m_poller.DisableWrite(socket);

                int timeout = TIMEOUT * MILLITOMICRO;

                m_mockPlatform.Setup(x => x.Select(It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   It.IsAny<IList<Socket>>(),
                                                   timeout))
                    .Callback((IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds) =>
                    {
                        Assert.Equal(2, checkRead.Count);
                        Assert.Equal(0, checkWrite.Count);
                        Assert.Equal(2, checkError.Count);
                        Assert.Equal(m_controlSocketRead, checkRead[0]);
                        Assert.Equal(m_controlSocketRead, checkError[0]);
                        Assert.Equal(socket, checkRead[1]);
                        Assert.Equal(socket, checkError[1]);
                        checkRead.Clear();
                        checkWrite.Clear();
                        checkError.Clear();
                    });

                PollerResult result = m_poller.Wait(TIMEOUT);

                m_mockPlatform.Verify(x => x.Select(It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    It.IsAny<IList<Socket>>(),
                                                    timeout), Times.Once);

                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);
            }
        }

    }
}
