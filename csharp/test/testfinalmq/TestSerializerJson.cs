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


namespace testfinalmq
{

    public class TestSerializerJson : IDisposable
    {
        static readonly int MAX_BLOCK_SIZE = 1024;

        SerializerJson m_serializer;
        SerializerJson m_serializerDefault;
        SerializerJson m_serializerEnumAsInt;
        byte[] m_data = new byte[MAX_BLOCK_SIZE];
        int m_size = MAX_BLOCK_SIZE;
        BufferRef m_bufferRef;
        Mock<IZeroCopyBuffer>? m_mockBuffer = null;

        public TestSerializerJson()
        {
            m_mockBuffer = new Mock<IZeroCopyBuffer>();
            m_bufferRef = new BufferRef(m_data, 0, m_data.Length);

            m_mockBuffer.Setup(x => x.AddBuffer(MAX_BLOCK_SIZE, It.IsAny<int>())).Returns(m_bufferRef);
            m_mockBuffer.Setup(x => x.DownsizeLastBuffer(It.IsAny<int>())).Callback((int size) => {
                m_size = size;
            });
            m_mockBuffer.Setup(x => x.RemainingSize).Returns(0);

            m_serializer = new SerializerJson(m_mockBuffer.Object, MAX_BLOCK_SIZE);
            m_serializerDefault = new SerializerJson(m_mockBuffer.Object, MAX_BLOCK_SIZE, true, false);
            m_serializerEnumAsInt = new SerializerJson(m_mockBuffer.Object, MAX_BLOCK_SIZE, false);
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
            m_serializer.EnterBool(new MetaField(MetaTypeId.TYPE_BOOL, "", "value", ""), VALUE);
            m_serializer.Finished();

            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "{\"value\":true}");
        }

