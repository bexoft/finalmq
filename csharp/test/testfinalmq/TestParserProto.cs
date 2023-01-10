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
    using VariantStruct = List<NameValue>;
    using VariantList = List<Variant>;

    public class TestParserProto : IDisposable
    {
        MetaField? m_fieldValue = null;
        MetaField? m_fieldValue2 = null;
        MetaField? m_fieldValueInt32 = null;
        MetaField? m_fieldName = null;
        MetaField? m_fieldType = null;
        MetaField? m_fieldInt32 = null;
        MetaField? m_fieldString = null;
        MetaField? m_fieldList = null;
        MetaField? m_fieldListWithoutArray = null;

        public TestParserProto()
        {
            MetaStruct? structTestVariant = MetaDataGlobal.Instance.GetStruct("test.TestVariant");
            Debug.Assert(structTestVariant != null);
            m_fieldValue = structTestVariant.GetFieldByName("value");
            Debug.Assert(m_fieldValue != null);
            Debug.Assert(m_fieldValue.TypeName == "finalmq.variant.VarValue");
            m_fieldValue2 = structTestVariant.GetFieldByName("value2");
            Debug.Assert(m_fieldValue2 != null);
            Debug.Assert(m_fieldValue2.TypeName == "finalmq.variant.VarValue");
            m_fieldValueInt32 = structTestVariant.GetFieldByName("valueInt32");
            Debug.Assert(m_fieldValueInt32 != null);

            MetaStruct? structVarVariant = MetaDataGlobal.Instance.GetStruct("finalmq.variant.VarValue");
            Debug.Assert(structVarVariant != null);

            m_fieldName = structVarVariant.GetFieldByName("name");
            m_fieldType = structVarVariant.GetFieldByName("type");
            m_fieldInt32 = structVarVariant.GetFieldByName("valint32");
            m_fieldString = structVarVariant.GetFieldByName("valstring");
            m_fieldList = structVarVariant.GetFieldByName("vallist");
            m_fieldListWithoutArray = MetaDataGlobal.Instance.GetArrayField(m_fieldList!);

            Debug.Assert(m_fieldName != null);
            Debug.Assert(m_fieldType != null);
            Debug.Assert(m_fieldInt32 != null);
            Debug.Assert(m_fieldString != null);
            Debug.Assert(m_fieldList != null);
            Debug.Assert(m_fieldListWithoutArray != null);
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestUnknownStruct()
        {
            Fmq.Test.TestBool message = new Fmq.Test.TestBool();
            byte[] data = message.ToByteArray();

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();
            ParserProto parser = new ParserProto(mockVisitor.Object, data);
            bool res = parser.ParseStruct("test.BlaBla");
            Debug.Assert(!res);

            mockVisitor.Verify(x => x.NotifyError("", It.IsAny<string>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestBool()
        {
            bool VALUE = true;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestBool", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestBool{ Value = VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestBool");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterBool(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt32()
        {
            int VALUE = -2;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestInt32", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestInt32{ Value = VALUE};
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestInt32");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt32()
        {
            uint VALUE = 130;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestUInt32", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestUInt32{ Value = VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestUInt32");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterUInt32(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt64()
        {
            long VALUE = -2;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestInt64", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestInt64 { Value = VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestInt64");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterInt64(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt64()
        {
            ulong VALUE = 130;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestUInt64", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestUInt64 { Value = VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestUInt64");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterUInt64(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestFloat()
        {
            float VALUE = -1.1f;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestFloat", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestFloat { Value = VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestFloat");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterFloat(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestDouble()
        {
            double VALUE = -1.1;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestDouble", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestDouble { Value = VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestDouble");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterDouble(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestString()
        {
            string VALUE = "Hello World";

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestString", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            mockVisitor.Setup(x => x.EnterString(fieldValue, It.IsAny<byte[]>(), It.IsAny<int>(), It.IsAny<int>()))
                .Callback((MetaField field, byte[] buffer, int offset, int size) =>
                {
                    string v = Encoding.UTF8.GetString(buffer, offset, size);
                    Debug.Assert(v == VALUE);
                });

            var root = new Fmq.Test.TestString { Value = VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestString");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterString(fieldValue, It.IsAny<byte[]>(), It.IsAny<int>(), It.IsAny<int>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestBytes()
        {
            byte[] VALUE = { 0x23, 0xaa, 0x00, 0x6a, 0x40, 0x00 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestBytes", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            mockVisitor.Setup(x => x.EnterBytes(fieldValue, It.IsAny<byte[]>(), It.IsAny<int>(), It.IsAny<int>()))
                .Callback((MetaField field, byte[] buffer, int offset, int size) =>
                {
                    byte[] v = new byte[size];
                    System.Array.Copy(buffer, offset, v, 0, size);
                    Debug.Assert(v.SequenceEqual(VALUE));
                });

            var root = new Fmq.Test.TestBytes { Value = ByteString.CopyFrom(VALUE) };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestBytes");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterBytes(fieldValue, It.IsAny<byte[]>(), It.IsAny<int>(), It.IsAny<int>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStruct()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";

            MetaField? fieldStructInt32 = MetaDataGlobal.Instance.GetField("test.TestStruct", "struct_int32");
            MetaField? fieldStructString = MetaDataGlobal.Instance.GetField("test.TestStruct", "struct_string");
            MetaField? fieldInt32 = MetaDataGlobal.Instance.GetField("test.TestInt32", "value");
            MetaField? fieldString = MetaDataGlobal.Instance.GetField("test.TestString", "value");

            Debug.Assert(fieldStructInt32 != null);
            Debug.Assert(fieldStructString != null);
            Debug.Assert(fieldInt32 != null);
            Debug.Assert(fieldString != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            mockVisitor.Setup(x => x.EnterString(fieldString, It.IsAny<byte[]>(), It.IsAny<int>(), It.IsAny<int>()))
                .Callback((MetaField field, byte[] buffer, int offset, int size) =>
                {
                    string v = Encoding.UTF8.GetString(buffer, offset, size);
                    Debug.Assert(v == VALUE_STRING);
                });

            var root = new Fmq.Test.TestStruct { StructInt32 = new Fmq.Test.TestInt32 { Value = VALUE_INT32 },
                                                 StructString = new Fmq.Test.TestString { Value = VALUE_STRING },
                                                 LastValue = 0
                                               };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestStruct");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterStruct(fieldStructInt32), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(fieldInt32, VALUE_INT32), Times.Once);
            mockVisitor.Verify(x => x.ExitStruct(fieldStructInt32), Times.Once);
            mockVisitor.Verify(x => x.EnterStruct(fieldStructString), Times.Once);
            mockVisitor.Verify(x => x.EnterString(fieldString, It.IsAny<byte[]>(), It.IsAny<int>(), It.IsAny<int>()), Times.Once);
            mockVisitor.Verify(x => x.ExitStruct(fieldStructString), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEnumAsInt()
        {
            int VALUE = -2;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestEnum", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestEnum { Value = Fmq.Test.Foo.Hello };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestEnum");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterEnum(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEnumNotAvailableInt()
        {
            int VALUE = 42;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestEnum", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestEnum { Value = (Fmq.Test.Foo)VALUE };
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestEnum");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterEnum(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }


        [Fact]
        public void TestVariantEmpty()
        {
            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            var root = new Fmq.Test.TestVariant();
            ParserProto parser = new ParserProto(mockVisitor.Object, root.ToByteArray());
            bool res = parser.ParseStruct("test.TestVariant");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

    }
}

