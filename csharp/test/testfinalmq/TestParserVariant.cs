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

    public class TestParserVariant : IDisposable
    {
        MetaField? m_fieldValue = null;
        MetaField? m_fieldValue2 = null;
        MetaField? m_fieldValueInt32 = null;
        MetaField? m_fieldName = null;
        MetaField? m_fieldIndex = null;
        MetaField? m_fieldInt32 = null;
        MetaField? m_fieldString = null;
        MetaField? m_fieldStruct = null;
        MetaField? m_fieldStructWithoutArray = null;
        MetaField? m_fieldList = null;
        MetaField? m_fieldListWithoutArray = null;

        public TestParserVariant()
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
            m_fieldIndex = structVarVariant.GetFieldByName("index");
            m_fieldInt32 = structVarVariant.GetFieldByName("valint32");
            m_fieldString = structVarVariant.GetFieldByName("valstring");
            m_fieldStruct = structVarVariant.GetFieldByName("valstruct");
            m_fieldStructWithoutArray = MetaDataGlobal.Instance.GetArrayField(m_fieldStruct!);
            m_fieldList = structVarVariant.GetFieldByName("vallist");
            m_fieldListWithoutArray = MetaDataGlobal.Instance.GetArrayField(m_fieldList!);

            Debug.Assert(m_fieldName != null);
            Debug.Assert(m_fieldIndex != null);
            Debug.Assert(m_fieldInt32 != null);
            Debug.Assert(m_fieldString != null);
            Debug.Assert(m_fieldStruct != null);
            Debug.Assert(m_fieldStructWithoutArray != null);
            Debug.Assert(m_fieldList != null);
            Debug.Assert(m_fieldListWithoutArray != null);
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestUnknownStruct()
        {
            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();
            Variant root = new Variant();
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.BlaBla");
            Debug.Assert(!res);

            mockVisitor.Verify(x => x.NotifyError(It.IsAny<string>(), It.IsAny<string>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestBool()
        {
            bool VALUE = true;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestBool", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) } );
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestBool");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterBool(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt8()
        {
            sbyte VALUE = -2;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestInt8", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestInt8");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterInt8(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt8()
        {
            byte VALUE = 130;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestUInt8", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestUInt8");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterUInt8(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt16()
        {
            short VALUE = -2;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestInt16", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestInt16");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterInt16(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt16()
        {
            ushort VALUE = 130;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestUInt16", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestUInt16");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterUInt16(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt32()
        {
            int VALUE = -2;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestInt32", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestFloat");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterFloat(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestFloatNaN()
        {
            float VALUE = Single.NaN;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestFloat", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestFloat");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterFloat(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestFloatInfinity()
        {
            float VALUE = Single.PositiveInfinity;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestFloat", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestFloat");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterFloat(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestFloatNInfinity()
        {
            float VALUE = Single.NegativeInfinity;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestFloat", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestDouble");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterDouble(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestDoubleNaN()
        {
            double VALUE = Double.NaN;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestDouble", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestDouble");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterDouble(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestDoubleInfinity()
        {
            double VALUE = Double.PositiveInfinity;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestDouble", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestDouble");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterDouble(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestDoubleNInfinity()
        {
            double VALUE = Double.NegativeInfinity;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestDouble", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestString");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterString(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestBytes()
        {
            byte[] VALUE = { 0x23, 0xaa, 0x00, 0x6a, 0x40, 0x00 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestBytes", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestBytes");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterBytes(fieldValue, VALUE, 0, VALUE.Length), Times.Once);
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

            mockVisitor.Setup(x => x.EnterString(fieldString, It.IsAny<byte[]>(), 10, 11))
                .Callback((MetaField field, byte[] buffer, int offset, int size) =>
                {
                    string v = Encoding.UTF8.GetString(buffer, offset, size);
                    Debug.Assert(v == VALUE_STRING);
                });

            Variant root = Variant.Create(new VariantStruct { new NameValue("struct_int32",  Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_INT32)) })),
                                                              new NameValue("struct_string", Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE_STRING)) }))});
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestStruct");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterStruct(fieldStructInt32), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(fieldInt32, VALUE_INT32), Times.Once);
            mockVisitor.Verify(x => x.ExitStruct(fieldStructInt32), Times.Once);
            mockVisitor.Verify(x => x.EnterStruct(fieldStructString), Times.Once);
            mockVisitor.Verify(x => x.EnterString(fieldString, It.IsAny<string>()), Times.Once);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
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

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", new Variant()) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestVariant");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterStruct(m_fieldValue!), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_NONE), Times.Exactly(2));
            mockVisitor.Verify(x => x.ExitStruct(m_fieldValue!), Times.Once);

            mockVisitor.Verify(x => x.EnterInt32(m_fieldValueInt32!, 0), Times.Once);

            mockVisitor.Verify(x => x.EnterStruct(m_fieldValue2!), Times.Once);
            //            mockVisitor.Verify(x => x.EnterEnum(m_fieldType!, variant.VarTypeId.T_NONE), Times.Once);
            mockVisitor.Verify(x => x.ExitStruct(m_fieldValue2!), Times.Once);

            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestVariantString()
        {
            string VALUE_STRING = "123";

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value",  Variant.Create(VALUE_STRING)),
                                                              new NameValue("valueInt32",  Variant.Create(0)),
                                                              new NameValue("value2", new Variant())});
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestVariant");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterStruct(m_fieldValue!), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_STRING), Times.Exactly(1));
            mockVisitor.Verify(x => x.EnterString(m_fieldString!, VALUE_STRING), Times.Exactly(1));
            mockVisitor.Verify(x => x.ExitStruct(m_fieldValue!), Times.Once);

            mockVisitor.Verify(x => x.EnterInt32(m_fieldValueInt32!, 0), Times.Once);

            mockVisitor.Verify(x => x.EnterStruct(m_fieldValue2!), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_NONE), Times.Once);
            mockVisitor.Verify(x => x.ExitStruct(m_fieldValue2!), Times.Once);

            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }


        [Fact]
        public void TestVariantStruct()
        {
            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value",  Variant.Create(new VariantStruct{ new NameValue("key1", Variant.Create(data: new VariantList{ Variant.Create(2), Variant.Create("Hello")})),
                                                                              new NameValue("key2", Variant.Create(new VariantStruct{ new NameValue("a", Variant.Create(3)), new NameValue("b", Variant.Create("Hi"))})),
                                                                              new NameValue("key3", new Variant() ),
                                                                             })),
                                                              new NameValue("valueInt32",  Variant.Create(0)),
                                                              new NameValue("value2", new Variant())});
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestVariant");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Exactly(1));
            // VariantStruct{ {"value", VariantStruct{
            mockVisitor.Verify(x => x.EnterStruct(m_fieldValue!), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_VARIANTSTRUCT), Times.Exactly(2));
            mockVisitor.Verify(x => x.EnterArrayStruct(m_fieldStruct!), Times.Exactly(2));
            // {"key1", VariantList{
            mockVisitor.Verify(x => x.EnterStruct(m_fieldStructWithoutArray!), Times.Exactly(5));
            mockVisitor.Verify(x => x.EnterString(m_fieldName!, "key1"), Times.Exactly(1));
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_VARIANTLIST), Times.Exactly(1));
            mockVisitor.Verify(x => x.EnterArrayStruct(m_fieldList!), Times.Once);
            // 2
            mockVisitor.Verify(x => x.EnterStruct(m_fieldListWithoutArray!), Times.Exactly(2));
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_INT32), Times.Exactly(2));
            mockVisitor.Verify(x => x.EnterInt32(m_fieldInt32!, 2), Times.Exactly(1));
            mockVisitor.Verify(x => x.ExitStruct(m_fieldListWithoutArray!), Times.Exactly(2));
            // , std::string("Hello")
