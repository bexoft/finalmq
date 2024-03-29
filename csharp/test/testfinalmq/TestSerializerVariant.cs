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

    public class TestSerializerVariant : IDisposable
    {
        MetaField m_fieldValue;
        MetaField m_fieldValue2;
        MetaField m_fieldValueInt32;
        MetaField m_fieldName;
        MetaField m_fieldIndex;
        MetaField m_fieldInt32;
        MetaField m_fieldString;
        MetaField m_fieldStruct;
        MetaField m_fieldStructWithoutArray;
        MetaField m_fieldList;
        MetaField m_fieldListWithoutArray;

        public TestSerializerVariant()
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
            m_fieldIndex = structVarVariant.GetFieldByName("index")!;
            m_fieldInt32 = structVarVariant.GetFieldByName("valint32")!;
            m_fieldString = structVarVariant.GetFieldByName("valstring")!;
            m_fieldStruct = structVarVariant.GetFieldByName("valstruct")!;
            m_fieldStructWithoutArray = MetaDataGlobal.Instance.GetArrayField(m_fieldStruct!)!;
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

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestBool")!);
            serializer.EnterBool(new MetaField(MetaTypeId.TYPE_BOOL, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt8()
        {
            sbyte VALUE = -2;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestInt8")!);
            serializer.EnterInt8(new MetaField(MetaTypeId.TYPE_INT8, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt8()
        {
            byte VALUE = 0xFE;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestUInt8")!);
            serializer.EnterUInt8(new MetaField(MetaTypeId.TYPE_UINT8, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt16()
        {
            short VALUE = -2;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestInt16")!);
            serializer.EnterInt16(new MetaField(MetaTypeId.TYPE_INT16, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt16()
        {
            ushort VALUE = 0xFFFE;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestUInt16")!);
            serializer.EnterUInt16(new MetaField(MetaTypeId.TYPE_UINT16, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt32()
        {
            int VALUE = -2;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestInt32")!);
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt32()
        {
            uint VALUE = 0xFFFFFFFE;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestUInt32")!);
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestInt64()
        {
            long VALUE = -2;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestInt64")!);
            serializer.EnterInt64(new MetaField(MetaTypeId.TYPE_INT64, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestUInt64()
        {
            ulong VALUE = 0xFFFFFFFFFFFFFFFE;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestUInt64")!);
            serializer.EnterUInt64(new MetaField(MetaTypeId.TYPE_UINT64, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestFloat()
        {
            float VALUE = -2.1f;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestFloat")!);
            serializer.EnterFloat(new MetaField(MetaTypeId.TYPE_FLOAT, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestDouble()
        {
            double VALUE = -2.1;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestDouble")!);
            serializer.EnterDouble(new MetaField(MetaTypeId.TYPE_DOUBLE, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestString()
        {
            string VALUE = "Hello World";

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestString")!);
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, null, 0), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestBytes()
        {
            byte[] VALUE = { (byte)'H', (byte)'e', (byte)'l', 0, 13, (byte)'l', (byte)'o' };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestString")!);
            serializer.EnterBytes(new MetaField(MetaTypeId.TYPE_BYTES, "", "value", "", 0, null, 0), VALUE, 0, VALUE.Length);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestStruct()
        {
            int VALUE_INT32 = -2;
            string VALUE_STRING = "Hello World";
            uint VALUE_UINT32 = 123;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestStruct")!);
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "", 0, null, 0), VALUE_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, null, 0));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "", 0, null, 0), VALUE_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "", 0, null, 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "", 0, null, 1), VALUE_UINT32);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("struct_int32",  Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_INT32))  })),
                                                             new NameValue("struct_string", Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_STRING)) })),
                                                             new NameValue("last_value",    Variant.Create(VALUE_UINT32))
                                                            });
            Debug.Assert(root.Equals(cmp));
        }


        [Fact]
        public void TestEnum()
        {
            test.Foo VALUE = test.Foo.FOO_HELLO;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestEnum")!);
            serializer.EnterEnum(new MetaField(MetaTypeId.TYPE_ENUM, "test.Foo", "value", "", 0, null, 0), (int)VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE.ToString())) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestEnumAsString()
        {
            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestEnum")!);
            serializer.EnterEnum(new MetaField(MetaTypeId.TYPE_ENUM, "test.Foo", "value", "", 0, null, 0), "world2");
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create("world2")) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantEmptyDefault()
        {
            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value",  new Variant()),
                                                             new NameValue("valueInt32", Variant.Create(0)),
                                                             new NameValue("value2", new Variant())});
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantEmpty()
        {
            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.EnterStruct(m_fieldValue);
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_NONE);
            serializer.ExitStruct(m_fieldValue);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value",  new Variant()),
                                                             new NameValue("valueInt32", Variant.Create(0)),
                                                             new NameValue("value2", new Variant())});
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestVariantInt32()
        {
            int VALUE = -2;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct("test.TestVariant");
            Debug.Assert(stru != null);
            MetaStruct? struValue = MetaDataGlobal.Instance.GetStruct(stru.GetFieldByName("value")!.TypeName);
            serializer.StartStruct(stru);
            serializer.EnterStruct(stru.GetFieldByName("value")!);
            serializer.EnterInt32(struValue!.GetFieldByName("index")!, (int)VarValueType2Index.VARVALUETYPE_INT32);
            serializer.EnterInt32(struValue!.GetFieldByName("valint32")!, VALUE);
            serializer.ExitStruct(stru.GetFieldByName("value")!);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value",  Variant.Create(VALUE)),
                                                             new NameValue("valueInt32", Variant.Create(0)),
                                                             new NameValue("value2", new Variant())});
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
            MetaField fieldIndex = structVarVariant.GetFieldByName("index")!;
            MetaField fieldInt32 = structVarVariant.GetFieldByName("valint32")!;
            MetaField fieldString = structVarVariant.GetFieldByName("valstring")!;
            MetaField fieldStruct = structVarVariant.GetFieldByName("valstruct")!;
            MetaField fieldStructWithoutArray = MetaDataGlobal.Instance.GetArrayField(fieldStruct)!;
            MetaField fieldList = structVarVariant.GetFieldByName("vallist")!;
            MetaField fieldListWithoutArray = MetaDataGlobal.Instance.GetArrayField(fieldList)!;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            // VariantStruct{ {"value", VariantStruct{
            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.EnterStruct(fieldValue);
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_VARIANTSTRUCT);
            serializer.EnterArrayStruct(fieldStruct);
            // {"key1", VariantList{
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "key1");
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_VARIANTLIST);
            serializer.EnterArrayStruct(fieldList);
            // 2
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_INT32);
            serializer.EnterInt32(fieldInt32, 2);
            serializer.ExitStruct(fieldListWithoutArray);
            // , std::string("Hello")
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_STRING);
            serializer.EnterString(fieldString, "Hello");
            serializer.ExitStruct(fieldListWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldStructWithoutArray);

            // {"key2", VariantStruct{
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "key2");
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_VARIANTSTRUCT);
            serializer.EnterArrayStruct(fieldStruct);
            // {"a", 3},
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "a");
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_INT32);
            serializer.EnterInt32(fieldInt32, 3);
            serializer.ExitStruct(fieldStructWithoutArray);
            // {"b", std::string("Hi")}
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "b");
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_STRING);
            serializer.EnterString(fieldString, "Hi");
            serializer.ExitStruct(fieldStructWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldStruct);
            serializer.ExitStruct(fieldStructWithoutArray);

            // {
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "key3");
            serializer.EnterInt32(fieldIndex, (int)VarValueType2Index.VARVALUETYPE_NONE);
            serializer.ExitStruct(fieldStructWithoutArray);
            // }}
            serializer.ExitArrayStruct(fieldStruct);
            serializer.ExitStruct(fieldValue);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value",  Variant.Create(new VariantStruct{ new NameValue("key1", Variant.Create(new VariantList{ Variant.Create(2), Variant.Create("Hello") })),
                                                                                                                       new NameValue("key2", Variant.Create(new VariantStruct{ new NameValue("a", Variant.Create(3)), new NameValue("b", Variant.Create("Hi")) })),
                                                                                                                       new NameValue("key3", new Variant())
                                                                           })),
                                                             new NameValue("valueInt32", Variant.Create(0)),
                                                             new NameValue("value2", new Variant())});
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
            MetaField fieldIndex = structVarVariant.GetFieldByName("index")!;
            MetaField fieldInt32 = structVarVariant.GetFieldByName("valint32")!;
            MetaField fieldString = structVarVariant.GetFieldByName("valstring")!;
            MetaField fieldStruct = structVarVariant.GetFieldByName("valstruct")!;
            MetaField fieldStructWithoutArray = MetaDataGlobal.Instance.GetArrayField(fieldStruct)!;
            MetaField fieldList = structVarVariant.GetFieldByName("vallist")!;
            MetaField fieldListWithoutArray = MetaDataGlobal.Instance.GetArrayField(fieldList)!;

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            // VariantStruct{ {"value", VariantStruct{
            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestVariant")!);
            serializer.EnterStruct(fieldValue);
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_VARIANTSTRUCT);
            serializer.EnterArrayStruct(fieldStruct);
            // {"key1", VariantList{
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "key1");
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_VARIANTLIST);
            serializer.EnterArrayStruct(fieldList);
            // 2
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_INT32);
            serializer.EnterInt32(fieldInt32, 2);
            serializer.ExitStruct(fieldListWithoutArray);
            // , std::string("Hello")
            serializer.EnterStruct(fieldListWithoutArray);
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_STRING);
            serializer.EnterString(fieldString, "Hello");
            serializer.ExitStruct(fieldListWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldList);
            serializer.ExitStruct(fieldStructWithoutArray);

            // {"key2", VariantStruct{
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "key2");
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_VARIANTSTRUCT);
            serializer.EnterArrayStruct(fieldStruct);
            // {"a", 3},
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "a");
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_INT32);
            serializer.EnterInt32(fieldInt32, 3);
            serializer.ExitStruct(fieldStructWithoutArray);
            // {"b", std::string("Hi")}
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "b");
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_STRING);
            serializer.EnterString(fieldString, "Hi");
            serializer.ExitStruct(fieldStructWithoutArray);
            // }
            serializer.ExitArrayStruct(fieldStruct);
            serializer.ExitStruct(fieldStructWithoutArray);

            // {
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "key3");
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_NONE);
            serializer.ExitStruct(fieldStructWithoutArray);
            // }}
            serializer.ExitArrayStruct(fieldStruct);
            serializer.ExitStruct(fieldValue);

            // {"valueInt32", 5}    
            serializer.EnterInt32(fieldValueInt32, 5);

            // VariantStruct{ {"value2", VariantStruct{
            serializer.EnterStruct(fieldValue2);
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_VARIANTSTRUCT);
            serializer.EnterArrayStruct(fieldStruct);
            // {"key1", "Hello"}
            serializer.EnterStruct(fieldStructWithoutArray);
            serializer.EnterString(fieldName, "key1");
            serializer.EnterInt32(m_fieldIndex, (int)VarValueType2Index.VARVALUETYPE_STRING);
            serializer.EnterString(fieldString, "Hello");
            serializer.ExitStruct(fieldStructWithoutArray);
            serializer.ExitArrayStruct(fieldStruct);
            serializer.ExitStruct(fieldValue2);

            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value",  Variant.Create(new VariantStruct{ new NameValue("key1", Variant.Create(new VariantList{ Variant.Create(2), Variant.Create("Hello") })),
                                                                             new NameValue("key2", Variant.Create(new VariantStruct{ new NameValue("a", Variant.Create(3)), new NameValue("b", Variant.Create("Hi")) })),
                                                                             new NameValue("key3", new Variant())
                                                                           })),
                                                             new NameValue("valueInt32", Variant.Create(5)),
                                                             new NameValue("value2", Variant.Create(new VariantStruct { new NameValue("key1", Variant.Create("Hello")) }))});
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayBool()
        {
            bool[] VALUE = { true, false, true };
            bool[] VALUE_CMP = { true, false, true };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayBool")!);
            serializer.EnterArrayBool(new MetaField(MetaTypeId.TYPE_ARRAY_BOOL, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt8()
        {
            sbyte[] VALUE = { -1, 0, 1 };
            sbyte[] VALUE_CMP = { -1, 0, 1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt8")!);
            serializer.EnterArrayInt8(new MetaField(MetaTypeId.TYPE_ARRAY_INT8, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt16()
        {
            short[] VALUE = { -1, 0, 1 };
            short[] VALUE_CMP = { -1, 0, 1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt16")!);
            serializer.EnterArrayInt16(new MetaField(MetaTypeId.TYPE_ARRAY_INT16, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt16()
        {
            ushort[] VALUE = { 0xfffe, 0, 1 };
            ushort[] VALUE_CMP = { 0xfffe, 0, 1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayUInt16")!);
            serializer.EnterArrayUInt16(new MetaField(MetaTypeId.TYPE_ARRAY_UINT16, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt32()
        {
            int[] VALUE = { -1, 0, 1 };
            int[] VALUE_CMP = { -1, 0, 1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt32")!);
            serializer.EnterArrayInt32(new MetaField(MetaTypeId.TYPE_ARRAY_INT32, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt32()
        {
            uint[] VALUE = { 0xfffffffe, 0, 1 };
            uint[] VALUE_CMP = { 0xfffffffe, 0, 1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayUInt32")!);
            serializer.EnterArrayUInt32(new MetaField(MetaTypeId.TYPE_ARRAY_UINT32, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayInt64()
        {
            long[] VALUE = { -1, 0, 1 };
            long[] VALUE_CMP = { -1, 0, 1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayInt64")!);
            serializer.EnterArrayInt64(new MetaField(MetaTypeId.TYPE_ARRAY_INT64, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayUInt64()
        {
            ulong[] VALUE = { 0xfffffffffffffffe, 0, 1 };
            ulong[] VALUE_CMP = { 0xfffffffffffffffe, 0, 1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayUInt64")!);
            serializer.EnterArrayUInt64(new MetaField(MetaTypeId.TYPE_ARRAY_UINT64, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayFloat()
        {
            float[] VALUE = { -1f, 0f, 1f };
            float[] VALUE_CMP = { -1f, 0f, 1f };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayFloat")!);
            serializer.EnterArrayFloat(new MetaField(MetaTypeId.TYPE_ARRAY_FLOAT, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayDouble()
        {
            double[] VALUE = { -1.1, 0.0, 1.1 };
            double[] VALUE_CMP = { -1.1, 0.0, 1.1 };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayDouble")!);
            serializer.EnterArrayDouble(new MetaField(MetaTypeId.TYPE_ARRAY_DOUBLE, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayString()
        {
            IList<string> VALUE = new List<string> { "Hello", "", "World" };
            IList<string> VALUE_CMP = new List<string> { "Hello", "", "World" };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayString")!);
            serializer.EnterArrayString(new MetaField(MetaTypeId.TYPE_ARRAY_STRING, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayBytes()
        {
            IList<byte[]> VALUE = new List<byte[]> { Encoding.ASCII.GetBytes("Hello"), Array.Empty<byte>(), Encoding.ASCII.GetBytes("World") };
            IList<byte[]> VALUE_CPM = new List<byte[]> { Encoding.ASCII.GetBytes("Hello"), Array.Empty<byte>(), Encoding.ASCII.GetBytes("World") };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayBytes")!);
            serializer.EnterArrayBytes(new MetaField(MetaTypeId.TYPE_ARRAY_BYTES, "", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CPM)) });
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

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayStruct")!);
            serializer.EnterArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc"));

            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, null));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, null));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0, null), VALUE1_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, null));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0, null), VALUE1_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 2), VALUE1_UINT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, null));

            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, null));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, null));
            serializer.EnterInt32(new MetaField(MetaTypeId.TYPE_INT32, "", "value", "desc", 0, null), VALUE2_INT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, null));
            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            serializer.EnterString(new MetaField(MetaTypeId.TYPE_STRING, "", "value", "desc", 0, null), VALUE2_STRING);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1));
            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 2), VALUE2_UINT32);
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, null));

            serializer.EnterStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, null));
            serializer.ExitStruct(new MetaField(MetaTypeId.TYPE_STRUCT, "test.TestStruct", "", "desc", 0, null));

            serializer.ExitArrayStruct(new MetaField(MetaTypeId.TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc"));

            serializer.EnterUInt32(new MetaField(MetaTypeId.TYPE_UINT32, "", "last_value", "desc", 1), LAST_VALUE);

            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(new VariantList {
                                                                    Variant.Create(new VariantStruct { new NameValue("struct_int32",  Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE1_INT32))  })),
                                                                                                       new NameValue("struct_string", Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE1_STRING)) })),
                                                                                                       new NameValue("last_value",  Variant.Create(VALUE1_UINT32))}),
                                                                    Variant.Create(new VariantStruct { new NameValue("struct_int32",  Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE2_INT32))  })),
                                                                                                       new NameValue("struct_string", Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE2_STRING)) })),
                                                                                                       new NameValue("last_value",  Variant.Create(VALUE2_UINT32))}),
                                                                    Variant.Create(new VariantStruct { new NameValue("struct_int32",  Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(0))  })),
                                                                                                       new NameValue("struct_string", Variant.Create(new VariantStruct { new NameValue("value", Variant.Create("")) })),
                                                                                                       new NameValue("last_value",  Variant.Create((uint)0))}),
                                                                })),
                                                              new NameValue("last_value", Variant.Create(LAST_VALUE))});

            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayEnum()
        {
            int[] VALUE = { (int)test.Foo.FOO_HELLO, (int)test.Foo.FOO_WORLD, (int)test.Foo.FOO_WORLD2, 123 };
            IList<string> VALUE_CMP = new List<string> { "FOO_HELLO", "FOO_WORLD", "world2", "FOO_WORLD" };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayEnum")!);
            serializer.EnterArrayEnum(new MetaField(MetaTypeId.TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

        [Fact]
        public void TestArrayÉnumAsInt()
        {
            IList<string> VALUE = new List<string> { "FOO_HELLO", "FOO_WORLD", "FOO_WORLD2", "world2", "BlaBla" };
            IList<string> VALUE_CMP = new List<string> { "FOO_HELLO", "FOO_WORLD", "FOO_WORLD2", "world2", "BlaBla" };

            var root = new Variant();
            IParserVisitor serializer = new SerializerVariant(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestArrayEnum")!);
            serializer.EnterArrayEnum(new MetaField(MetaTypeId.TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, null), VALUE);
            serializer.Finished();

            Variant cmp = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_CMP)) });
            Debug.Assert(root.Equals(cmp));
        }

    }
}

