using System;
using System.Runtime;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;

using Xunit;
using Moq;

using finalmq;

namespace testfinalmq
{
    [Collection("Our Test Collection #1")]
    public class TestIntegrationSelect : IDisposable
    {
        //        static int MILLITOMICRO = 1000;
        //        static int TIMEOUT = 10;


        IPoller m_poller = new PollerImplSelect();
        Socket m_controlSocketRead = new Socket(SocketType.Stream, ProtocolType.Tcp);
        Socket m_controlSocketWrite = new Socket(SocketType.Stream, ProtocolType.Tcp);
        Mock<IPlatform> m_mockPlatform = new Mock<IPlatform>();

        public TestIntegrationSelect()
        {
            m_poller.Init();
        }

        public void Dispose()
        {
            m_controlSocketRead.Dispose();
            m_controlSocketWrite.Dispose();
        }

        [Fact]
        public void TestTimeout()
        {
            PollerResult result = m_poller.Wait(0);
            Assert.True(result.Timeout);
            Assert.Equal(0, result.DescriptorInfos.Count);
        }

        [Fact]
        public void TestAddSocketReadableBeforeWait()
        {
            Socket? socketInside = null;
            Socket? socketOutside = null;

            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);

            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            if (socketInside != null && socketOutside != null)
            {
                m_poller.AddSocket(socketInside);
                m_poller.EnableRead(socketInside);
                byte[] BUFFER = new byte[20];
                Platform.Instance.Send(socketOutside, BUFFER, 0, BUFFER.Length);

                PollerResult result = m_poller.Wait(10);

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.Equal(result.DescriptorInfos[socketInside].socket, socketInside);
                Assert.False(result.DescriptorInfos[socketInside].disconnected);
                Assert.True(result.DescriptorInfos[socketInside].readable);
                Assert.False(result.DescriptorInfos[socketInside].writable);
                Assert.Equal(BUFFER.Length, result.DescriptorInfos[socketInside].bytesToRead);
            }
        }