//            mockVisitor.Verify(x => x.EnterStruct(m_fieldListWithoutArray!), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_STRING), Times.Exactly(2));
            mockVisitor.Verify(x => x.EnterString(m_fieldString!, "Hello"), Times.Exactly(1));
//            mockVisitor.Verify(x => x.ExitStruct(m_fieldListWithoutArray!), Times.Once);
            // }
            mockVisitor.Verify(x => x.ExitArrayStruct(m_fieldList!), Times.Exactly(1));
            mockVisitor.Verify(x => x.ExitStruct(m_fieldStructWithoutArray!), Times.Exactly(5));

            // {"key2", VariantStruct{
//            mockVisitor.Verify(x => x.EnterStruct(m_fieldStructWithoutArray!), Times.Once);
            mockVisitor.Verify(x => x.EnterString(m_fieldName!, "key2"), Times.Exactly(1));
//            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_STRUCT), Times.Exactly(1));
//            mockVisitor.Verify(x => x.EnterArrayStruct(m_fieldStruct!), Times.Exactly(1));
            // {"a", 3},
//            mockVisitor.Verify(x => x.EnterStruct(m_fieldStructWithoutArray!), Times.Once);
            mockVisitor.Verify(x => x.EnterString(m_fieldName!, "a"), Times.Exactly(1));
//            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_INT32), Times.Exactly(1));
            mockVisitor.Verify(x => x.EnterInt32(m_fieldInt32!, 3), Times.Exactly(1));
//            mockVisitor.Verify(x => x.ExitStruct(m_fieldStructWithoutArray!), Times.Once);
            // {"b", std::string("Hi")}
