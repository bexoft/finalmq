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
    public class UnitTestSelect : IDisposable
    {
        static int MILLITOMICRO = 1000;
        static int TIMEOUT = 10;
        static int NUMBER_OF_BYTES_TO_READ = 20;


        IPoller m_poller = new PollerImplSelect();
        Socket m_controlSocketRead = new Socket(SocketType.Stream, ProtocolType.Tcp);
        Socket m_controlSocketWrite = new Socket(SocketType.Stream, ProtocolType.Tcp);
        Mock<IPlatform> m_mockPlatform = new Mock<IPlatform>();

        public UnitTestSelect()
        {
            Platform.Instance = m_mockPlatform.Object;

            m_mockPlatform.Setup(x => x.MakeSocketPair(out m_controlSocketWrite, out m_controlSocketRead));
            m_mockPlatform.Setup(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(),0,1,SocketFlags.None)).Returns(1);

            m_poller.Init();

            Socket socketAny = It.IsAny<Socket>();
            m_mockPlatform.Verify(x => x.MakeSocketPair(out socketAny, out socketAny), Times.Once);
            m_mockPlatform.Verify(x => x.Send(m_controlSocketWrite, It.IsAny<byte[]>(), 0, 1, SocketFlags.None), Times.Once);
        }

        public void Dispose()
        {
            m_controlSocketRead.Dispose();
            m_controlSocketWrite.Dispose();
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
                Assert.Equal(result.DescriptorInfos[socket].bytesToRead, NUMBER_OF_BYTES_TO_READ);
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
        public void TestAddSocketReadableWaitSocketDescriptorsChanged()
        {
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
                Assert.Equal(result.DescriptorInfos[socket].bytesToRead, NUMBER_OF_BYTES_TO_READ);
            }
        }

        [Fact]
        public void TestAddSocketDisconnect()
        {
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
    }
}