        [Fact]
        public void TestEnableWriteSocketBeforeWait()
        {
            Socket? socketInside = null;
            Socket? socketOutside = null;

            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);

            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            if (socketInside != null && socketOutside != null)
            {
                m_poller.AddSocket(socketInside);
                m_poller.EnableRead(socketInside);
                PollerResult result = m_poller.Wait(0);
                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                m_poller.EnableWrite(socketInside);
                result = m_poller.Wait(10);

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.Equal(result.DescriptorInfos[socketInside].socket, socketInside);
                Assert.False(result.DescriptorInfos[socketInside].disconnected);
                Assert.False(result.DescriptorInfos[socketInside].readable);
                Assert.True(result.DescriptorInfos[socketInside].writable);
                Assert.Equal(0, result.DescriptorInfos[socketInside].bytesToRead);
            }
        }

        [Fact]
        public void TestEnableWriteSocketInsideWait()
        {
            Socket? socketInside = null;
            Socket? socketOutside = null;
            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            if (socketInside != null && socketOutside != null)
            {
                m_poller.AddSocket(socketInside);
                m_poller.EnableRead(socketInside);
                PollerResult result = m_poller.Wait(0);
                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                Thread thread = new Thread(() =>
                {
                    Thread.Sleep(10);
                    m_poller.EnableWrite(socketInside);
                });
                thread.Start();

                result = m_poller.Wait(1000000);

                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.Equal(result.DescriptorInfos[socketInside].socket, socketInside);
                Assert.False(result.DescriptorInfos[socketInside].disconnected);
                Assert.False(result.DescriptorInfos[socketInside].readable);
                Assert.True(result.DescriptorInfos[socketInside].writable);
                Assert.Equal(0, result.DescriptorInfos[socketInside].bytesToRead);

                thread.Join();
            }
        }
        
        [Fact]
        public void TestEnableWriteSocketNotWritable()
        {
            Socket? socketInside = null;
            Socket? socketOutside = null;
            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            if (socketInside != null && socketOutside != null)
            {
                m_poller.AddSocket(socketInside);
                m_poller.EnableRead(socketInside);
                PollerResult result = m_poller.Wait(0);
                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                m_poller.EnableWrite(socketInside);

                try
                {
                    byte[] buffer = new byte[100];
                    while (true)
                    {
                        int res = Platform.Instance.Send(socketInside, buffer, 0, buffer.Length);
                        Assert.Equal(buffer.Length, res);
                    }
                }
                catch (System.Net.Sockets.SocketException)
                {
                }

                result = m_poller.Wait(0);

                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                socketInside.Dispose();
                socketOutside.Dispose();
            }
        }

        [Fact]
        public void TestEnableWriteSocketNotWritableToWritable()
        {
            Socket? socketInside = null;
            Socket? socketOutside = null;
            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            if (socketInside != null && socketOutside != null)
            {
                m_poller.AddSocket(socketInside);
                m_poller.EnableRead(socketInside);
                PollerResult result = m_poller.Wait(0);
                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                try
                {
                    byte[] buffer = new byte[100];
                    while (true)
                    {
                        int res = Platform.Instance.Send(socketInside, buffer, 0, buffer.Length);
                        Assert.Equal(buffer.Length, res);
                    }
                }
                catch (System.Net.Sockets.SocketException)
                {
                }

                m_poller.EnableWrite(socketInside);

                Thread thread = new Thread(() =>
                {
                    Thread.Sleep(10);
                    try
                    {
                        byte[] buffer = new byte[1000];
                        int res = 0;
                        while (res >= 0)
                        {
                            res = Platform.Instance.Receive(socketOutside, buffer, 0, buffer.Length);
                        }
                    }
                    catch (System.Net.Sockets.SocketException)
                    {
                    }
                });
                thread.Start();

                result = m_poller.Wait(1000000);
                Assert.False(result.Timeout);
                Assert.Equal(1, result.DescriptorInfos.Count);
                Assert.Equal(result.DescriptorInfos[socketInside].socket, socketInside);
                Assert.False(result.DescriptorInfos[socketInside].disconnected);
                Assert.False(result.DescriptorInfos[socketInside].readable);
                Assert.True(result.DescriptorInfos[socketInside].writable);
                Assert.Equal(0, result.DescriptorInfos[socketInside].bytesToRead);

                thread.Join();

                socketInside.Dispose();
                socketOutside.Dispose();
            }
        }

        [Fact]
        public void TestDisableWriteSocket()
        {
            Socket? socketInside = null;
            Socket? socketOutside = null;
            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            if (socketInside != null && socketOutside != null)
            {
                m_poller.AddSocket(socketInside);
                m_poller.EnableRead(socketInside);
                PollerResult result = m_poller.Wait(0);
                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                try
                {
                    byte[] buffer = new byte[100];
                    while (true)
                    {
                        int res = Platform.Instance.Send(socketInside, buffer, 0, buffer.Length);
                        Assert.Equal(buffer.Length, res);
                    }
                }
                catch (System.Net.Sockets.SocketException)
                {
                }

                m_poller.EnableWrite(socketInside);

                Thread thread = new Thread(() =>
                {
                    Thread.Sleep(10);
                    m_poller.DisableWrite(socketInside);
                    Thread.Sleep(10);
                    try
                    {
                        byte[] buffer = new byte[1000];
                        int res = 0;
                        while (res >= 0)
                        {
                            res = Platform.Instance.Receive(socketOutside, buffer, 0, buffer.Length);
                        }
                    }
                    catch (System.Net.Sockets.SocketException)
                    {
                    }
                });
                thread.Start();

                result = m_poller.Wait(50);
                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                thread.Join();

                socketInside.Dispose();
                socketOutside.Dispose();
            }
        }

        [Fact]
        public void TestRemoveSocketInsideWait()
        {
            Socket? socketInside = null;
            Socket? socketOutside = null;
            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);
            Socket? nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            if (socketInside != null && socketOutside != null)
            {
                m_poller.AddSocket(socketInside);
                m_poller.EnableRead(socketInside);

                Thread thread = new Thread(() =>
                {
                    Thread.Sleep(10);
                    m_poller.RemoveSocket(socketInside);
                    Thread.Sleep(10);
                    try
                    {
                        byte[] buffer = new byte[1000];
                        Platform.Instance.Receive(socketOutside, buffer, 0, buffer.Length);
                    }
                    catch (System.Net.Sockets.SocketException)
                    {
                    }
                });
                thread.Start();

                PollerResult result = m_poller.Wait(50);
                Assert.True(result.Timeout);
                Assert.Equal(0, result.DescriptorInfos.Count);

                thread.Join();

                socketInside.Dispose();
                socketOutside.Dispose();
            }
        }
    }
}
