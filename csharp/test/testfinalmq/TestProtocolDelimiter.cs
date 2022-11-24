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
    class ProtocolDelimiterTest : ProtocolDelimiter
    {
        public static readonly uint PROTOCOL_ID = 12345;

        public static readonly byte[] DELIMITER = Encoding.UTF8.GetBytes("lol");

        public ProtocolDelimiterTest()
            : base(DELIMITER)
        {
        }

        // IProtocol
        public override uint ProtocolId
        {
            get
            {
                return PROTOCOL_ID;
            }
        }
    };

    public class TestProtocolDelimiter : IDisposable
    {
        IProtocol m_protocol = new ProtocolDelimiterTest();
        Mock<IProtocolCallback> m_mockCallback = new Mock<IProtocolCallback>();

        public TestProtocolDelimiter()
        {
            m_protocol.SetCallback(m_mockCallback.Object);
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestReceiveByteByByte()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();
            byte[] receiveBuffer = Encoding.UTF8.GetBytes("H");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            receiveBuffer = Encoding.UTF8.GetBytes("l");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            receiveBuffer = Encoding.UTF8.GetBytes("o");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            m_mockCallback.Setup(x => x.Received(It.IsAny<IMessage>(), 0))
               .Callback((IMessage message, long connectionId) =>
               {
                   BufferRef bufferRef = message.GetReceivePayload();
                   string result = Encoding.UTF8.GetString(bufferRef.Buffer, bufferRef.Offset, bufferRef.Length);
                   Debug.Assert(result == "H");
               });
            receiveBuffer = Encoding.UTF8.GetBytes("l");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Once);
            m_mockCallback.Reset();

            receiveBuffer = Encoding.UTF8.GetBytes("Ha");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            receiveBuffer = Encoding.UTF8.GetBytes("l");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            receiveBuffer = Encoding.UTF8.GetBytes("o");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            m_mockCallback.Setup(x => x.Received(It.IsAny<IMessage>(), 0))
               .Callback((IMessage message, long connectionId) =>
               {
                   BufferRef bufferRef = message.GetReceivePayload();
                   string result = Encoding.UTF8.GetString(bufferRef.Buffer, bufferRef.Offset, bufferRef.Length);
                   Debug.Assert(result == "Ha");
               });
            receiveBuffer = Encoding.UTF8.GetBytes("l");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Once);
        }

        [Fact]
        public void TestReceive()
        {
            Mock<IStreamConnection> mockConnection = new Mock<IStreamConnection>();
            m_mockCallback.Setup(x => x.Received(It.IsAny<IMessage>(), 0))
               .Callback((IMessage message, long connectionId) =>
               {
                   BufferRef bufferRef = message.GetReceivePayload();
                   string result = Encoding.UTF8.GetString(bufferRef.Buffer, bufferRef.Offset, bufferRef.Length);
                   Debug.Assert(result == "A");
               });
            byte[] receiveBuffer = Encoding.UTF8.GetBytes("AlolB");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Once);
            m_mockCallback.Reset();

            receiveBuffer = Encoding.UTF8.GetBytes("CD");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            receiveBuffer = Encoding.UTF8.GetBytes("EF");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            receiveBuffer = Encoding.UTF8.GetBytes("Gl");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Never);

            m_mockCallback.Setup(x => x.Received(It.IsAny<IMessage>(), 0))
               .Callback((IMessage message, long connectionId) =>
               {
                   BufferRef bufferRef = message.GetReceivePayload();
                   string result = Encoding.UTF8.GetString(bufferRef.Buffer, bufferRef.Offset, bufferRef.Length);
                   Debug.Assert(result == "BCDEFG");
               });
            receiveBuffer = Encoding.UTF8.GetBytes("ol");
            m_protocol.Received(mockConnection.Object, receiveBuffer, receiveBuffer.Length);
            m_mockCallback.Verify(x => x.Received(It.IsAny<IMessage>(), 0), Times.Once);
        }
    }
}