//            mockVisitor.Verify(x => x.EnterStruct(m_fieldStructWithoutArray!), Times.Once);
            mockVisitor.Verify(x => x.EnterString(m_fieldName!, "b"), Times.Exactly(1));
//            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_STRING), Times.Exactly(1));
            mockVisitor.Verify(x => x.EnterString(m_fieldString!, "Hi"), Times.Exactly(1));
//            mockVisitor.Verify(x => x.ExitStruct(m_fieldStructWithoutArray!), Times.Once);
            // }
            mockVisitor.Verify(x => x.ExitArrayStruct(m_fieldStruct!), Times.Exactly(2));
//            mockVisitor.Verify(x => x.ExitStruct(m_fieldStructWithoutArray!), Times.Once);

            // {
//            mockVisitor.Verify(x => x.EnterStruct(m_fieldStructWithoutArray!), Times.Once);
            mockVisitor.Verify(x => x.EnterString(m_fieldName!, "key3"), Times.Exactly(1));
            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_NONE), Times.Exactly(2));
//            mockVisitor.Verify(x => x.ExitStruct(m_fieldStructWithoutArray!), Times.Once);
            // }}
//            mockVisitor.Verify(x => x.ExitArrayStruct(m_fieldStruct!), Times.Once);
            mockVisitor.Verify(x => x.ExitStruct(m_fieldValue!), Times.Once);

            mockVisitor.Verify(x => x.EnterInt32(m_fieldValueInt32!, 0), Times.Exactly(1));

            mockVisitor.Verify(x => x.EnterStruct(m_fieldValue2!), Times.Once);
