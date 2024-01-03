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

    public class TestSerializerProtoStruct : IDisposable
    {
        ZeroCopyBuffer m_buffer = new ZeroCopyBuffer();
        int MAX_BLOCK_SIZE = 0;

        public TestSerializerProtoStruct()
        {
        }

        public void Dispose()
        {
        }

        [Fact]
        public void Test0Data()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            MAX_BLOCK_SIZE = 1024;
            SerializerProto serializer = new SerializerProto(m_buffer, MAX_BLOCK_SIZE);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStruct")!);
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, null, 0), VALUE_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, null, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", 0, null, 2), VALUE_UINT32);
            serializer.Finished();

            byte[] data = m_buffer.GetData();
            var root = Fmq.Test.TestStruct.Parser.ParseFrom(data, 0, data.Length);
            var cmp = new Fmq.Test.TestStruct
            {
                StructInt32 = new Fmq.Test.TestInt32 { Value = VALUE_INT32 },
                StructString = new Fmq.Test.TestString { Value = VALUE_STRING },
                LastValue = VALUE_UINT32
            };

            Debug.Assert(root.Equals(cmp));
        }

        void HelperTestStructSize(int size)
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            MAX_BLOCK_SIZE = size + 100;
            SerializerProto serializer = new SerializerProto(m_buffer, MAX_BLOCK_SIZE);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStruct")!);
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, null, 0), VALUE_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, null, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", 0, null, 2), VALUE_UINT32);
            serializer.Finished();

            byte[] data = m_buffer.GetData();
            var root = Fmq.Test.TestStruct.Parser.ParseFrom(data, 0, data.Length);
            var cmp = new Fmq.Test.TestStruct
            {
                StructInt32 = new Fmq.Test.TestInt32 { Value = VALUE_INT32 },
                StructString = new Fmq.Test.TestString { Value = VALUE_STRING },
                LastValue = VALUE_UINT32
            };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void Test32Data()
        {
            HelperTestStructSize(32);
        }

        [Fact]
        public void Test33Data()
        {
            HelperTestStructSize(33);
        }

        [Fact]
        public void Test120Data()
        {
            HelperTestStructSize(128 - 7 -1);
        }

        [Fact]
        public void Test121Data()
        {
            HelperTestStructSize(128 - 7);
        }

        [Fact]
        public void Test122Data()
        {
            HelperTestStructSize(128 - 7 + 1);
        }

        [Fact]
        public void Test16377Data()
        {
            HelperTestStructSize(16384 - 7 - 1);
        }

        [Fact]
        public void Test16378Data()
        {
            HelperTestStructSize(16384 - 7);
        }

        [Fact]
        public void Test16379Data()
        {
            HelperTestStructSize(16384 - 7 + 1);
        }

        [Fact]
        public void Test2097146Data()
        {
            HelperTestStructSize(2097152 - 7 - 1);
        }

        [Fact]
        public void Test2097147Data()
        {
            HelperTestStructSize(2097152 - 7);
        }

        [Fact]
        public void Test2097148Data()
        {
            HelperTestStructSize(2097152 - 7 + 1);
        }

        [Fact]
        public void Test268435451Data()
        {
            HelperTestStructSize(268435456 - 7 - 1);
        }

        [Fact]
        public void Test268435452Data()
        {
            HelperTestStructSize(268435456 - 7);
        }

        [Fact]
        public void Test268435453Data()
        {
            HelperTestStructSize(268435456 - 7 + 1);
        }

        [Fact]
        public void Test0DataBlockSize1()
        {
            int VALUE_INT32 = 0;
            string VALUE_STRING = "";
            uint VALUE_UINT32 = 123;

            MAX_BLOCK_SIZE = 1;
            SerializerProto serializer = new SerializerProto(m_buffer, MAX_BLOCK_SIZE);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStruct")!);
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, null, 0), VALUE_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, null, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", (int)MetaFieldFlags.METAFLAG_PROTO_VARINT, null, 2), VALUE_UINT32);
            serializer.Finished();

            byte[] data = m_buffer.GetData();
            var root = Fmq.Test.TestStructBlockSize.Parser.ParseFrom(data, 0, data.Length);
            var cmp = new Fmq.Test.TestStructBlockSize
            {
                StructInt32 = null,
                StructString = null,
                LastValue = VALUE_UINT32
            };

            Debug.Assert(root.Equals(cmp));
        }

        void HelperTestStructSizeBlockSize1(int size)
        {
            int VALUE_INT32 = 0;
            string VALUE_STRING = new string('a', size -2);
            uint VALUE_UINT32 = 123;

            MAX_BLOCK_SIZE = 1;
            SerializerProto serializer = new SerializerProto(m_buffer, MAX_BLOCK_SIZE);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStruct")!);
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, null, 0), VALUE_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, null, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", (int)MetaFieldFlags.METAFLAG_PROTO_VARINT, null, 2), VALUE_UINT32);
            serializer.Finished();

            byte[] data = m_buffer.GetData();

            //Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();
            //ParserProto parser = new ParserProto(mockVisitor.Object, data);
            //bool res = parser.ParseStruct("test.TestStructBlockSize");
            //Debug.Assert(res);

            var root = Fmq.Test.TestStructBlockSize.Parser.ParseFrom(data, 0, data.Length);
            var cmp = new Fmq.Test.TestStructBlockSize
            {
                StructInt32 = null,
                StructString = new Fmq.Test.TestString { Value = VALUE_STRING },
                LastValue = VALUE_UINT32
            };

            Debug.Assert(root.StructInt32 == cmp.StructInt32);
            Debug.Assert(root.StructString.Value == cmp.StructString.Value);
            Debug.Assert(root.LastValue == cmp.LastValue);
        }

        [Fact]
        public void Test32DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(32);
        }

        [Fact]
        public void Test33DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(33);
        }

        [Fact]
        public void Test120DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(128 - 7 - 1);
        }

        [Fact]
        public void Test121DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(128 - 7);
        }

        [Fact]
        public void Test122DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(128 - 7 + 1);
        }

        [Fact]
        public void Test16377DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(16384 - 7 - 1);
        }

        [Fact]
        public void Test16378DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(16384 - 7);
        }

        [Fact]
        public void Test16379DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(16384 - 7 + 1);
        }

        [Fact]
        public void Test2097146DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(2097152 - 7 - 1);
        }

        [Fact]
        public void Test2097147DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(2097152 - 7);
        }

        [Fact]
        public void Test2097148DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(2097152 - 7 + 1);
        }

        [Fact]
        public void Test268435451DataBlockSize1()
        {
            HelperTestStructSizeBlockSize1(268435456 - 7 - 1);
        }

        [Fact]
        public void Test268435452DataBlockSize1()
        {
            HelperTestStructSize(268435456 - 7);
        }

        [Fact]
        public void Test268435453DataBlockSize1()
        {
            HelperTestStructSize(268435456 - 7 + 1);
        }
    }
}

