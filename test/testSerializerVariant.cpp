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


#include "finalmq/serializevariant/SerializerVariant.h"
#include "finalmq/serializevariant/VarValueToVariant.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/metadataserialize/variant.fmq.h"
#include "matchers.h"


using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;

using namespace finalmq;


class TestSerializerVariant : public testing::Test
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

        m_serializer = std::make_unique<SerializerVariant>(m_root);
        m_serializerEnumAsInt = std::make_unique<SerializerVariant>(m_root, false);
    }

    virtual void TearDown()
    {
    }

    Variant                         m_root;
    std::unique_ptr<IParserVisitor> m_serializer;
    std::unique_ptr<IParserVisitor> m_serializerEnumAsInt;

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




TEST_F(TestSerializerVariant, testBool)
{
    static const bool VALUE = true;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBool"));
    m_serializer->enterBool(MetaField{MetaTypeId::TYPE_BOOL, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testInt8)
{
    static const std::int8_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt8"));
    m_serializer->enterInt8(MetaField{MetaTypeId::TYPE_INT8, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testUInt8)
{
    static const std::uint8_t VALUE = 0xFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt8"));
    m_serializer->enterUInt8(MetaField{MetaTypeId::TYPE_UINT8, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testInt16)
{
    static const std::int16_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt16"));
    m_serializer->enterInt16({ MetaTypeId::TYPE_INT16, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testUInt16)
{
    static const std::uint16_t VALUE = 0xFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt16"));
    m_serializer->enterUInt16({ MetaTypeId::TYPE_UINT16, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testInt32)
{
    static const std::int32_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt32"));
    m_serializer->enterInt32({ MetaTypeId::TYPE_INT32, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt32"));
    m_serializer->enterUInt32({ MetaTypeId::TYPE_UINT32, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testInt64)
{
    static const std::int64_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt64"));
    m_serializer->enterInt64(MetaField{MetaTypeId::TYPE_INT64, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt64"));
    m_serializer->enterUInt64(MetaField{MetaTypeId::TYPE_UINT64, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}



TEST_F(TestSerializerVariant, testFloat)
{
    static const float VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestFloat"));
    m_serializer->enterFloat(MetaField{MetaTypeId::TYPE_FLOAT, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testDouble)
{
    static const double VALUE = -2.1;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestDouble"));
    m_serializer->enterDouble(MetaField{MetaTypeId::TYPE_DOUBLE, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testString)
{
    static const std::string VALUE = "Hello World";

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestString"));
    m_serializer->enterString(MetaField{MetaTypeId::TYPE_STRING, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBytes"));
    m_serializer->enterBytes(MetaField{MetaTypeId::TYPE_BYTES, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    m_serializer->enterStruct({ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    m_serializer->enterInt32({ MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0 }, VALUE_INT32);
    m_serializer->exitStruct({ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    m_serializer->enterStruct({ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    m_serializer->enterString({ MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0 }, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct({ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    m_serializer->enterUInt32({ MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2 }, VALUE_UINT32);

    m_serializer->finished();

    Variant VALUE = VariantStruct{ {"struct_int32", VariantStruct{ {"value", VALUE_INT32} }}, {"struct_string", VariantStruct{ {"value", VALUE_STRING} }},  {"last_value", VALUE_UINT32} };
    ASSERT_EQ(m_root == VALUE, true);
}

TEST_F(TestSerializerVariant, testStructDefaultInStruct)
{
    static const std::int32_t VALUE_INT32_DEFAULT = 0;
    static const std::string VALUE_STRING_DEFAULT = "";
    static const std::uint32_t VALUE_UINT32_DEFAULT = 0;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    m_serializer->enterStruct({ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    m_serializer->exitStruct({ MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0 });
    m_serializer->enterStruct({ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });
    m_serializer->exitStruct({ MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1 });

    m_serializer->finished();

    Variant VALUE = VariantStruct{ {"struct_int32", VariantStruct{ {"value", VALUE_INT32_DEFAULT} }}, {"struct_string", VariantStruct{ {"value", VALUE_STRING_DEFAULT} }},  {"last_value", VALUE_UINT32_DEFAULT} };
    ASSERT_EQ(m_root == VALUE, true);
}


TEST_F(TestSerializerVariant, testStructDefault)
{
    static const std::int32_t VALUE_INT32_DEFAULT = 0;
    static const std::string VALUE_STRING_DEFAULT = "";
    static const std::uint32_t VALUE_UINT32_DEFAULT = 0;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    m_serializer->finished();

    Variant VALUE = VariantStruct{ {"struct_int32", VariantStruct{ {"value", VALUE_INT32_DEFAULT} }}, {"struct_string", VariantStruct{ {"value", VALUE_STRING_DEFAULT} }},  {"last_value", VALUE_UINT32_DEFAULT} };
    ASSERT_EQ(m_root == VALUE, true);
}


TEST_F(TestSerializerVariant, testEnum)
{
    static const std::int32_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializer->enterEnum({ MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", std::string("FOO_HELLO")} });
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testEnumAlias)
{
    static const std::int32_t VALUE = 1;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializer->enterEnum({ MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", std::string("world2")} });
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testEnumAsInt)
{
    static const std::int32_t VALUE = -2;

    m_serializerEnumAsInt->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializerEnumAsInt->enterEnum(MetaField{MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE);
    m_serializerEnumAsInt->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}



TEST_F(TestSerializerVariant, testVariantEmptyDefault)
{

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->finished();

    Variant cmp = VariantStruct{ {"value", Variant()}, {"valueInt32", 0},  {"value2", {}} };
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testVariantEmpty)
{

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
    m_serializer->exitStruct(*m_fieldValue);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", Variant()}, {"valueInt32", 0},  {"value2", {}} });
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testVariantString)
{
    std::string VALUE_STRING = "123";

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    m_serializer->enterString(*m_fieldString, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct(*m_fieldValue);
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE_STRING }, {"valueInt32", 0},  {"value2", {}} });
    ASSERT_EQ(m_root == cmp, true);
}



TEST_F(TestSerializerVariant, testVariantStruct)
{
    // VariantStruct{ {"value", VariantStruct{
    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    m_serializer->enterArrayStruct(*m_fieldStruct);
        // {"key1", VariantList{
        m_serializer->enterStruct(*m_fieldStructWithoutArray);
        m_serializer->enterString(*m_fieldName, "key1", 4);
        m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTLIST);
        m_serializer->enterArrayStruct(*m_fieldList);
            // 2
            m_serializer->enterStruct(*m_fieldListWithoutArray);
            m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
            m_serializer->enterInt32(*m_fieldInt32, 2);
            m_serializer->exitStruct(*m_fieldListWithoutArray);
            // , std::string("Hello")
            m_serializer->enterStruct(*m_fieldListWithoutArray);
            m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
            m_serializer->enterString(*m_fieldString, "Hello", 5);
            m_serializer->exitStruct(*m_fieldListWithoutArray);
        // }
        m_serializer->exitArrayStruct(*m_fieldList);
        m_serializer->exitStruct(*m_fieldStructWithoutArray);

        // {"key2", VariantStruct{
        m_serializer->enterStruct(*m_fieldStructWithoutArray);
        m_serializer->enterString(*m_fieldName, "key2", 4);
        m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
        m_serializer->enterArrayStruct(*m_fieldStruct);
            // {"a", 3},
            m_serializer->enterStruct(*m_fieldStructWithoutArray);
            m_serializer->enterString(*m_fieldName, "a", 1);
            m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
            m_serializer->enterInt32(*m_fieldInt32, 3);
            m_serializer->exitStruct(*m_fieldStructWithoutArray);
            // {"b", std::string("Hi")}
            m_serializer->enterStruct(*m_fieldStructWithoutArray);
            m_serializer->enterString(*m_fieldName, "b", 1);
            m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
            m_serializer->enterString(*m_fieldString, "Hi", 2);
            m_serializer->exitStruct(*m_fieldStructWithoutArray);
        // }
        m_serializer->exitArrayStruct(*m_fieldStruct);
        m_serializer->exitStruct(*m_fieldStructWithoutArray);

        // {
        m_serializer->enterStruct(*m_fieldStructWithoutArray);
        m_serializer->enterString(*m_fieldName, "key3", 4);
        m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
        m_serializer->exitStruct(*m_fieldStructWithoutArray);
    // }}
    m_serializer->exitArrayStruct(*m_fieldStruct);
    m_serializer->exitStruct(*m_fieldValue);
    m_serializer->finished();

    Variant cmp = VariantStruct{ {"value", VariantStruct{ {"key1", VariantList{std::int32_t(2), std::string("Hello")}}, {"key2", VariantStruct{{"a", std::int32_t(3)}, {"b", std::string("Hi")}}}, {"key3", Variant()} } }, {"valueInt32", 0},  {"value2", {}} };
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testVariantStruct2)
{
    // VariantStruct{ {"value", VariantStruct{
    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    m_serializer->enterArrayStruct(*m_fieldStruct);
    // {"key1", VariantList{
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterString(*m_fieldName, "key1", 4);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTLIST);
    m_serializer->enterArrayStruct(*m_fieldList);
    // 2
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
    m_serializer->enterInt32(*m_fieldInt32, 2);
    m_serializer->exitStruct(*m_fieldListWithoutArray);
    // , std::string("Hello")
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    m_serializer->enterString(*m_fieldString, "Hello", 5);
    m_serializer->exitStruct(*m_fieldListWithoutArray);
    // }
    m_serializer->exitArrayStruct(*m_fieldList);
    m_serializer->exitStruct(*m_fieldListWithoutArray);

    // {"key2", VariantStruct{
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterString(*m_fieldName, "key2", 4);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    m_serializer->enterArrayStruct(*m_fieldStruct);
    // {"a", 3},
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterString(*m_fieldName, "a", 1);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_INT32);
    m_serializer->enterInt32(*m_fieldInt32, 3);
    m_serializer->exitStruct(*m_fieldListWithoutArray);
    // {"b", std::string("Hi")}
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterString(*m_fieldName, "b", 1);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    m_serializer->enterString(*m_fieldString, "Hi", 2);
    m_serializer->exitStruct(*m_fieldListWithoutArray);
    // }
    m_serializer->exitArrayStruct(*m_fieldList);
    m_serializer->exitStruct(*m_fieldListWithoutArray);

    // {
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterString(*m_fieldName, "key3", 4);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
    m_serializer->exitStruct(*m_fieldListWithoutArray);
    // }}
    m_serializer->exitArrayStruct(*m_fieldList);
    m_serializer->exitStruct(*m_fieldValue);


    // VariantStruct{ {"value2", VariantStruct{
    m_serializer->enterStruct(*m_fieldValue2);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    m_serializer->enterArrayStruct(*m_fieldStruct);
    // {"key1", "Hello"}
    m_serializer->enterStruct(*m_fieldListWithoutArray);
    m_serializer->enterString(*m_fieldName, "key1", 4);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    m_serializer->enterString(*m_fieldString, "Hello", 5);
    m_serializer->exitStruct(*m_fieldListWithoutArray);
    m_serializer->exitArrayStruct(*m_fieldList);
    m_serializer->exitStruct(*m_fieldValue2);

    // {"valueInt32", 5}    
    m_serializer->enterInt32(*m_fieldValueInt32, 5);

    m_serializer->finished();

    Variant cmp = VariantStruct{ {"value", VariantStruct{ {"key1", VariantList{std::int32_t(2), std::string("Hello")}}, {"key2", VariantStruct{{"a", std::int32_t(3)}, {"b", std::string("Hi")}}}, {"key3", Variant()} } }, {"value2", VariantStruct{ {"key1", std::string("Hello")} }}, {"valueInt32", std::int32_t(5)} };
    ASSERT_EQ(m_root == cmp, true);
}




TEST_F(TestSerializerVariant, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBool"));
    m_serializer->enterArrayBool(MetaField{MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testArrayInt8)
{
    static const std::int8_t VALUE1 = -1;
    static const std::int8_t VALUE2 = 0;
    static const std::int8_t VALUE3 = 1;
    static const std::vector<std::int8_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt8"));
    m_serializer->enterArrayInt8(MetaField{MetaTypeId::TYPE_ARRAY_INT8, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayInt16)
{
    static const std::int16_t VALUE1 = -1;
    static const std::int16_t VALUE2 = 0;
    static const std::int16_t VALUE3 = 1;
    static const std::vector<std::int16_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt16"));
    m_serializer->enterArrayInt16({ MetaTypeId::TYPE_ARRAY_INT16, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayUInt16)
{
    static const std::uint16_t VALUE1 = 0xfffe;
    static const std::uint16_t VALUE2 = 0;
    static const std::uint16_t VALUE3 = 1;
    static const std::vector<std::uint16_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt16"));
    m_serializer->enterArrayUInt16({ MetaTypeId::TYPE_ARRAY_UINT16, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt32"));
    m_serializer->enterArrayInt32({ MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt32"));
    m_serializer->enterArrayUInt32({ MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({ {"value", VALUE} });
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt64"));
    m_serializer->enterArrayInt64(MetaField{MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt64"));
    m_serializer->enterArrayUInt64(MetaField{MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayFloat)
{
    static const float VALUE1 = -1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayFloat"));
    m_serializer->enterArrayFloat(MetaField{MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayDouble"));
    m_serializer->enterArrayDouble(MetaField{MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayString"));
    m_serializer->enterArrayString(MetaField{MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBytes"));
    m_serializer->enterArrayBytes(MetaField{MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;
    static const std::uint32_t LAST_VALUE = 5;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayStruct"));
    m_serializer->enterArrayStruct(MetaField{MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestMessageStruct", "value", "desc", 0, {}, 0});

    m_serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0, {}, 0});
    m_serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterInt32(MetaField{MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE1_INT32);
    m_serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterString(MetaField{MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    m_serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterUInt32(MetaField{MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE1_UINT32);
    m_serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0, {}, 0});

    m_serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0, {}, 0});
    m_serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterInt32(MetaField{MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE2_INT32);
    m_serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterString(MetaField{MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    m_serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterUInt32(MetaField{MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE2_UINT32);
    m_serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0, {}, 0});

    m_serializer->enterStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0, {}, 0});
    m_serializer->exitStruct(MetaField{MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0, {}, 0});

    m_serializer->exitArrayStruct(MetaField{MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestMessageStruct", "value", "desc", 0, {}, 0});
    m_serializer->enterUInt32(MetaField{MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 1}, LAST_VALUE);

    m_serializer->finished();

    Variant VALUE = VariantStruct{ {"value", VariantList{
        VariantStruct{ {"struct_int32", VariantStruct{ {"value", VALUE1_INT32} }}, {"struct_string", VariantStruct{ {"value", VALUE1_STRING} }},  {"last_value", VALUE1_UINT32} },
        VariantStruct{ {"struct_int32", VariantStruct{ {"value", VALUE2_INT32} }}, {"struct_string", VariantStruct{ {"value", VALUE2_STRING} }},  {"last_value", VALUE2_UINT32} },
        VariantStruct{}
    }}, {"last_value", LAST_VALUE} };
    ASSERT_EQ(m_root == VALUE, true);
}

TEST_F(TestSerializerVariant, testArrayEnum)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::int32_t VALUE4 = 123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    m_serializer->enterArrayEnum(MetaField{MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", std::vector<std::string>({"FOO_HELLO","FOO_WORLD","world2","FOO_WORLD"})}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testArrayEnumAsInt)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::int32_t VALUE4 = 123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializerEnumAsInt->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    m_serializerEnumAsInt->enterArrayEnum(MetaField{MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializerEnumAsInt->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


