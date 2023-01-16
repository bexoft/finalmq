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
using System.Reflection;
using System.Reflection.Metadata;

using Google.Protobuf;


namespace testfinalmq
{

    public class TestSerializerProto : IDisposable
    {
        static readonly int MAX_BLOCK_SIZE = 1024;

        SerializerProto m_serializer;
        byte[] m_data = new byte[MAX_BLOCK_SIZE];
        int m_size = MAX_BLOCK_SIZE;
        BufferRef m_bufferRef;
        Mock<IZeroCopyBuffer>? m_mockBuffer = null;

        public TestSerializerProto()
        {
            m_mockBuffer = new Mock<IZeroCopyBuffer>();
            m_bufferRef = new BufferRef(m_data, 0, m_data.Length);

            m_mockBuffer.Setup(x => x.AddBuffer(MAX_BLOCK_SIZE, It.IsAny<int>())).Returns(m_bufferRef);
            m_mockBuffer.Setup(x => x.DownsizeLastBuffer(It.IsAny<int>())).Callback((int size) => {
                m_size = size;
            });
            m_mockBuffer.Setup(x => x.RemainingSize).Returns(0);

            m_serializer = new SerializerProto(m_mockBuffer.Object, MAX_BLOCK_SIZE);
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
        public void TestBool()
        {
            bool VALUE = true;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestBool");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterBool(new MetaField(MetaTypeId.TYPE_BOOL, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestBool.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestBool { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt32()
        {
            int VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestInt32");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestInt32.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestInt32 { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt32()
        {
            uint VALUE = 0xFFFFFFFE;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestUInt32");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestUInt32.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestUInt32 { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }


    }
}

