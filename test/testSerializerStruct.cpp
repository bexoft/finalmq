//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/serializevariant/VarValueToVariant.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/metadataserialize/variant.fmq.h"
#include "test.fmq.h"
#include "matchers.h"


using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;

using namespace finalmq;




class TestSerializerStruct : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        const MetaStruct* structTestVariant = MetaDataGlobal::instance().getStruct("test.TestVariant");
        ASSERT_NE(structTestVariant, nullptr);
        m_fieldValue = structTestVariant->getFieldByName("value");
        ASSERT_NE(m_fieldValue, nullptr);
        ASSERT_EQ(m_fieldValue->typeName, "finalmq.variant.VarValue");
        m_fieldValue2 = structTestVariant->getFieldByName("value2");
        ASSERT_NE(m_fieldValue2, nullptr);
        ASSERT_EQ(m_fieldValue2->typeName, "finalmq.variant.VarValue");
        m_fieldValueInt32 = structTestVariant->getFieldByName("valueInt32");
        ASSERT_NE(m_fieldValueInt32, nullptr);

        const MetaStruct* structVarVariant = MetaDataGlobal::instance().getStruct("finalmq.variant.VarValue");
        ASSERT_NE(structVarVariant, nullptr);

        m_fieldName = structVarVariant->getFieldByName("name");
        m_fieldIndex = structVarVariant->getFieldByName("index");
        m_fieldInt32 = structVarVariant->getFieldByName("valint32");
        m_fieldString = structVarVariant->getFieldByName("valstring");
        m_fieldStruct = structVarVariant->getFieldByName("valstruct");
        m_fieldStructWithoutArray = MetaDataGlobal::instance().getArrayField(*m_fieldStruct);
        m_fieldList = structVarVariant->getFieldByName("vallist");
        m_fieldListWithoutArray = MetaDataGlobal::instance().getArrayField(*m_fieldList);

        ASSERT_NE(m_fieldName, nullptr);
        ASSERT_NE(m_fieldIndex, nullptr);
        ASSERT_NE(m_fieldInt32, nullptr);
        ASSERT_NE(m_fieldString, nullptr);
        ASSERT_NE(m_fieldStruct, nullptr);
        ASSERT_NE(m_fieldStructWithoutArray, nullptr);
        ASSERT_NE(m_fieldList, nullptr);
        ASSERT_NE(m_fieldListWithoutArray, nullptr);
    }

    virtual void TearDown()
    {
    }

    const MetaField* m_fieldValue = nullptr;
    const MetaField* m_fieldValue2 = nullptr;
    const MetaField* m_fieldValueInt32 = nullptr;
    const MetaField* m_fieldName = nullptr;
    const MetaField* m_fieldIndex = nullptr;
    const MetaField* m_fieldInt32 = nullptr;
    const MetaField* m_fieldString = nullptr;
    const MetaField* m_fieldStruct = nullptr;
    const MetaField* m_fieldStructWithoutArray = nullptr;
    const MetaField* m_fieldList = nullptr;
    const MetaField* m_fieldListWithoutArray = nullptr;
};




