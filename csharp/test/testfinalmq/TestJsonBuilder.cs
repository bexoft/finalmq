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

    public class TestJsonBuilder : IDisposable
    {
        static readonly int MAX_BLOCK_SIZE = 1024;

        IJsonParserVisitor m_builder;
        byte[] m_data = new byte[MAX_BLOCK_SIZE];
        int m_size = MAX_BLOCK_SIZE;
        BufferRef m_bufferRef;
        Mock<IZeroCopyBuffer>? m_mockBuffer = null;

        public TestJsonBuilder()
        {
            m_mockBuffer = new Mock<IZeroCopyBuffer>();
            m_bufferRef = new BufferRef(m_data, 0, m_data.Length);

            m_mockBuffer.Setup(x => x.AddBuffer(MAX_BLOCK_SIZE, It.IsAny<int>())).Returns(m_bufferRef);
            m_mockBuffer.Setup(x => x.DownsizeLastBuffer(It.IsAny<int>())).Callback((int size) => {
                m_size = size;
            });
            m_mockBuffer.Setup(x => x.RemainingSize).Returns(0);

            m_builder = new JsonBuilder(m_mockBuffer.Object, MAX_BLOCK_SIZE);
        }

        public void Dispose()
        {
            if (m_mockBuffer != null)
            {
                m_mockBuffer.Verify(x => x.AddBuffer(MAX_BLOCK_SIZE, It.IsAny<int>()), Times.Once);
                m_mockBuffer.Verify(x => x.DownsizeLastBuffer(It.IsAny<int>()), Times.Once);
                m_mockBuffer.Verify(x => x.RemainingSize, Times.Once);
            }
        }


        [Fact]
        public void TestNull()
        {
            m_builder.EnterNull();
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "null");
        }

        [Fact]
        public void TestTrue()
        {
            m_builder.EnterBool(true);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "true");
        }

        [Fact]
        public void TestFalse()
        {
            m_builder.EnterBool(false);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "false");
        }

        [Fact]
        public void TestInt32_0()
        {
            m_builder.EnterInt32(0);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "0");
        }

        [Fact]
        public void TestInt32_1234567890()
        {
            m_builder.EnterInt32(1234567890);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "1234567890");
        }

        [Fact]
        public void TestInt32_n1234567890()
        {
            m_builder.EnterInt32(-1234567890);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "-1234567890");
        }

        [Fact]
        public void TestUInt32_4234567890()
        {
            m_builder.EnterUInt32(4234567890);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "4234567890");
        }

        [Fact]
        public void TestInt64_0()
        {
            m_builder.EnterInt64(0);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "0");
        }

        [Fact]
        public void TestInt64_1234567890123456789()
        {
            m_builder.EnterInt64(1234567890123456789);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "1234567890123456789");
        }

        [Fact]
        public void TestInt64_n1234567890123456789()
        {
            m_builder.EnterInt64(-1234567890123456789);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "-1234567890123456789");
        }

        [Fact]
        public void TestUInt64_2234567890123456789()
        {
            m_builder.EnterUInt64(2234567890123456789);
            m_builder.Finished();
            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "2234567890123456789");
        }

        [Fact]
        public void TestDouble_22345678901234567()
        {
            m_builder.EnterDouble(-0.22345678901234567);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size - 1);
            Debug.Assert(s == "-0.2234567890123456");
        }

        [Fact]
        public void TestDouble_2234567en123()
        {
            m_builder.EnterDouble(2234567e-123);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "2.234567E-117");
        }

        [Fact]
        public void TestString()
        {
            m_builder.EnterString("Hello");
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"Hello\"");
        }

        [Fact]
        public void TestStringEmpty()
        {
            m_builder.EnterString("");
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\"");
        }

        [Fact]
        public void TestStringEscape()
        {
            m_builder.EnterString("\\\"\r\n\f\b\t\x01");
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\\\\\\"\\r\\n\\f\\b\\t\\u0001\"");
        }

        [Fact]
        public void TestStringEscapeU16()
        {
            m_builder.EnterString(Encoding.UTF8.GetString(new byte[] { 0xc3, 0xa4 }));
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\u00e4\"");
        }

        [Fact]
        public void TestStringEscapeU32()
        {
            m_builder.EnterString(Encoding.UTF8.GetString(new byte[] { 0xf0, 0x90, 0xa6, 0xaa }));
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\ud802\\uddaa\"");
        }

        [Fact]
        public void TestStringBuffer()
        {
            m_builder.EnterString(Encoding.UTF8.GetBytes("Hello"), 0, 5);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"Hello\"");
        }

        [Fact]
        public void TestStringBufferEmpty()
        {
            m_builder.EnterString(Encoding.UTF8.GetBytes(""), 0, 0);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\"");
        }

        [Fact]
        public void TestStringBufferEscape()
        {
            m_builder.EnterString(Encoding.UTF8.GetBytes("\\\"\r\n\f\b\t\x01"), 0, 8);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\\\\\\"\\r\\n\\f\\b\\t\\u0001\"");
        }

        [Fact]
        public void TestStringBufferEscapeU16()
        {
            m_builder.EnterString(new byte[] { 0xc3, 0xa4 }, 0, 2);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\u00e4\"");
        }

        [Fact]
        public void TestStringBufferEscapeU32()
        {
            m_builder.EnterString(new byte[] { 0xf0, 0x90, 0xa6, 0xaa }, 0, 4);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\ud802\\uddaa\"");
        }

        [Fact]
        public void TestArray()
        {
            m_builder.EnterArray();
            m_builder.ExitArray();
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "[]");
        }

        [Fact]
        public void TestObject()
        {
            m_builder.EnterObject();
            m_builder.ExitObject();
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{}");
        }
        
        [Fact]
        public void TestKey()
        {
            m_builder.EnterKey("Hello");
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"Hello\":");
        }

        [Fact]
        public void TestKeyEmpty()
        {
            m_builder.EnterKey("");
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\":");
        }

        [Fact]
        public void TestKeyEscape()
        {
            m_builder.EnterKey("\\\"\r\n\f\b\t\x01");
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\\\\\\"\\r\\n\\f\\b\\t\\u0001\":");
        }

        [Fact]
        public void TestKeyEscapeU16()
        {
            m_builder.EnterKey(Encoding.UTF8.GetString(new byte[] { 0xc3, 0xa4 }));
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\u00e4\":");
        }

        [Fact]
        public void TestKeyEscapeU32()
        {
            m_builder.EnterKey(Encoding.UTF8.GetString(new byte[] { 0xf0, 0x90, 0xa6, 0xaa }));
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\ud802\\uddaa\":");
        }

        [Fact]
        public void TestKeyBuffer()
        {
            m_builder.EnterKey(Encoding.UTF8.GetBytes("Hello"), 0, 5);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"Hello\":");
        }

        [Fact]
        public void TestKeyBufferEmpty()
        {
            m_builder.EnterKey(Encoding.UTF8.GetBytes(""), 0, 0);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\":");
        }

        [Fact]
        public void TestKeyBufferEscape()
        {
            m_builder.EnterKey(Encoding.UTF8.GetBytes("\\\"\r\n\f\b\t\x01"), 0, 8);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\\\\\\"\\r\\n\\f\\b\\t\\u0001\":");
        }

        [Fact]
        public void TestKeyBufferEscapeU16()
        {
            m_builder.EnterKey(new byte[] { 0xc3, 0xa4 }, 0, 2);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\u00e4\":");
        }

        [Fact]
        public void TestKeyBufferEscapeU32()
        {
            m_builder.EnterKey(new byte[] { 0xf0, 0x90, 0xa6, 0xaa }, 0, 4);
            m_builder.Finished();
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "\"\\ud802\\uddaa\":");
        }

        [Fact]
        public void TestObjectExample()
        {
            m_builder.EnterObject();
            m_builder.EnterKey("name");
            m_builder.EnterString("Elvis");
            m_builder.EnterKey("age");
            m_builder.EnterInt32(42);
            m_builder.EnterKey("arr");
            m_builder.EnterArray();
            m_builder.EnterDouble(1.234);
            m_builder.EnterDouble(2.345);
            m_builder.EnterDouble(3.456);
            m_builder.ExitArray();
            m_builder.ExitObject();
            m_builder.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"name\":\"Elvis\",\"age\":42,\"arr\":[1.234,2.345,3.456]}");
        }

        [Fact]
        public void TestWithRealZeroCopyBuffer()
        {
            m_mockBuffer = null;

            IMessage message = new ProtocolMessage(0);
            m_builder = new JsonBuilder(message, 8);

            m_builder.EnterObject();
            m_builder.EnterKey("name");
            m_builder.EnterString("This is Elvis");
            m_builder.EnterKey("age");
            m_builder.EnterInt32(42);
            m_builder.EnterKey("arr");
            m_builder.EnterArray();
            m_builder.EnterString("This is a loooooooooooooooooooooooooooooooooooooooooooong text");
            m_builder.EnterDouble(1.234);
            m_builder.EnterDouble(2.345);
            m_builder.EnterDouble(3.456);
            m_builder.ExitArray();
            m_builder.ExitObject();
            m_builder.Finished();

            string result = "";
            IList<BufferRef> buffers = message.GetAllSendBuffers();
            foreach (var buffer in buffers)
            {
                result += Encoding.UTF8.GetString(buffer.Buffer, buffer.Offset, buffer.Length);
            }
            Debug.Assert(result == "{\"name\":\"This is Elvis\",\"age\":42,\"arr\":[\"This is a loooooooooooooooooooooooooooooooooooooooooooong text\",1.234,2.345,3.456]}");
        }

        [Fact]
        public void TestWithRealZeroCopyBufferPreAllocateBuffer()
        {
            m_mockBuffer = null;

            IMessage message = new ProtocolMessage(0);
            BufferRef preBuffer = message.AddBuffer(40, 100);
            Encoding.UTF8.GetBytes("PRE").CopyTo(preBuffer.Buffer, preBuffer.Offset);
            message.DownsizeLastBuffer(3);

            m_builder = new JsonBuilder(message, 8);

            m_builder.EnterObject();
            m_builder.EnterKey("name");
            m_builder.EnterString("This is Elvis");
            m_builder.EnterKey("age");
            m_builder.EnterInt32(42);
            m_builder.EnterKey("arr");
            m_builder.EnterArray();
            m_builder.EnterString("This is a loooooooooooooooooooooooooooooooooooooooooooong text");
            m_builder.EnterDouble(1.234);
            m_builder.EnterDouble(2.345);
            m_builder.EnterDouble(3.456);
            m_builder.ExitArray();
            m_builder.ExitObject();
            m_builder.Finished();

            string result = "";
            IList<BufferRef> buffers = message.GetAllSendBuffers();
            foreach (var buffer in buffers)
            {
                result += Encoding.UTF8.GetString(buffer.Buffer, buffer.Offset, buffer.Length);
            }
            Debug.Assert(result == "PRE{\"name\":\"This is Elvis\",\"age\":42,\"arr\":[\"This is a loooooooooooooooooooooooooooooooooooooooooooong text\",1.234,2.345,3.456]}");
        }
    }
}

