using System;
using System.Runtime;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;

using Xunit;
using Moq;

using finalmq;
using System.Text;

namespace testfinalmq
{
    using VariantStruct = List<NameValue>;

    public class TestProtocolHttpServer : IDisposable
    {
        IProtocol m_protocol = new ProtocolHttpServer();
        Mock<IProtocolCallback> m_mockCallback = new Mock<IProtocolCallback>();

        public TestProtocolHttpServer()
        {
            m_protocol.SetCallback(m_mockCallback.Object);
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestReceiveFirstLineIncomplete()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            byte[] receiveBuffer = Encoding.ASCII.GetBytes("GET /hello HTTP/1.1\r");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), It.IsAny<int>()), Times.Never);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

        [Fact]
        public void TestReceiveFirstLineComplete()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            byte[] receiveBuffer = Encoding.ASCII.GetBytes("GET /hello HTTP/1.1\r");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            receiveBuffer = new byte[] { (byte)'r' };
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), It.IsAny<int>()), Times.Never);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

        [Fact]
        public void TestReceiveHeaders()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            byte[] receiveBuffer = Encoding.ASCII.GetBytes("GET /hello?filter=world&lang=en HTTP/1.1\r\nhello: ");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            receiveBuffer = Encoding.ASCII.GetBytes("123\r\n");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            receiveBuffer = Encoding.ASCII.GetBytes("\r\n");
            m_protocol.Connection = mockConnection.Object;
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            m_mockCallback.Verify(x => x.SetSessionName(It.IsAny<string>(), It.IsAny<IProtocol>(), mockConnection.Object), Times.Once);
            m_mockCallback.Verify(x => x.Received(It.Is<IMessage>(p => p.AllMetainfo[ProtocolHttpServer.FMQ_HTTP] == "request" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_METHOD] == "GET" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_PATH] == "/hello" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_QUERY_PREFIX + "filter"] == "world" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_QUERY_PREFIX + "lang"] == "en" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_PROTOCOL] == "HTTP/1.1" &&
                                                                       p.AllMetainfo["hello"] == "123"
                                                                       ), It.IsAny<long>()), Times.Once);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

        [Fact]
        public void TestReceivePayload()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            m_protocol.Connection = mockConnection.Object;
            byte[] receiveBuffer = Encoding.ASCII.GetBytes("GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456789");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            m_mockCallback.Verify(x => x.SetSessionName(It.IsAny<string>(), It.IsAny<IProtocol>(), mockConnection.Object), Times.Once);
            m_mockCallback.Verify(x => x.Received(It.Is<IMessage>(p => p.AllMetainfo[ProtocolHttpServer.FMQ_HTTP] == "request" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_METHOD] == "GET" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_PATH] == "/hello" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_QUERY_PREFIX + "filter"] == "world" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_QUERY_PREFIX + "lang"] == "en" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_PROTOCOL] == "HTTP/1.1" &&
                                                                       p.AllMetainfo["Content-Length"] == "10" &&
                                                                       p.GetReceivePayload().Buffer.SequenceEqual(Encoding.ASCII.GetBytes("0123456789"))
                                                                       ), It.IsAny<long>()), Times.Once);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

        [Fact]
        public void TestReceiveSplitPayload()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            m_protocol.Connection = mockConnection.Object;
            byte[] receiveBuffer = Encoding.ASCII.GetBytes("GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);

            receiveBuffer = Encoding.ASCII.GetBytes("789");
            bool ok = m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            Assert.True(ok);

            m_mockCallback.Verify(x => x.SetSessionName(It.IsAny<string>(), It.IsAny<IProtocol>(), mockConnection.Object), Times.Once);
            m_mockCallback.Verify(x => x.Received(It.Is<IMessage>(p => p.AllMetainfo[ProtocolHttpServer.FMQ_HTTP] == "request" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_METHOD] == "GET" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_PATH] == "/hello" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_QUERY_PREFIX + "filter"] == "world" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_QUERY_PREFIX + "lang"] == "en" &&
                                                                       p.AllMetainfo[ProtocolHttpServer.FMQ_PROTOCOL] == "HTTP/1.1" &&
                                                                       p.AllMetainfo["Content-Length"] == "10" &&
                                                                       p.GetReceivePayload().Buffer.SequenceEqual(Encoding.ASCII.GetBytes("0123456789"))
                                                                       ), It.IsAny<long>()), Times.Once);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

        [Fact]
        public void TestReceivePayloadTooBig()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            m_protocol.Connection = mockConnection.Object;
            byte[] receiveBuffer = Encoding.ASCII.GetBytes("GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n01234567890");
            bool ok = m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            Assert.False(ok);

            m_mockCallback.Verify(x => x.SetSessionName(It.IsAny<string>(), It.IsAny<IProtocol>(), mockConnection.Object), Times.Never);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), It.IsAny<long>()), Times.Never);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

        [Fact]
        public void TestReceiveSplitPayloadTooBig()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            m_protocol.Connection = mockConnection.Object;
            byte[] receiveBuffer = Encoding.ASCII.GetBytes("GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456");
            bool ok = m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            Assert.True(ok);

            receiveBuffer = Encoding.ASCII.GetBytes("7890");
            ok = m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            Assert.False(ok);

            m_mockCallback.Verify(x => x.SetSessionName(It.IsAny<string>(), It.IsAny<IProtocol>(), mockConnection.Object), Times.Never);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), It.IsAny<long>()), Times.Never);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

        [Fact]
        public void TestSendMessage()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();

            IMessage message = new ProtocolMessage(0);
            Variant controlData = message.ControlData;
            controlData.Add(ProtocolHttpServer.FMQ_HTTP, "request");
            controlData.Add(ProtocolHttpServer.FMQ_METHOD, "GET");
            controlData.Add(ProtocolHttpServer.FMQ_PATH, "/hello");
            controlData.Add("queries", new VariantStruct{ new NameValue("filter", Variant.Create("world")), new NameValue("lang", Variant.Create("en")) });

            message.AddSendPayload(Encoding.ASCII.GetBytes("0123456789"));

            m_protocol.Connection = mockConnection.Object;

            m_protocol.SendMessage(message);

            mockConnection.Verify(x => x.SendMessage(message), Times.Once);

            m_mockCallback.Verify(x => x.SetSessionName(It.IsAny<string>(), It.IsAny<IProtocol>(), mockConnection.Object), Times.Never);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), It.IsAny<long>()), Times.Never);
            m_mockCallback.Verify(x => x.Disconnected(), Times.Never);
        }

    }
}