TEST_F(TestSerializerStruct, testBool)
{
    static const bool VALUE = true;

    test::TestBool root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBool"));
    serializer->enterBool(MetaField{ MetaTypeId::TYPE_BOOL, "", "value", "", 0, {}, 0 }, VALUE);
    serializer->finished();

    test::TestBool cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testInt8)
{
    static const std::int8_t VALUE = -2;

    test::TestInt8 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt8"));
    serializer->enterInt8(MetaField{MetaTypeId::TYPE_INT8, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestInt8 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt8)
{
    static const std::uint8_t VALUE = 0xFE;

    test::TestUInt8 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt8"));
    serializer->enterUInt8(MetaField{MetaTypeId::TYPE_UINT8, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestUInt8 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testInt16)
{
    static const std::int16_t VALUE = -2;

    test::TestInt16 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt16"));
    serializer->enterInt16(MetaField{ MetaTypeId::TYPE_INT16, "", "value", "", 0, {}, 0 }, VALUE);
    serializer->finished();

    test::TestInt16 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt16)
{
    static const std::uint16_t VALUE = 0xFFFE;

    test::TestUInt16 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt16"));
    serializer->enterUInt16(MetaField{ MetaTypeId::TYPE_UINT16, "", "value", "", 0, {}, 0 }, VALUE);
    serializer->finished();

    test::TestUInt16 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testInt32)
{
    static const std::int32_t VALUE = -2;

    test::TestInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt32"));
    serializer->enterInt32(MetaField{ MetaTypeId::TYPE_INT32, "", "value", "", 0, {}, 0 }, VALUE);
    serializer->finished();

    test::TestInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    test::TestUInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt32"));
    serializer->enterUInt32(MetaField{ MetaTypeId::TYPE_UINT32, "", "value", "", 0, {}, 0 }, VALUE);
    serializer->finished();

    test::TestUInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testInt64)
{
    static const std::int64_t VALUE = -2;

    test::TestInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt64"));
    serializer->enterInt64(MetaField{MetaTypeId::TYPE_INT64, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    test::TestUInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt64"));
    serializer->enterUInt64(MetaField{MetaTypeId::TYPE_UINT64, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestUInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}



TEST_F(TestSerializerStruct, testFloat)
{
    static const float VALUE = -2;

    test::TestFloat root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestFloat"));
    serializer->enterFloat(MetaField{MetaTypeId::TYPE_FLOAT, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestFloat cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testDouble)
{
    static const double VALUE = -2.1;

    test::TestDouble root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestDouble"));
    serializer->enterDouble(MetaField{MetaTypeId::TYPE_DOUBLE, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestDouble cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testString)
{
    static const std::string VALUE = "Hello World";

    test::TestString root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestString"));
    serializer->enterString(MetaField{MetaTypeId::TYPE_STRING, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestString cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    test::TestBytes root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBytes"));
    serializer->enterBytes(MetaField{MetaTypeId::TYPE_BYTES, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestBytes cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    test::TestStruct root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    serializer->enterStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    serializer->enterInt32(MetaField{MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE_INT32);
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    serializer->enterString(MetaField{MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0}, VALUE_STRING.data(), VALUE_STRING.size());
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    serializer->enterUInt32(MetaField{MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE_UINT32);

    serializer->finished();

    test::TestStruct cmp;
    cmp.struct_int32.value = VALUE_INT32;
    cmp.struct_string.value = VALUE_STRING;
    cmp.last_value = VALUE_UINT32;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testStructNullableNotNull)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    test::TestStructNullable root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStructNullable"));
    serializer->enterStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    serializer->enterInt32(MetaField{ MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0 }, VALUE_INT32);
    serializer->exitStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    serializer->enterStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    serializer->enterString(MetaField{ MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0 }, VALUE_STRING.data(), VALUE_STRING.size());
    serializer->exitStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    serializer->enterUInt32(MetaField{ MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2 }, VALUE_UINT32);

    serializer->finished();

    test::TestStructNullable cmp;
    cmp.struct_int32 = std::make_shared<test::TestInt32>(VALUE_INT32);
    cmp.struct_string.value = VALUE_STRING;
    cmp.last_value = VALUE_UINT32;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testStructNullableNull)
{
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    test::TestStructNullable root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStructNullable"));
    serializer->enterStructNull(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    serializer->enterStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    serializer->enterString(MetaField{ MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0 }, VALUE_STRING.data(), VALUE_STRING.size());
    serializer->exitStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    serializer->enterUInt32(MetaField{ MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2 }, VALUE_UINT32);

    serializer->finished();

    test::TestStructNullable cmp;
    cmp.struct_string.value = VALUE_STRING;
    cmp.last_value = VALUE_UINT32;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testStructNullableNoCallOfEnterStructNull)
{
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    test::TestStructNullable root;
    root.struct_int32 = std::make_shared<test::TestInt32>(-2);
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStructNullable"));
    serializer->enterStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    serializer->enterString(MetaField{ MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0 }, VALUE_STRING.data(), VALUE_STRING.size());
    serializer->exitStruct(MetaField{ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    serializer->enterUInt32(MetaField{ MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2 }, VALUE_UINT32);

    serializer->finished();

    test::TestStructNullable cmp;
    cmp.struct_string.value = VALUE_STRING;
    cmp.last_value = VALUE_UINT32;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnum)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    serializer->enterEnum(MetaField{MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnumStdString)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    serializer->enterEnum(MetaField{MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, {}, 0}, std::string(VALUE.toString()));
    serializer->finished();

    test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnumString)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    serializer->enterEnum(MetaField{MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE.toString().c_str(), VALUE.toString().size());
    serializer->finished();

    test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}



TEST_F(TestSerializerStruct, testVariantEmptyDefault)
{
    test::TestVariant root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    serializer->finished();

    test::TestVariant cmp = { };
    ASSERT_EQ(root == cmp, true);
}


TEST_F(TestSerializerStruct, testVariantEmpty)
{
    test::TestVariant root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    serializer->enterStruct(*m_fieldValue);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
    serializer->exitStruct(*m_fieldValue);
    serializer->finished();

    test::TestVariant cmp = { };
    ASSERT_EQ(root == cmp, true);
}


TEST_F(TestSerializerStruct, testVariantString)
{
    std::string VALUE_STRING = "123";

    test::TestVariant root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    serializer->enterStruct(*m_fieldValue);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    serializer->enterString(*m_fieldString, VALUE_STRING.data(), VALUE_STRING.size());
    serializer->exitStruct(*m_fieldValue);
    serializer->finished();

    test::TestVariant cmp{ VALUE_STRING,  0, {} };
    ASSERT_EQ(root == cmp, true);
}


TEST_F(TestSerializerStruct, testVariantStruct)
{
    test::TestVariant root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    // VariantStruct{ {"value", VariantStruct{
    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    serializer->enterStruct(*m_fieldValue);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    serializer->enterArrayStruct(*m_fieldStruct);
    // {"key1", VariantList{
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "key1", 4);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTLIST);
    serializer->enterArrayStruct(*m_fieldList);
    // 2
    serializer->enterStruct(*m_fieldListWithoutArray);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
    serializer->enterInt32(*m_fieldInt32, 2);
    serializer->exitStruct(*m_fieldListWithoutArray);
    // , std::string("Hello")
    serializer->enterStruct(*m_fieldListWithoutArray);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    serializer->enterString(*m_fieldString, "Hello", 5);
    serializer->exitStruct(*m_fieldListWithoutArray);
    // }
    serializer->exitArrayStruct(*m_fieldList);
    serializer->exitStruct(*m_fieldListWithoutArray);

    // {"key2", VariantStruct{
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "key2", 4);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    serializer->enterArrayStruct(*m_fieldStruct);
    // {"a", 3},
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "a", 1);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
    serializer->enterInt32(*m_fieldInt32, 3);
    serializer->exitStruct(*m_fieldStructWithoutArray);
    // {"b", std::string("Hi")}
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "b", 1);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    serializer->enterString(*m_fieldString, "Hi", 2);
    serializer->exitStruct(*m_fieldStructWithoutArray);
    // }
    serializer->exitArrayStruct(*m_fieldStruct);
    serializer->exitStruct(*m_fieldStructWithoutArray);

    // {
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "key3", 4);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
    serializer->exitStruct(*m_fieldStructWithoutArray);
    // }}
    serializer->exitArrayStruct(*m_fieldStruct);
    serializer->exitStruct(*m_fieldValue);
    serializer->finished();

    test::TestVariant cmp{ VariantStruct{ {"key1", VariantList{std::int32_t(2), std::string("Hello")}}, 
                                          {"key2", VariantStruct{{"a", std::int32_t(3)}, {"b", std::string("Hi")}}}, 
                                          {"key3", Variant()} 
                                        }, 0, {} };
    ASSERT_EQ(root == cmp, true);
}


TEST_F(TestSerializerStruct, testVariantStruct2)
{
    test::TestVariant root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    // VariantStruct{ {"value", VariantStruct{
    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    serializer->enterStruct(*m_fieldValue);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    serializer->enterArrayStruct(*m_fieldStruct);
    // {"key1", VariantList{
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "key1", 4);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTLIST);
    serializer->enterArrayStruct(*m_fieldList);
    // 2
    serializer->enterStruct(*m_fieldListWithoutArray);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
    serializer->enterInt32(*m_fieldInt32, 2);
    serializer->exitStruct(*m_fieldListWithoutArray);
    // , std::string("Hello")
    serializer->enterStruct(*m_fieldListWithoutArray);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    serializer->enterString(*m_fieldString, "Hello", 5);
    serializer->exitStruct(*m_fieldListWithoutArray);
    // }
    serializer->exitArrayStruct(*m_fieldList);
    serializer->exitStruct(*m_fieldStructWithoutArray);

    // {"key2", VariantStruct{
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "key2", 4);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    serializer->enterArrayStruct(*m_fieldStruct);
    // {"a", 3},
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "a", 1);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
    serializer->enterInt32(*m_fieldInt32, 3);
    serializer->exitStruct(*m_fieldStructWithoutArray);
    // {"b", std::string("Hi")}
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "b", 1);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    serializer->enterString(*m_fieldString, "Hi", 2);
    serializer->exitStruct(*m_fieldStructWithoutArray);
    // }
    serializer->exitArrayStruct(*m_fieldStruct);
    serializer->exitStruct(*m_fieldStructWithoutArray);

    // {
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "key3", 4);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
    serializer->exitStruct(*m_fieldStructWithoutArray);
    // }}
    serializer->exitArrayStruct(*m_fieldStruct);
    serializer->exitStruct(*m_fieldValue);


    // VariantStruct{ {"value2", VariantStruct{
    serializer->enterStruct(*m_fieldValue2);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    serializer->enterArrayStruct(*m_fieldStruct);
    // {"key1", "Hello"}
    serializer->enterStruct(*m_fieldStructWithoutArray);
    serializer->enterString(*m_fieldName, "key1", 4);
    serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    serializer->enterString(*m_fieldString, "Hello", 5);
    serializer->exitStruct(*m_fieldStructWithoutArray);
    serializer->exitArrayStruct(*m_fieldStruct);
    serializer->exitStruct(*m_fieldValue2);

    // {"valueInt32", 5}    
    serializer->enterInt32(*m_fieldValueInt32, 5);

    serializer->finished();

    test::TestVariant cmp{ VariantStruct{ {"key1", VariantList{std::int32_t(2), std::string("Hello")}}, 
                                          {"key2", VariantStruct{{"a", std::int32_t(3)}, {"b", std::string("Hi")}}}, 
                                          {"key3", Variant()} }, 
                                          std::int32_t(5), VariantStruct{ {"key1", std::string("Hello")} } };
    ASSERT_EQ(root == cmp, true);
}


TEST_F(TestSerializerStruct, testJson)
{
    static const Variant VALUE = VariantStruct{ {"a", static_cast<std::uint32_t>(3)} };

    test::TestJson root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestJson"));
    serializer->enterJsonVariant(MetaField{ MetaTypeId::TYPE_JSON, "", "value", "", 0, {}, 0 }, VALUE);
    serializer->finished();

    test::TestJson cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayBool root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBool"));
    serializer->enterArrayBool(MetaField{MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestArrayBool cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}



TEST_F(TestSerializerStruct, testArrayInt8)
{
    static const std::int8_t VALUE1 = -1;
    static const std::int8_t VALUE2 = 0;
    static const std::int8_t VALUE3 = 1;
    static const std::vector<std::int8_t> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayInt8 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt8"));
    serializer->enterArrayInt8(MetaField{MetaTypeId::TYPE_ARRAY_INT8, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayInt8 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayInt16)
{
    static const std::int16_t VALUE1 = -1;
    static const std::int16_t VALUE2 = 0;
    static const std::int16_t VALUE3 = 1;
    static const std::vector<std::int16_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    test::TestArrayInt16 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt16"));
    serializer->enterArrayInt16(MetaField{ MetaTypeId::TYPE_ARRAY_INT16, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayInt16 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayUInt16)
{
    static const std::uint16_t VALUE1 = 0xfffe;
    static const std::uint16_t VALUE2 = 0;
    static const std::uint16_t VALUE3 = 1;
    static const std::vector<std::uint16_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    test::TestArrayUInt16 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt16"));
    serializer->enterArrayUInt16(MetaField{ MetaTypeId::TYPE_ARRAY_UINT16, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayUInt16 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    test::TestArrayInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt32"));
    serializer->enterArrayInt32(MetaField{ MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    test::TestArrayUInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt32"));
    serializer->enterArrayUInt32(MetaField{ MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayUInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt64"));
    serializer->enterArrayInt64(MetaField{MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayUInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt64"));
    serializer->enterArrayUInt64(MetaField{MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayUInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayFloat)
{
    static const float VALUE1 = -1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayFloat root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayFloat"));
    serializer->enterArrayFloat(MetaField{MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayFloat cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayDouble root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayDouble"));
    serializer->enterArrayDouble(MetaField{MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayDouble cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayString root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayString"));
    serializer->enterArrayString(MetaField{MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestArrayString cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayBytes root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBytes"));
    serializer->enterArrayBytes(MetaField{MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0, {}, 0}, VALUE);
    serializer->finished();

    test::TestArrayBytes cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}




TEST_F(TestSerializerStruct, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;
    static const std::uint32_t LAST_VALUE = 5;

    test::TestArrayStruct root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayStruct"));
    serializer->enterArrayStruct(MetaField{MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, {}, 0});

    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});
    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    serializer->enterInt32(MetaField{MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE1_INT32);
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    serializer->enterString(MetaField{MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    serializer->enterUInt32(MetaField{MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE1_UINT32);
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});

    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});
    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    serializer->enterInt32(MetaField{MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE2_INT32);
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    serializer->enterString(MetaField{MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    serializer->enterUInt32(MetaField{MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE2_UINT32);
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});

    serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});
    serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});

    serializer->exitArrayStruct(MetaField{MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, {}, 0});
    serializer->enterUInt32(MetaField{MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 1}, LAST_VALUE);

    serializer->finished();

    test::TestArrayStruct cmp = {{
                                          {{VALUE1_INT32}, {VALUE1_STRING}, VALUE1_UINT32},
                                          {{VALUE2_INT32}, {VALUE2_STRING}, VALUE2_UINT32},
                                          {}
    }, LAST_VALUE};
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayEnum)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo::Enum)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    test::TestArrayEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    serializer->enterArrayEnum(MetaField{MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayEnum cmp = {{VALUE1, VALUE2, VALUE3, VALUE2}};
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayEnumString)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const std::vector<std::string> VALUEString = {VALUE1.toString(), VALUE2.toString(), VALUE3.toString(), "BlaBla"};

    test::TestArrayEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    serializer->enterArrayEnum(MetaField{MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUEString);
    serializer->finished();

    test::TestArrayEnum cmp = {{VALUE1, VALUE2, VALUE3, VALUE2}};
    ASSERT_EQ(root, cmp);
}


