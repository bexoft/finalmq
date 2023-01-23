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
    using VariantStruct = List<NameValue>;
    using VariantList = List<Variant>;

    public class TestSerializerStruct : IDisposable
    {
        MetaField m_fieldValue;
        MetaField m_fieldValue2;
        MetaField m_fieldValueInt32;
        MetaField m_fieldName;
        MetaField m_fieldType;
        MetaField m_fieldInt32;
        MetaField m_fieldString;
        MetaField m_fieldList;
        MetaField m_fieldListWithoutArray;

        public TestSerializerStruct()
        {
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
        }

        [Fact]
        public void TestBool()
        {
            bool VALUE = true;

            var root = new test.TestBool();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestBool")!);
            serializer.EnterBool(new MetaField(MetaTypeId.TYPE_BOOL, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestBool(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt32()
        {
            int VALUE = -2;

            var root = new test.TestInt32();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestInt32")!);
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestInt32(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt32()
        {
            uint VALUE = 0xFFFFFFFE;

            var root = new test.TestUInt32();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestUInt32")!);
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestUInt32(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt64()
        {
            long VALUE = -2;

            var root = new test.TestInt64();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestInt64")!);
            serializer.EnterInt64(new MetaField(MetaTypeId.TYPE_INT64, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestInt64(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt64()
        {
            ulong VALUE = 0xFFFFFFFFFFFFFFFE;

            var root = new test.TestUInt64();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestUInt64")!);
            serializer.EnterUInt64(new MetaField(MetaTypeId.TYPE_UINT64, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestUInt64(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestFloat()
        {
            float VALUE = -2.1f;

            var root = new test.TestFloat();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestFloat")!);
            serializer.EnterFloat(new MetaField(MetaTypeId.TYPE_FLOAT, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestFloat(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestDouble()
        {
            double VALUE = -2.1;

            var root = new test.TestDouble();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestDouble")!);
            serializer.EnterDouble(new MetaField(MetaTypeId.TYPE_DOUBLE, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestDouble(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestString()
        {
            string VALUE = "Hello World";

            var root = new test.TestString();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestString")!);
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestString(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestBytes()
        {
            byte[] VALUE = { (byte)'H', (byte)'e', (byte)'l', 0, 13, (byte)'l', (byte)'o' };

            var root = new test.TestBytes();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestString")!);
            serializer.EnterBytes(new MetaField(MetaTypeId.TYPE_BYTES, "", "value", "", 0, 0), VALUE, 0, VALUE.Length);
            serializer.Finished();

            var cmp = new test.TestBytes(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestStruct()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            var root = new test.TestStruct();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStruct")!);
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, 0), VALUE_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", 0, 2), VALUE_UINT32);
            serializer.Finished();

            var cmp = new test.TestStruct(new test.TestInt32(VALUE_INT32), new test.TestString(VALUE_STRING), VALUE_UINT32);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestStructNullableNotNull()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            var root = new test.TestStructNullable();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStructNullable")!);
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, 0), VALUE_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", 0, 2), VALUE_UINT32);
            serializer.Finished();

            var cmp = new test.TestStructNullable(new test.TestInt32(VALUE_INT32), new test.TestString(VALUE_STRING), VALUE_UINT32);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestStructNullableNull()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            var root = new test.TestStructNullable();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStructNullable")!);
            serializer.EnterStructNull(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", 0, 2), VALUE_UINT32);
            serializer.Finished();

            var cmp = new test.TestStructNullable(null, new test.TestString(VALUE_STRING), VALUE_UINT32);
            Debug.Assert(root.Equals(cmp));
        }


        [Fact]
        public void TestEnum()
        {
            test.Foo VALUE = test.Foo.FOO_HELLO;

            var root = new test.TestEnum();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestEnum")!);
            serializer.EnterEnum(new MetaField(MetaTypeId.TYPE_ENUM, "test.Foo", "value", "", 0, 0), (int)VALUE);
            serializer.Finished();

            var cmp = new test.TestEnum(VALUE);
            Debug.Assert(root.value == cmp.value);
        }

        [Fact]
        public void TestEnumAsString()
        {
            test.Foo VALUE = test.Foo.FOO_WORLD2;

            var root = new test.TestEnum();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestEnum")!);
            serializer.EnterEnum(new MetaField(MetaTypeId.TYPE_ENUM, "test.Foo", "value", "", 0, 0), "world2");
            serializer.Finished();

            var cmp = new test.TestEnum(VALUE);
            Debug.Assert(root.value == cmp.value);
        }

        [Fact]
        public void TestVariantEmptyDefault()
        {
            var root = new test.TestVariant();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.Finished();

            var cmp = new test.TestVariant();
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantEmpty()
        {
            var root = new test.TestVariant();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.EnterStruct(m_fieldValue);
            serializer.EnterEnum(m_fieldType, (int)finalmq.variant.VarTypeId.T_NONE);
            serializer.ExitStruct(m_fieldValue);
            serializer.Finished();

            var cmp = new test.TestVariant();
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantInt32()
        {
            int VALUE = -2;

            var root = new test.TestVariant();
            IParserVisitor serializer = new SerializerStruct(root);

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestVariant");
            Debug.Assert(stru != null);
            MetaStruct? struValue = MetaDataGlobal.Instance.GetStruct(stru.GetFieldByName("value")!.TypeName);
            serializer.StartStruct(stru);
            serializer.EnterStruct(stru.GetFieldByName("value")!);
            serializer.EnterEnum(struValue!.GetFieldByName("type")!, "int32");
            serializer.EnterInt32(struValue!.GetFieldByName("valint32")!, VALUE);
            serializer.ExitStruct(stru.GetFieldByName("value")!);
            serializer.Finished();

            var cmp = new test.TestVariant(Variant.Create(VALUE), 0, new Variant());
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

            var root = new test.TestVariant();
            IParserVisitor serializer = new SerializerStruct(root);

            // VariantStruct{ {"value", VariantStruct{
            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.EnterStruct(fieldValue);
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            serializer.EnterArrayStruct(fieldList);
            // {"key1", VariantList{
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "key1");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_LIST);
            serializer.EnterArrayStruct(fieldList);
            // 2
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            serializer.EnterInt32(fieldInt32, 2);
            serializer.ExitStruct(fieldListWithoutArray);
            // , std::string("Hello")
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            serializer.EnterString(fieldString, "Hello");
            serializer.ExitStruct(fieldListWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldListWithoutArray);

            // {"key2", VariantStruct{
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "key2");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            serializer.EnterArrayStruct(fieldList);
            // {"a", 3},
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "a");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            serializer.EnterInt32(fieldInt32, 3);
            serializer.ExitStruct(fieldListWithoutArray);
            // {"b", std::string("Hi")}
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "b");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            serializer.EnterString(fieldString, "Hi");
            serializer.ExitStruct(fieldListWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldListWithoutArray);

            // {
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "key3");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_NONE);
            serializer.ExitStruct(fieldListWithoutArray);
            // }}
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldValue);
            serializer.Finished();

            var cmp = new test.TestVariant(Variant.Create(new VariantStruct{ new NameValue("key1", Variant.Create(new VariantList{ Variant.Create(2), Variant.Create("Hello") })),
                                                                             new NameValue("key2", Variant.Create(new VariantStruct{ new NameValue("a", Variant.Create(3)), new NameValue("b", Variant.Create("Hi")) })),
                                                                             new NameValue("key3", new Variant())
                                                                           }), 0, new Variant());
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantStruct2()
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

            var root = new test.TestVariant();
            IParserVisitor serializer = new SerializerStruct(root);

            // VariantStruct{ {"value", VariantStruct{
            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.EnterStruct(fieldValue);
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            serializer.EnterArrayStruct(fieldList);
            // {"key1", VariantList{
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "key1");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_LIST);
            serializer.EnterArrayStruct(fieldList);
            // 2
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            serializer.EnterInt32(fieldInt32, 2);
            serializer.ExitStruct(fieldListWithoutArray);
            // , std::string("Hello")
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            serializer.EnterString(fieldString, "Hello");
            serializer.ExitStruct(fieldListWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldListWithoutArray);

            // {"key2", VariantStruct{
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "key2");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            serializer.EnterArrayStruct(fieldList);
            // {"a", 3},
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "a");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_INT32);
            serializer.EnterInt32(fieldInt32, 3);
            serializer.ExitStruct(fieldListWithoutArray);
            // {"b", std::string("Hi")}
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "b");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            serializer.EnterString(fieldString, "Hi");
            serializer.ExitStruct(fieldListWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldListWithoutArray);

            // {
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "key3");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_NONE);
            serializer.ExitStruct(fieldListWithoutArray);
            // }}
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldValue);

            // VariantStruct{ {"value2", VariantStruct{
            serializer.EnterStruct(fieldValue2);
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRUCT);
            serializer.EnterArrayStruct(fieldList);
            // {"key1", "Hello"}
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterString(fieldName, "key1");
            serializer.EnterEnum(fieldType, (int)finalmq.variant.VarTypeId.T_STRING);
            serializer.EnterString(fieldString, "Hello");
            serializer.ExitStruct(fieldListWithoutArray);
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldValue2);

            // {"valueInt32", 5}    
            serializer.EnterInt32(fieldValueInt32, 5);

            serializer.Finished();

            var cmp = new test.TestVariant(Variant.Create(new VariantStruct{ new NameValue("key1", Variant.Create(new VariantList{ Variant.Create(2), Variant.Create("Hello") })),
                                                                             new NameValue("key2", Variant.Create(new VariantStruct{ new NameValue("a", Variant.Create(3)), new NameValue("b", Variant.Create("Hi")) })),
                                                                             new NameValue("key3", new Variant())
                                                                           }), 
                                                                           5, 
                                                                           Variant.Create(new VariantStruct { new NameValue("key1", Variant.Create("Hello")) }  )
                                                         );
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayBool()
        {
            bool[] VALUE = { true, false, true };

            var root = new test.TestArrayBool();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayBool")!);
            serializer.EnterArrayBool(new MetaField(MetaTypeId.TYPE_ARRAY_BOOL, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayBool(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt32()
        {
            int[] VALUE = { -1, 0, 1 };

            var root = new test.TestArrayInt32();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt32")!);
            serializer.EnterArrayInt32(new MetaField(MetaTypeId.TYPE_ARRAY_INT32, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayInt32(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt32()
        {
            uint[] VALUE = { 0xfffffffe, 0, 1 };

            var root = new test.TestArrayUInt32();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayUInt32")!);
            serializer.EnterArrayUInt32(new MetaField(MetaTypeId.TYPE_ARRAY_UINT32, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayUInt32(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt64()
        {
            long[] VALUE = { -1, 0, 1 };

            var root = new test.TestArrayInt64();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt64")!);
            serializer.EnterArrayInt64(new MetaField(MetaTypeId.TYPE_ARRAY_INT64, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayInt64(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt64()
        {
            ulong[] VALUE = { 0xfffffffffffffffe, 0, 1 };

            var root = new test.TestArrayUInt64();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayUInt64")!);
            serializer.EnterArrayUInt64(new MetaField(MetaTypeId.TYPE_ARRAY_UINT64, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayUInt64(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayFloat()
        {
            float[] VALUE = { -1f, 0f, 1f };

            var root = new test.TestArrayFloat();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayFloat")!);
            serializer.EnterArrayFloat(new MetaField(MetaTypeId.TYPE_ARRAY_FLOAT, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayFloat(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayDouble()
        {
            double[] VALUE = { -1.1, 0.0, 1.1 };

            var root = new test.TestArrayDouble();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayDouble")!);
            serializer.EnterArrayDouble(new MetaField(MetaTypeId.TYPE_ARRAY_DOUBLE, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayDouble(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayString()
        {
            IList<string> VALUE = new List<string> { "Hello", "", "World" };

            var root = new test.TestArrayString();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayString")!);
            serializer.EnterArrayString(new MetaField(MetaTypeId.TYPE_ARRAY_STRING, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayString(VALUE);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayBytes()
        {
            IList<byte[]> VALUE = new List<byte[]> { Encoding.ASCII.GetBytes("Hello"), Array.Empty<byte>(), Encoding.ASCII.GetBytes("World") };

            var root = new test.TestArrayBytes();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayBytes")!);
            serializer.EnterArrayBytes(new MetaField(MetaTypeId.TYPE_ARRAY_BYTES, "", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayBytes(VALUE);
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

            var root = new test.TestArrayStruct();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayStruct")!);
            serializer.EnterArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, 0));

            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0, 0), VALUE1_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0, 0), VALUE1_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 0, 2), VALUE1_UINT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));

            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0, 0), VALUE2_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0, 0), VALUE2_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 0, 2), VALUE2_UINT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));

            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0));

            serializer.ExitArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, 0));

            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 0, 1), LAST_VALUE);

            serializer.Finished();

            test.TestArrayStruct cmp = new test.TestArrayStruct( 
                                        new List<test.TestStruct> 
                                        {
                                            new test.TestStruct(new test.TestInt32(VALUE1_INT32), new test.TestString(VALUE1_STRING), VALUE1_UINT32),
                                            new test.TestStruct(new test.TestInt32(VALUE2_INT32), new test.TestString(VALUE2_STRING), VALUE2_UINT32),
                                            new test.TestStruct()
                                        }, 
                                        LAST_VALUE);

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayEnum()
        {
            int[] VALUE = { (int)test.Foo.FOO_HELLO, (int)test.Foo.FOO_WORLD, (int)test.Foo.FOO_WORLD2, 123 };
            IList<test.Foo> VALUE_CMP = new List<test.Foo> { test.Foo.FOO_HELLO, test.Foo.FOO_WORLD, test.Foo.FOO_WORLD2, test.Foo.FOO_WORLD };

            var root = new test.TestArrayEnum();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayEnum")!);
            serializer.EnterArrayEnum(new MetaField(MetaTypeId.TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayEnum(VALUE_CMP);
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArray…numAsInt()
        {
            IList<string> VALUE = new List<string> { "FOO_HELLO", "FOO_WORLD", "FOO_WORLD2", "world2", "BlaBla" };
            IList< test.Foo> VALUE_CMP = new List<test.Foo> { test.Foo.FOO_HELLO, test.Foo.FOO_WORLD, test.Foo.FOO_WORLD2, test.Foo.FOO_WORLD2, test.Foo.FOO_WORLD };

            var root = new test.TestArrayEnum();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayEnum")!);
            serializer.EnterArrayEnum(new MetaField(MetaTypeId.TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestArrayEnum(VALUE_CMP);
            Debug.Assert(root.Equals(cmp));
        }

    }
}

