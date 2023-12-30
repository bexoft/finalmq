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

        MetaField m_fieldValue;
        MetaField m_fieldValue2;
        MetaField m_fieldValueInt32;
        MetaField m_fieldName;
        MetaField m_fieldType;
        MetaField m_fieldInt32;
        MetaField m_fieldString;
        MetaField m_fieldList;
        MetaField m_fieldListWithoutArray;

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

            MetaStruct? structTestVariant = MetaDataGlobal.Instance.GetStruct("test.TestVariant");
            Debug.Assert(structTestVariant != null);
            m_fieldValue = structTestVariant.GetFieldByName("value")!;
            Debug.Assert(m_fieldValue != null);
            Debug.Assert(m_fieldValue.TypeName == "finalmq.variant.VarValue");
            m_fieldValue2 = structTestVariant.GetFieldByName("value2")!;
            Debug.Assert(m_fieldValue2 != null);
            Debug.Assert(m_fieldValue2.TypeName == "finalmq.variant.VarValue");
            m_fieldValueInt32 = structTestVariant.GetFieldByName("valueInt32")!;
            Debug.Assert(m_fieldValueInt32 != null);

            MetaStruct? structVarVariant = MetaDataGlobal.Instance.GetStruct("finalmq.variant.VarValue");
            Debug.Assert(structVarVariant != null);

            m_fieldName = structVarVariant.GetFieldByName("name")!;
            m_fieldType = structVarVariant.GetFieldByName("type")!;
            m_fieldInt32 = structVarVariant.GetFieldByName("valint32")!;
            m_fieldString = structVarVariant.GetFieldByName("valstring")!;
            m_fieldList = structVarVariant.GetFieldByName("vallist")!;
            m_fieldListWithoutArray = MetaDataGlobal.Instance.GetArrayField(m_fieldList!)!;
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
        public void TestInt8()
        {
            sbyte VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestInt8");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterInt8(new MetaField(MetaTypeId.TYPE_INT8, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestInt8.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestInt8 { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt8()
        {
            byte VALUE = 0xFE;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestUInt8");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterUInt8(new MetaField(MetaTypeId.TYPE_UINT8, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestUInt8.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestUInt8 { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt16()
        {
            short VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestInt16");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterInt16(new MetaField(MetaTypeId.TYPE_INT16, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestInt16.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestInt16 { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt16()
        {
            ushort VALUE = 0xFFFE;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestUInt16");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterUInt16(new MetaField(MetaTypeId.TYPE_UINT16, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestUInt16.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestUInt16 { Value = VALUE };

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

        [Fact]
        public void TestInt64()
        {
            long VALUE = -2;

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestInt64")!);
            m_serializer.EnterInt64(new MetaField(MetaTypeId.TYPE_INT64, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestInt64.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestInt64 { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt64()
        {
            ulong VALUE = 0xFFFFFFFFFFFFFFFE;

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestUInt64")!);
            m_serializer.EnterUInt64(new MetaField(MetaTypeId.TYPE_UINT64, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestUInt64.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestUInt64 { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestFloat()
        {
            float VALUE = -2.1f;

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestFloat")!);
            m_serializer.EnterFloat(new MetaField(MetaTypeId.TYPE_FLOAT, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestFloat.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestFloat { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestDouble()
        {
            double VALUE = -2.1;

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestDouble")!);
            m_serializer.EnterDouble(new MetaField(MetaTypeId.TYPE_DOUBLE, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestDouble.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestDouble { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestString()
        {
            string VALUE = "Hello World";

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestString")!);
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestString.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestString { Value = VALUE };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestBytes()
        {
            byte[] VALUE = { (byte)'H', (byte)'e', (byte)'l', 0, 13, (byte)'l', (byte)'o' };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestString")!);
            m_serializer.EnterBytes(new MetaField(MetaTypeId.TYPE_BYTES, "", "value", "", 0, 0), VALUE, 0, VALUE.Length);
            m_serializer.Finished();

            var root = Fmq.Test.TestBytes.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestBytes { Value = ByteString.CopyFrom(VALUE) };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestStruct()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStruct")!);
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0));
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, 0), VALUE_INT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, 0), VALUE_STRING);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", 0, 2), VALUE_UINT32);
            m_serializer.Finished();

            var root = Fmq.Test.TestStruct.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestStruct
            {
                StructInt32 = new Fmq.Test.TestInt32 { Value = VALUE_INT32 },
                StructString = new Fmq.Test.TestString { Value = VALUE_STRING },
                LastValue = VALUE_UINT32
            };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestEnum()
        {
            test.Foo VALUE = test.Foo.FOO_HELLO;

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestEnum")!);
            m_serializer.EnterEnum(new MetaField(MetaTypeId.TYPE_ENUM, "test.Foo", "value", "", 0, 0), (int)VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestEnum.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestEnum { Value = Fmq.Test.Foo.Hello };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantEmpty()
        {
            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            m_serializer.EnterStruct(m_fieldValue);
            m_serializer.EnterEnum(m_fieldType, (int)finalmq.variant.VarTypeId.T_NONE);
            m_serializer.ExitStruct(m_fieldValue);
            m_serializer.Finished();

            var root = Fmq.Test.TestVariant.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestVariant {  };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantInt32()
        {
            int VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestVariant");
            Debug.Assert(stru != null);
            MetaStruct? struValue = MetaDataGlobal.Instance.GetStruct(stru.GetFieldByName("value")!.TypeName);
            m_serializer.StartStruct(stru);
            m_serializer.EnterStruct(stru.GetFieldByName("value")!);
            m_serializer.EnterEnum(struValue!.GetFieldByName("type")!, "int32");
            m_serializer.EnterInt32(struValue!.GetFieldByName("valint32")!, VALUE);
            m_serializer.ExitStruct(stru.GetFieldByName("value")!);

            m_serializer.EnterStruct(stru.GetFieldByName("value2")!);
            m_serializer.ExitStruct(stru.GetFieldByName("value2")!);
            m_serializer.Finished();

            var root = Fmq.Test.TestVariant.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestVariant
            {
                Value = new Fmq.Finalmq.Variant.VarValue { Type = Fmq.Finalmq.Variant.VarTypeId.TInt32, Valint32 = VALUE },
                ValueInt32 = 0,
                Value2 = null
            };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantStruct()
        {
            MetaStruct structTestVariant = MetaDataGlobal.Instance.GetStruct("test.TestVariant")!;
            MetaField fieldValue = structTestVariant.GetFieldByName("value")!;
            MetaField fieldValue2 = structTestVariant.GetFieldByName("value2")!;
            MetaField fieldValueInt32 = structTestVariant.GetFieldByName("valueInt32")!;

            MetaStruct structVarVariant = MetaDataGlobal.Instance.GetStruct("finalmq.variant.VarValue")!;

            MetaField fieldName = structVarVariant.GetFieldByName("name")!;
            MetaField fieldType = structVarVariant.GetFieldByName("type")!;
            MetaField fieldInt32 = structVarVariant.GetFieldByName("valint32")!;
            MetaField fieldString = structVarVariant.GetFieldByName("valstring")!;
            MetaField fieldList = structVarVariant.GetFieldByName("vallist")!;
            MetaField fieldListWithoutArray = MetaDataGlobal.Instance.GetArrayField(fieldList)!;

            // VariantStruct{ {"value", VariantStruct{
            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            m_serializer.EnterStruct(fieldValue);
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            m_serializer.EnterArrayStruct(fieldList);
            // {"key1", VariantList{
            m_serializer.EnterStruct(fieldListWithoutArray);
            m_serializer.EnterString(fieldName, "key1");
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_LIST);
            m_serializer.EnterArrayStruct(fieldList);
            // 2
            m_serializer.EnterStruct(fieldListWithoutArray);
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            m_serializer.EnterInt32(fieldInt32, 2);
            m_serializer.ExitStruct(fieldListWithoutArray);
            // , std::string("Hello")
            m_serializer.EnterStruct(fieldListWithoutArray);
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            m_serializer.EnterString(fieldString, "Hello");
            m_serializer.ExitStruct(fieldListWithoutArray);
            // }
            m_serializer.ExitArrayStruct(fieldList);
            m_serializer.ExitStruct(fieldListWithoutArray);

            // {"key2", VariantStruct{
            m_serializer.EnterStruct(fieldListWithoutArray);
            m_serializer.EnterString(fieldName, "key2");
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            m_serializer.EnterArrayStruct(fieldList);
            // {"a", 3},
            m_serializer.EnterStruct(fieldListWithoutArray);
            m_serializer.EnterString(fieldName, "a");
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            m_serializer.EnterInt32(fieldInt32, 3);
            m_serializer.ExitStruct(fieldListWithoutArray);
            // {"b", std::string("Hi")}
            m_serializer.EnterStruct(fieldListWithoutArray);
            m_serializer.EnterString(fieldName, "b");
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            m_serializer.EnterString(fieldString, "Hi");
            m_serializer.ExitStruct(fieldListWithoutArray);
            // }
            m_serializer.ExitArrayStruct(fieldList);
            m_serializer.ExitStruct(fieldListWithoutArray);

            // {
            m_serializer.EnterStruct(fieldListWithoutArray);
            m_serializer.EnterString(fieldName, "key3");
            m_serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_NONE);
            m_serializer.ExitStruct(fieldListWithoutArray);
            // }}
            m_serializer.ExitArrayStruct(fieldList);
            m_serializer.ExitStruct(fieldValue);
            m_serializer.Finished();

            var root = Fmq.Test.TestVariant.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestVariant
            {
                Value = new Fmq.Finalmq.Variant.VarValue
                {
                    Type = Fmq.Finalmq.Variant.VarTypeId.TStruct,
                    Vallist = {
                                                                new Fmq.Finalmq.Variant.VarValue {Name = "key1", Type = Fmq.Finalmq.Variant.VarTypeId.TList,
                                                                    Vallist = { new Fmq.Finalmq.Variant.VarValue {Type = Fmq.Finalmq.Variant.VarTypeId.TInt32, Valint32 = 2},
                                                                                new Fmq.Finalmq.Variant.VarValue {Type = Fmq.Finalmq.Variant.VarTypeId.TString, Valstring = "Hello" } } },
                                                                new Fmq.Finalmq.Variant.VarValue {Name = "key2", Type = Fmq.Finalmq.Variant.VarTypeId.TStruct,
                                                                    Vallist = { new Fmq.Finalmq.Variant.VarValue {Name = "a", Type = Fmq.Finalmq.Variant.VarTypeId.TInt32, Valint32 = 3},
                                                                                new Fmq.Finalmq.Variant.VarValue {Name = "b", Type = Fmq.Finalmq.Variant.VarTypeId.TString, Valstring = "Hi"    } } },
                                                                new Fmq.Finalmq.Variant.VarValue {Name = "key3", Type = Fmq.Finalmq.Variant.VarTypeId.TNone }
                                                    }
                },
                ValueInt32 = 0,
                Value2 = null
            };

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayBool()
        {
            bool[] VALUE = { true, false, true };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayBool")!);
            m_serializer.EnterArrayBool(new MetaField(MetaTypeId.TYPE_ARRAY_BOOL, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayBool.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayBool();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt8()
        {
            sbyte[] VALUE = { -2, 0, 2, 22 };
            int[] VALUE32 = { -2, 0, 2, 22 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt8")!);
            m_serializer.EnterArrayInt8(new MetaField(MetaTypeId.TYPE_ARRAY_INT8, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayInt8.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayInt8();
            cmp.Value.AddRange(VALUE32);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt16()
        {
            short[] VALUE = { -2, 0, 2, 222 };
            int[] VALUE32 = { -2, 0, 2, 222 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt32")!);
            m_serializer.EnterArrayInt16(new MetaField(MetaTypeId.TYPE_ARRAY_INT16, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayInt16.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayInt16();
            cmp.Value.AddRange(VALUE32);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt16()
        {
            ushort[] VALUE = { 0xFFFE, 0, 2, 222 };
            uint[] VALUE32 = { 0xFFFE, 0, 2, 222 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt16")!);
            m_serializer.EnterArrayUInt16(new MetaField(MetaTypeId.TYPE_ARRAY_UINT16, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayUInt16.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayUInt16();
            cmp.Value.AddRange(VALUE32);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt32()
        {
            int[] VALUE = { -2, 0, 2, 222 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt32")!);
            m_serializer.EnterArrayInt32(new MetaField(MetaTypeId.TYPE_ARRAY_INT32, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayInt32.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayInt32();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt32()
        {
            uint[] VALUE = { 0xFFFFFFFE, 0, 2, 222 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt32")!);
            m_serializer.EnterArrayUInt32(new MetaField(MetaTypeId.TYPE_ARRAY_UINT32, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayUInt32.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayUInt32();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt64()
        {
            long[] VALUE = { -2, 0, 2, 222 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt64")!);
            m_serializer.EnterArrayInt64(new MetaField(MetaTypeId.TYPE_ARRAY_INT64, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayInt64.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayInt64();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt64()
        {
            ulong[] VALUE = { 0xFFFFFFFFFFFFFFF, 0, 2, 222 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt32")!);
            m_serializer.EnterArrayUInt64(new MetaField(MetaTypeId.TYPE_ARRAY_UINT64, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayUInt64.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayUInt64();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayFloat()
        {
            float[] VALUE = { -2.1f, 0f, 2.1f, 222.1f, Single.NaN, Single.PositiveInfinity, Single.NegativeInfinity };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayFloat")!);
            m_serializer.EnterArrayFloat(new MetaField(MetaTypeId.TYPE_ARRAY_FLOAT, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayFloat.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayFloat();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayDouble()
        {
            double[] VALUE = { -2.1, 0, 2.1, 222.1, Double.NaN, Double.PositiveInfinity, Double.NegativeInfinity };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayDouble")!);
            m_serializer.EnterArrayDouble(new MetaField(MetaTypeId.TYPE_ARRAY_DOUBLE, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayDouble.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayDouble();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayString()
        {
            IList<string> VALUE = new List<string> { "Hello", "", "World" };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayString")!);
            m_serializer.EnterArrayString(new MetaField(MetaTypeId.TYPE_ARRAY_STRING, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayString.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayString();
            cmp.Value.AddRange(VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayBytes()
        {
            IList<byte[]> VALUE = new List<byte[]>();
            VALUE.Add(new byte[] { 0x23, 0xaa, 0x00, 0x6a, 0x40, 0x00 });
            VALUE.Add(new byte[] { });
            VALUE.Add(new byte[] { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff });
            VALUE.Add(new byte[] { 0x00 });

            IList<ByteString> VALUE_BYTESTRING = new List<ByteString>();
            foreach (var v in VALUE)
            {
                VALUE_BYTESTRING.Add(ByteString.CopyFrom(v));
            }

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayBytes")!);
            m_serializer.EnterArrayBytes(new MetaField(MetaTypeId.TYPE_ARRAY_BYTES, "", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayBytes.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayBytes();
            cmp.Value.AddRange(VALUE_BYTESTRING);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayStruct()
        {
            int VALUE1_INT32 = -2;
            string VALUE1_STRING = "Hello World";
            uint VALUE1_UINT32 = 123;
            int VALUE2_INT32 = 345;
            string VALUE2_STRING = "foo";
            uint VALUE2_UINT32 = 12345678;
            uint LAST_VALUE = 5;

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayStruct")!);
            m_serializer.EnterArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc"));

            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0, 0), VALUE1_INT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0, 0), VALUE1_STRING);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 0, 2), VALUE1_UINT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));

            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0, 0), VALUE2_INT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0, 0), VALUE2_STRING);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 0, 2), VALUE2_UINT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));

            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));

            m_serializer.ExitArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, 0));

            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 0, 1), LAST_VALUE);

            m_serializer.Finished();

            var root = Fmq.Test.TestArrayStruct.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayStruct { LastValue = LAST_VALUE };
            cmp.Value.Add(new Fmq.Test.TestStruct { StructInt32 = new Fmq.Test.TestInt32 { Value = VALUE1_INT32 }, StructString = new Fmq.Test.TestString { Value = VALUE1_STRING }, LastValue = VALUE1_UINT32 });
            cmp.Value.Add(new Fmq.Test.TestStruct { StructInt32 = new Fmq.Test.TestInt32 { Value = VALUE2_INT32 }, StructString = new Fmq.Test.TestString { Value = VALUE2_STRING }, LastValue = VALUE2_UINT32 });
            cmp.Value.Add(new Fmq.Test.TestStruct());

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayEnum()
        {
            int[] VALUE = { (int)test.Foo.FOO_HELLO, (int)test.Foo.FOO_WORLD, (int)test.Foo.FOO_WORLD2, 123 };
            IList<Fmq.Test.Foo> VALUE_CMP = new List<Fmq.Test.Foo> { Fmq.Test.Foo.Hello, Fmq.Test.Foo.World, Fmq.Test.Foo.World2, (Fmq.Test.Foo)123 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayEnum")!);
            m_serializer.EnterArrayEnum(new MetaField(MetaTypeId.TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, 0), VALUE);
            m_serializer.Finished();

            var root = Fmq.Test.TestArrayEnum.Parser.ParseFrom(m_data, 0, m_size);
            var cmp = new Fmq.Test.TestArrayEnum();
            cmp.Value.AddRange(VALUE_CMP);

            Debug.Assert(root.Equals(cmp));
        }

    }
}

