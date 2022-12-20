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
        MetaField? m_fieldValue = null;
        MetaField? m_fieldValue2 = null;
        MetaField? m_fieldValueInt32 = null;
        MetaField? m_fieldName = null;
        MetaField? m_fieldType = null;
        MetaField? m_fieldInt32 = null;
        MetaField? m_fieldString = null;
        MetaField? m_fieldList = null;
        MetaField? m_fieldListWithoutArray = null;

        public TestSerializerStruct()
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
        public void TestBool()
        {
            bool VALUE = true;

            var root = new test.TestBool();
            IParserVisitor serializer = new SerializerStruct(root);

            serializer.StartStruct(MetaDataGlobal.Instance.GetStruct("test.TestBool")!);
            serializer.EnterBool(new MetaField(MetaTypeId.TYPE_BOOL, "", "value", "", 0, 0), VALUE);
            serializer.Finished();

            var cmp = new test.TestBool(VALUE);
            Debug.Assert(root.value == cmp.value);
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
            Debug.Assert(root.value == cmp.value);
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
            Debug.Assert(root.value == cmp.value);
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
            Debug.Assert(root.value == cmp.value);
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
            Debug.Assert(root.value == cmp.value);
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
            Debug.Assert(root.value == cmp.value);
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
            Debug.Assert(root.value.SequenceEqual(cmp.value));
        }

    }
}

