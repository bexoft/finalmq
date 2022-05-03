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
    public class TestIntegrationSelect : IDisposable
    {
//        static int MILLITOMICRO = 1000;
//        static int TIMEOUT = 10;
        static byte[] BUFFER = new byte[20];


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
            Socket socketInside;
            Socket socketOutside;

            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);

            Socket nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            m_poller.AddSocket(socketInside);
            m_poller.EnableRead(socketInside);
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

        [Fact]
        public void TestEnableWriteSocketBeforeWait()
        {
            Socket socketInside;
            Socket socketOutside;

            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);

            Socket nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

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

        [Fact]
        public void TestEnableWriteSocketInsideWait()
        {
            Socket socketInside;
            Socket socketOutside;
            Platform.Instance.MakeSocketPair(out socketInside, out socketOutside);
            Socket nullSocket = null;
            Assert.NotEqual(nullSocket, socketInside);
            Assert.NotEqual(nullSocket, socketOutside);

            m_poller.AddSocket(socketInside);
            m_poller.EnableRead(socketInside);
            PollerResult result = m_poller.Wait(0);
            Assert.True(result.Timeout);
            Assert.Equal(0, result.DescriptorInfos.Count);

            m_poller.EnableWrite(socketInside);

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
}