        [Fact]
        public void TestInt32()
        {
            int VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestInt32");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", ""), VALUE);
            m_serializer.Finished();

            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "{\"value\":-2}");
        }

        [Fact]
        public void TestUInt32()
        {
            uint VALUE = 0xFFFFFFFE;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestUInt32");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "value", ""), VALUE);
            m_serializer.Finished();

            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "{\"value\":4294967294}");
        }

        [Fact]
        public void TestInt64()
        {
            int VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestInt64");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterInt64(new MetaField(MetaTypeId.TYPE_INT64, "", "value", ""), VALUE);
            m_serializer.Finished();

            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "{\"value\":\"-2\"}");
        }

        [Fact]
        public void TestUInt64()
        {
            ulong VALUE = 0xFFFFFFFFFFFFFFFE;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestUInt64");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterUInt64(new MetaField(MetaTypeId.TYPE_UINT64, "", "value", ""), VALUE);
            m_serializer.Finished();

            Debug.Assert(Encoding.UTF8.GetString(m_data, 0, m_size) == "{\"value\":\"18446744073709551614\"}");
        }

        [Fact]
        public void TestFloat()
        {
            float VALUE = -2f;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestFloat");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterFloat(new MetaField(MetaTypeId.TYPE_FLOAT, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":-2}");
        }

        [Fact]
        public void TestFloatNaN()
        {
            float VALUE = Single.NaN;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestFloat");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterFloat(new MetaField(MetaTypeId.TYPE_FLOAT, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"NaN\"}");
        }

        [Fact]
        public void TestFloatInfinity()
        {
            float VALUE = Single.PositiveInfinity;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestFloat");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterFloat(new MetaField(MetaTypeId.TYPE_FLOAT, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"Infinity\"}");
        }

        [Fact]
        public void TestFloatNInfinity()
        {
            float VALUE = Single.NegativeInfinity;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestFloat");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterFloat(new MetaField(MetaTypeId.TYPE_FLOAT, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"-Infinity\"}");
        }

        [Fact]
        public void TestDouble()
        {
            double VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestDouble");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterDouble(new MetaField(MetaTypeId.TYPE_DOUBLE, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":-2}");
        }

        [Fact]
        public void TestDoubleNaN()
        {
            double VALUE = Single.NaN;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestDouble");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterDouble(new MetaField(MetaTypeId.TYPE_DOUBLE, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"NaN\"}");
        }

        [Fact]
        public void TestDoubleInfinity()
        {
            double VALUE = Single.PositiveInfinity;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestDouble");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterDouble(new MetaField(MetaTypeId.TYPE_DOUBLE, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"Infinity\"}");
        }

        [Fact]
        public void TestDoubleNInfinity()
        {
            double VALUE = Single.NegativeInfinity;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestDouble");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterDouble(new MetaField(MetaTypeId.TYPE_DOUBLE, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"-Infinity\"}");
        }

        [Fact]
        public void TestString()
        {
            string VALUE = "Hello World";

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestString");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"Hello World\"}");
        }

        [Fact]
        public void TestStringAsBuffer()
        {
            string VALUE = "Hello World";

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestString");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", ""), Encoding.UTF8.GetBytes(VALUE), 0, VALUE.Length);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"Hello World\"}");
        }

        [Fact]
        public void TestBytes()
        {
            byte[] VALUE = { 0x23, 0xaa, 0x00, 0x6a, 0x40, 0x00 };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestBytes");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterBytes(new MetaField(MetaTypeId.TYPE_BYTES, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"I6oAakAA\"}");
        }

        [Fact]
        public void TestStruct()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestStruct");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0));
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0), VALUE_INT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0), VALUE_STRING);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 2), VALUE_UINT32);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"struct_int32\":{\"value\":-2},\"struct_string\":{\"value\":\"Hello World\"},\"last_value\":123}");
        }

        [Fact]
        public void TestEnum()
        {
            test.Foo VALUE = test.Foo.FOO_HELLO;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestEnum");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterEnum(new MetaField(MetaTypeId.TYPE_ENUM, "test.Foo", "value", ""), (int)VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"FOO_HELLO\"}");
        }

        [Fact]
        public void TestEnumAsString()
        {
            test.Foo VALUE = test.Foo.FOO_HELLO;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestEnum");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterEnum(new MetaField(MetaTypeId.TYPE_ENUM, "test.Foo", "value", ""), VALUE.ToString());
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":\"FOO_HELLO\"}");
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
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":{\"type\":\"int32\",\"valint32\":-2}}");
        }

        [Fact]
        public void TestVariantInt32Default()
        {
            int VALUE = -2;

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestVariant");
            Debug.Assert(stru != null);
            MetaStruct? struValue = MetaDataGlobal.Instance.GetStruct(stru.GetFieldByName("value")!.TypeName);
            m_serializerDefault.StartStruct(stru);
            m_serializerDefault.EnterStruct(stru.GetFieldByName("value")!);
            m_serializerDefault.EnterEnum(struValue!.GetFieldByName("type")!, "int32");
            m_serializerDefault.EnterInt32(struValue!.GetFieldByName("valint32")!, VALUE);
            m_serializerDefault.ExitStruct(stru.GetFieldByName("value")!);
            m_serializerDefault.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":{\"type\":\"int32\",\"valint32\":-2},\"valueInt32\":0,\"value2\":{}}");
        }

        [Fact]
        public void TestVariantEmptyDefault()
        {
            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestVariant");
            Debug.Assert(stru != null);
            MetaStruct? struValue = MetaDataGlobal.Instance.GetStruct(stru.GetFieldByName("value")!.TypeName);
            m_serializerDefault.StartStruct(stru);
            m_serializerDefault.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":{},\"valueInt32\":0,\"value2\":{}}");
        }

        [Fact]
        public void TestVariantStructDefault()
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
            m_serializerDefault.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            m_serializerDefault.EnterStruct(fieldValue);
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            m_serializerDefault.EnterArrayStruct(fieldList);
            // {"key1", VariantList{
            m_serializerDefault.EnterStruct(fieldListWithoutArray);
            m_serializerDefault.EnterString(fieldName, "key1");
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_LIST);
            m_serializerDefault.EnterArrayStruct(fieldList);
            // 2
            m_serializerDefault.EnterStruct(fieldListWithoutArray);
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            m_serializerDefault.EnterInt32(fieldInt32, 2);
            m_serializerDefault.ExitStruct(fieldListWithoutArray);
            // , std::string("Hello")
            m_serializerDefault.EnterStruct(fieldListWithoutArray);
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            m_serializerDefault.EnterString(fieldString, "Hello");
            m_serializerDefault.ExitStruct(fieldListWithoutArray);
            // }
            m_serializerDefault.ExitArrayStruct(fieldList);
            m_serializerDefault.ExitStruct(fieldListWithoutArray);

            // {"key2", VariantStruct{
            m_serializerDefault.EnterStruct(fieldListWithoutArray);
            m_serializerDefault.EnterString(fieldName, "key2");
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            m_serializerDefault.EnterArrayStruct(fieldList);
            // {"a", 3},
            m_serializerDefault.EnterStruct(fieldListWithoutArray);
            m_serializerDefault.EnterString(fieldName, "a");
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            m_serializerDefault.EnterInt32(fieldInt32, 3);
            m_serializerDefault.ExitStruct(fieldListWithoutArray);
            // {"b", std::string("Hi")}
            m_serializerDefault.EnterStruct(fieldListWithoutArray);
            m_serializerDefault.EnterString(fieldName, "b");
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            m_serializerDefault.EnterString(fieldString, "Hi");
            m_serializerDefault.ExitStruct(fieldListWithoutArray);
            // }
            m_serializerDefault.ExitArrayStruct(fieldList);
            m_serializerDefault.ExitStruct(fieldListWithoutArray);

            // {
            m_serializerDefault.EnterStruct(fieldListWithoutArray);
            m_serializerDefault.EnterString(fieldName, "key3");
            m_serializerDefault.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_NONE);
            m_serializerDefault.ExitStruct(fieldListWithoutArray);
            // }}
            m_serializerDefault.ExitArrayStruct(fieldList);
            m_serializerDefault.ExitStruct(fieldValue);
            m_serializerDefault.Finished();

            string cmp = "{\"value\":{\"type\":\"struct\",\"vallist\":[{\"name\":\"key1\",\"type\":\"list\",\"vallist\":[{\"type\":\"int32\",\"valint32\":2},{\"type\":\"string\",\"valstring\":\"Hello\"}]},{\"name\":\"key2\",\"type\":\"struct\",\"vallist\":[{\"name\":\"a\",\"type\":\"int32\",\"valint32\":3},{\"name\":\"b\",\"type\":\"string\",\"valstring\":\"Hi\"}]},{\"name\":\"key3\",\"type\":\"none\"}]},\"valueInt32\":0,\"value2\":{}}";
            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == cmp);
        }

        [Fact]
        public void TestArrayBool()
        {
            bool[] VALUE = { true, false, true };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayBool");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayBool(new MetaField(MetaTypeId.TYPE_ARRAY_BOOL, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[true,false,true]}");
        }

        [Fact]
        public void TestArrayInt32()
        {
            int[] VALUE = { -1, 0, 1 };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayInt32");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayInt32(new MetaField(MetaTypeId.TYPE_ARRAY_INT32, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[-1,0,1]}");
        }

        [Fact]
        public void TestArrayUInt32()
        {
            uint[] VALUE = { 0xfffffffe, 0, 1 };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayUInt32");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayUInt32(new MetaField(MetaTypeId.TYPE_ARRAY_UINT32, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[4294967294,0,1]}");
        }

        [Fact]
        public void TestArrayInt64()
        {
            long[] VALUE = { -1, 0, 1 };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayInt64");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayInt64(new MetaField(MetaTypeId.TYPE_ARRAY_INT64, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[\"-1\",\"0\",\"1\"]}");
        }

        [Fact]
        public void TestArrayUInt64()
        {
            ulong[] VALUE = { 0xfffffffffffffffe, 0, 1 };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayUInt64");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayUInt64(new MetaField(MetaTypeId.TYPE_ARRAY_UINT64, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[\"18446744073709551614\",\"0\",\"1\"]}");
        }

        [Fact]
        public void TestArrayFloat()
        {
            float[] VALUE = { -1, 0, 1 };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayFloat");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayFloat(new MetaField(MetaTypeId.TYPE_ARRAY_FLOAT, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[-1,0,1]}");
        }

        [Fact]
        public void TestArrayFloatNaN()
        {
            float[] VALUE = { Single.NaN, Single.PositiveInfinity, Single.NegativeInfinity };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayFloat");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayFloat(new MetaField(MetaTypeId.TYPE_ARRAY_FLOAT, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[\"NaN\",\"Infinity\",\"-Infinity\"]}");
        }

        [Fact]
        public void TestArrayDouble()
        {
            double[] VALUE = { -1.1, 0, 1.2 };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayDouble");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayDouble(new MetaField(MetaTypeId.TYPE_ARRAY_DOUBLE, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[-1.1,0,1.2]}");
        }

        [Fact]
        public void TestArrayDoubleNaN()
        {
            double[] VALUE = { Double.NaN, Double.PositiveInfinity, Double.NegativeInfinity };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayDouble");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayDouble(new MetaField(MetaTypeId.TYPE_ARRAY_DOUBLE, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[\"NaN\",\"Infinity\",\"-Infinity\"]}");
        }

        [Fact]
        public void TestArrayString()
        {
            string[] VALUE = { "Hello", "", "World" };

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayString");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayString(new MetaField(MetaTypeId.TYPE_ARRAY_STRING, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[\"Hello\",\"\",\"World\"]}");
        }

        [Fact]
        public void TestArrayBytes()
        {
            IList<byte[]> VALUE = new List<byte[]>();
            VALUE.Add(new byte[] { 0x23, 0xaa, 0x00, 0x6a, 0x40, 0x00 });
            VALUE.Add(new byte[] { });
            VALUE.Add(new byte[] { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff });
            VALUE.Add(new byte[] { 0x00 });

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestArrayBytes");
            Debug.Assert(stru != null);
            m_serializer.StartStruct(stru);
            m_serializer.EnterArrayBytes(new MetaField(MetaTypeId.TYPE_ARRAY_BYTES, "", "value", ""), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[\"I6oAakAA\",\"\",\"ABEiM0RVZneImaq7zN3u/w==\",\"AA==\"]}");
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


            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayStruct")!);
            m_serializer.EnterArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc"));

            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0));
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0), VALUE1_INT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0), VALUE1_STRING);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 2), VALUE1_UINT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0));

            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0));
            m_serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0), VALUE2_INT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0));
            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            m_serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0), VALUE2_STRING);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            m_serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 2), VALUE2_UINT32);
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0));

            m_serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0));
            m_serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0));

            m_serializer.ExitArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc"));

            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[{\"struct_int32\":{\"value\":-2},\"struct_string\":{\"value\":\"Hello World\"},\"last_value\":123},{\"struct_int32\":{\"value\":345},\"struct_string\":{\"value\":\"foo\"},\"last_value\":12345678},{}]}");
        }

        [Fact]
        public void TestArray…num()
        {
            int[] VALUE = { (int)test.Foo.FOO_HELLO, (int)test.Foo.FOO_WORLD, (int)test.Foo.FOO_WORLD2, 123 };

            m_serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayEnum")!);
            m_serializer.EnterArrayEnum(new MetaField(MetaTypeId.TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0), VALUE);
            m_serializer.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[\"FOO_HELLO\",\"FOO_WORLD\",\"world2\",\"FOO_WORLD\"]}");
        }

        [Fact]
        public void TestArray…numAsInt()
        {
            int[] VALUE = { (int)test.Foo.FOO_HELLO, (int)test.Foo.FOO_WORLD, (int)test.Foo.FOO_WORLD2, 123 };

            m_serializerEnumAsInt.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayEnum")!);
            m_serializerEnumAsInt.EnterArrayEnum(new MetaField(MetaTypeId.TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0), VALUE);
            m_serializerEnumAsInt.Finished();

            string s = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(s == "{\"value\":[-2,0,1,123]}");
        }

    }
}