//            mockVisitor.Verify(x => x.EnterInt32(m_fieldIndex!, (int)VarValueType2Index.VARVALUETYPE_NONE), Times.Exactly(1));
            mockVisitor.Verify(x => x.ExitStruct(m_fieldValue2!), Times.Once);

            mockVisitor.Verify(x => x.Finished(), Times.Exactly(1));
        }


        [Fact]
        public void TestArrayBool()
        {
            bool[] VALUE = { true, false, false, true };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayBool", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayBool");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayBool(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayInt8()
        {
            sbyte[] VALUE = { -2, 0, 2, 22 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayInt8", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayInt8");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayInt8(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayInt16()
        {
            short[] VALUE = { -2, 0, 2, 222 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayInt16", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayInt16");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayInt16(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayUInt16()
        {
            ushort[] VALUE = { 0xFFFE, 0, 2, 222 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayUInt16", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayUInt16");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayUInt16(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayInt32()
        {
            int[] VALUE = { -2, 0, 2, 222 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayInt32", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayInt32");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayInt32(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayUInt32()
        {
            uint[] VALUE = { 0xFFFFFFFE, 0, 2, 222 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayUInt32", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayUInt32");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayUInt32(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayInt64()
        {
            long[] VALUE = { -2, 0, 2, 222 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayInt64", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayInt64");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayInt64(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayUInt64()
        {
            ulong[] VALUE = { 0xFFFFFFFFFFFFFFF, 0, 2, 222 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayUInt64", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayUInt64");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayUInt64(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayFloat()
        {
            float[] VALUE = { -2.1f, 0f, 2.1f, 222.1f };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayFloat", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayFloat");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayFloat(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayFloatNaN()
        {
            float[] VALUE = { Single.NaN, Single.PositiveInfinity, Single.NegativeInfinity };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayFloat", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayFloat");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayFloat(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayDouble()
        {
            double[] VALUE = { -2.1, 0, 2.1, 222.1 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayDouble", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayDouble");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayDouble(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayDoubleNaN()
        {
            double[] VALUE = { Double.NaN, Double.PositiveInfinity, Double.NegativeInfinity };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayDouble", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayDouble");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayDouble(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayString()
        {
            IList<string> VALUE = new List<string>();
            VALUE.Add("Hello");
            VALUE.Add("");
            VALUE.Add("World");
            VALUE.Add("Foo");

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayString", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            mockVisitor.Setup(x => x.EnterArrayString(fieldValue, It.IsAny<IList<string>>()))
                .Callback((MetaField field, IList<string> value) =>
                {
                    Debug.Assert(value.SequenceEqual(VALUE));
                });

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayString");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayString(fieldValue, It.IsAny<IList<string>>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayBytes()
        {
            IList<byte[]> VALUE = new List<byte[]>();
            VALUE.Add(new byte[] { 0x23, 0xaa, 0x00, 0x6a, 0x40, 0x00 });
            VALUE.Add(new byte[] { });
            VALUE.Add(new byte[] { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff });
            VALUE.Add(new byte[] { 0x00 });

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayBytes", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            mockVisitor.Setup(x => x.EnterArrayBytes(fieldValue, It.IsAny<IList<byte[]>>()))
                .Callback((MetaField field, IList<byte[]> value) =>
                {
                    Debug.Assert(value.Count == VALUE.Count);
                    for (int i = 0; i < VALUE.Count; i++)
                    {
                        Debug.Assert(value[i].SequenceEqual(VALUE[i]));
                    }
                });

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayBytes");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayBytes(fieldValue, It.IsAny<IList<byte[]>>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayStruct()
        {
            int VALUE1_INT32 = -2;
            string VALUE1_STRING = "Hello World";
            int VALUE2_INT32 = 345;
            string VALUE2_STRING = "foo";

            MetaField? fieldStruct = MetaDataGlobal.Instance.GetField("test.TestArrayStruct", "value");
            MetaField? fieldStructWithoutArray = MetaDataGlobal.Instance.GetArrayField("test.TestArrayStruct", "value");
            MetaField? fieldStructInt32 = MetaDataGlobal.Instance.GetField("test.TestStruct", "struct_int32");
            MetaField? fieldStructString = MetaDataGlobal.Instance.GetField("test.TestStruct", "struct_string");
            MetaField? fieldInt32 = MetaDataGlobal.Instance.GetField("test.TestInt32", "value");
            MetaField? fieldString = MetaDataGlobal.Instance.GetField("test.TestString", "value");
            Debug.Assert(fieldStruct != null);
            Debug.Assert(fieldStructWithoutArray != null);
            Debug.Assert(fieldStructInt32 != null);
            Debug.Assert(fieldStructString != null);
            Debug.Assert(fieldInt32 != null);
            Debug.Assert(fieldString != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(new VariantList {
                                                                    Variant.Create(new VariantStruct { new NameValue("struct_int32",  Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE1_INT32)) })),
                                                                                                       new NameValue("struct_string", Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE1_STRING)) }))}),
                                                                    Variant.Create(new VariantStruct { new NameValue("struct_int32",  Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE2_INT32)) })),
                                                                                                       new NameValue("struct_string", Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE2_STRING)) }))}),
                                                                    Variant.Create(new VariantStruct { }),
                                                                })),
                                                              new NameValue("last_value", Variant.Create(0))});



            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayStruct");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayStruct(fieldStruct), Times.Once);

            mockVisitor.Verify(x => x.EnterStruct(fieldStructWithoutArray), Times.Exactly(3));
            mockVisitor.Verify(x => x.EnterStruct(fieldStructInt32), Times.Exactly(3));
            mockVisitor.Verify(x => x.EnterInt32(fieldInt32, VALUE1_INT32), Times.Once);
            mockVisitor.Verify(x => x.ExitStruct(fieldStructInt32), Times.Exactly(3));
            mockVisitor.Verify(x => x.EnterStruct(fieldStructString), Times.Exactly(3));
            mockVisitor.Verify(x => x.EnterString(fieldString, It.IsAny<string>()), Times.Exactly(3));
            mockVisitor.Verify(x => x.ExitStruct(fieldStructString), Times.Exactly(3));
            mockVisitor.Verify(x => x.ExitStruct(fieldStructWithoutArray), Times.Exactly(3));

            //            mockVisitor.Verify(x => x.EnterStruct(fieldStructWithoutArray), Times.Once);
            //            mockVisitor.Verify(x => x.EnterStruct(fieldStructInt32), Times.Once);
            mockVisitor.Verify(x => x.EnterInt32(fieldInt32, VALUE2_INT32), Times.Once);
            //            mockVisitor.Verify(x => x.ExitStruct(fieldStructInt32), Times.Once);
            //            mockVisitor.Verify(x => x.EnterStruct(fieldStructString), Times.Once);
            //            mockVisitor.Verify(x => x.EnterString(fieldString, It.IsAny<byte[]>(), It.IsAny<int>(), It.IsAny<int>()), Times.Once);
            //            mockVisitor.Verify(x => x.ExitStruct(fieldStructString), Times.Once);
            //            mockVisitor.Verify(x => x.ExitStruct(fieldStructWithoutArray), Times.Once);

            //            mockVisitor.Verify(x => x.EnterStruct(fieldStructWithoutArray), Times.Once);
            //            mockVisitor.Verify(x => x.ExitStruct(fieldStructWithoutArray), Times.Once);

            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayEnum()
        {
            int[] VALUE = new int[]{ -2, 0, 1, 42 };

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestArrayEnum", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            Variant root = Variant.Create(new VariantStruct { new NameValue("value", Variant.Create(VALUE)) });
            ParserVariant parser = new ParserVariant(mockVisitor.Object, root);
            bool res = parser.ParseStruct("test.TestArrayEnum");
            Debug.Assert(res);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterArrayEnum(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

    }
}

