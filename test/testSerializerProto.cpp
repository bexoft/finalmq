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


#include "finalmq/serializeproto/SerializerProto.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/serializevariant/VarValueToVariant.h"
#include "MockIZeroCopyBuffer.h"
#include "test.pb.h"
#include "matchers.h"

//#include <thread>
//#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;

using namespace finalmq;


struct String : public std::string
{
    void resize(size_t size)
    {
        std::string::resize(size);
    }
};



class TestSerializerProto : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        static const int MAX_BLOCK_SIZE = 500;

        m_data.resize(MAX_BLOCK_SIZE);
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE, _)).Times(1).WillOnce(Return((char*)m_data.data()));
        EXPECT_CALL(m_mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&m_data, &String::resize));
        EXPECT_CALL(m_mockBuffer, getRemainingSize()).WillRepeatedly(Return(0));

        m_serializer = std::make_unique<SerializerProto>(m_mockBuffer, MAX_BLOCK_SIZE);

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

    String                          m_data;
    MockIZeroCopyBuffer             m_mockBuffer;
    std::unique_ptr<IParserVisitor> m_serializer;

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




TEST_F(TestSerializerProto, testBool)
{
    static const bool VALUE = true;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBool"));
    m_serializer->enterBool(MetaField{MetaTypeId::TYPE_BOOL, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestBool message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testInt8)
{
    static const std::int8_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt8"));
    m_serializer->enterInt8({MetaTypeId::TYPE_INT8, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestInt8 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt8)
{
    static const std::uint8_t VALUE = 0xFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt8"));
    m_serializer->enterUInt8({MetaTypeId::TYPE_UINT8, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestUInt8 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testInt16)
{
    static const std::int16_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt16"));
    m_serializer->enterInt16({ MetaTypeId::TYPE_INT16, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    fmq::test::TestInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt16)
{
    static const std::uint16_t VALUE = 0xFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt16"));
    m_serializer->enterUInt16({ MetaTypeId::TYPE_UINT16, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    fmq::test::TestUInt16 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testInt32)
{
    static const std::int32_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt32"));
    m_serializer->enterInt32({ MetaTypeId::TYPE_INT32, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    fmq::test::TestInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt32"));
    m_serializer->enterUInt32({ MetaTypeId::TYPE_UINT32, "", "value", "", 0, {}, 0 }, VALUE);
    m_serializer->finished();

    fmq::test::TestUInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testInt64)
{
    static const std::int64_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt64"));
    m_serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt64"));
    m_serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestUInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}



TEST_F(TestSerializerProto, testFloat)
{
    static const float VALUE = -2.1f;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestFloat"));
    m_serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestFloat message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testDouble)
{
    static const double VALUE = -2.1;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestDouble"));
    m_serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestDouble message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testString)
{
    static const std::string VALUE = "Hello World";

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestString"));
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestString message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBytes"));
    m_serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestBytes message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    Bytes result(message.value().data(), message.value().data() + message.value().size());
    EXPECT_EQ(result, VALUE);
}



TEST_F(TestSerializerProto, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0}, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE_UINT32);
    m_serializer->finished();

    fmq::test::TestStruct message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE_INT32);
    EXPECT_EQ(message.struct_string().value(), VALUE_STRING);
    EXPECT_EQ(message.last_value(), VALUE_UINT32);
}



TEST_F(TestSerializerProto, testEnum)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializer->enterEnum({MetaTypeId::TYPE_ENUM, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestEnum message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testVariantEmpty)
{
    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
    m_serializer->exitStruct(*m_fieldValue);
    m_serializer->finished();

    fmq::test::TestVariant message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value().index(), finalmq::VarValueType2Index::VARVALUETYPE_NONE);
}


TEST_F(TestSerializerProto, testVariantString)
{
    std::string VALUE_STRING = "123";

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    m_serializer->enterString(*m_fieldString, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct(*m_fieldValue);
    m_serializer->finished();

    fmq::test::TestVariant message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value().index(), finalmq::VarValueType2Index::VARVALUETYPE_STRING);
    EXPECT_EQ(message.value().valstring(), VALUE_STRING);
}


TEST_F(TestSerializerProto, testVariantStruct)
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
    m_serializer->exitStruct(*m_fieldListWithoutArray);

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

    fmq::test::TestVariant message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value().index(), finalmq::VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    EXPECT_EQ(message.value().valstruct().size(), 3);

    EXPECT_EQ(message.value().valstruct().Get(0).name(), "key1");
    EXPECT_EQ(message.value().valstruct().Get(0).index(), finalmq::VarValueType2Index::VARVALUETYPE_VARIANTLIST);
    EXPECT_EQ(message.value().valstruct().Get(0).vallist().size(), 2);
    EXPECT_EQ(message.value().valstruct().Get(0).vallist().Get(0).index(), finalmq::VarValueType2Index::VARVALUETYPE_INT32);
    EXPECT_EQ(message.value().valstruct().Get(0).vallist().Get(0).valint32(), 2);
    EXPECT_EQ(message.value().valstruct().Get(0).vallist().Get(1).index(), finalmq::VarValueType2Index::VARVALUETYPE_STRING);
    EXPECT_EQ(message.value().valstruct().Get(0).vallist().Get(1).valstring(), "Hello");

    EXPECT_EQ(message.value().valstruct().Get(1).name(), "key2");
    EXPECT_EQ(message.value().valstruct().Get(1).index(), finalmq::VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT);
    EXPECT_EQ(message.value().valstruct().Get(1).valstruct().size(), 2);
    EXPECT_EQ(message.value().valstruct().Get(1).valstruct().Get(0).name(), "a");
    EXPECT_EQ(message.value().valstruct().Get(1).valstruct().Get(0).index(), finalmq::VarValueType2Index::VARVALUETYPE_INT32);
    EXPECT_EQ(message.value().valstruct().Get(1).valstruct().Get(0).valint32(), 3);
    EXPECT_EQ(message.value().valstruct().Get(1).valstruct().Get(1).name(), "b");
    EXPECT_EQ(message.value().valstruct().Get(1).valstruct().Get(1).index(), finalmq::VarValueType2Index::VARVALUETYPE_STRING);
    EXPECT_EQ(message.value().valstruct().Get(1).valstruct().Get(1).valstring(), "Hi");

    EXPECT_EQ(message.value().valstruct().Get(2).name(), "key3");
    EXPECT_EQ(message.value().valstruct().Get(2).index(), finalmq::VarValueType2Index::VARVALUETYPE_NONE);

    EXPECT_EQ(message.value2().index(), finalmq::VarValueType2Index::VARVALUETYPE_NONE);

    EXPECT_EQ(message.valueint32(), 0);
}


TEST_F(TestSerializerProto, testJson)
{
    static const std::string VALUE = "{\"a\":3}";

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestJson"));
    m_serializer->enterJsonString({ MetaTypeId::TYPE_JSON, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestJson message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}



TEST_F(TestSerializerProto, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBool"));
    m_serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestArrayBool message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<bool>(message.value().begin(), message.value().end()), VALUE);
}


TEST_F(TestSerializerProto, testArrayInt8)
{
    static const std::int8_t VALUE1 = -1;
    static const std::int8_t VALUE2 = 0;
    static const std::int8_t VALUE3 = 1;
    static const std::vector<std::int8_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt8"));
    m_serializer->enterArrayInt8({MetaTypeId::TYPE_ARRAY_INT8, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayInt8 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int8_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayInt16)
{
    static const std::int16_t VALUE1 = -1;
    static const std::int16_t VALUE2 = 0;
    static const std::int16_t VALUE3 = 1;
    static const std::vector<std::int16_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt16"));
    m_serializer->enterArrayInt16({ MetaTypeId::TYPE_ARRAY_INT16, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayInt16 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int16_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayUInt16)
{
    static const std::uint16_t VALUE1 = 0xfffe;
    static const std::uint16_t VALUE2 = 0;
    static const std::uint16_t VALUE3 = 1;
    static const std::vector<std::uint16_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt16"));
    m_serializer->enterArrayUInt16({ MetaTypeId::TYPE_ARRAY_UINT16, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayUInt16 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::uint16_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt32"));
    m_serializer->enterArrayInt32({ MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int32_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = { VALUE1, VALUE2, VALUE3 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt32"));
    m_serializer->enterArrayUInt32({ MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0, {}, 0 }, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayUInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::uint32_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt64"));
    m_serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int64_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt64"));
    m_serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayUInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::uint64_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayFloat)
{
    static const float VALUE1 = -1.1f;
    static const float VALUE2 = 0.0f;
    static const float VALUE3 = 1.1f;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayFloat"));
    m_serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayFloat message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<float>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayDouble"));
    m_serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayDouble message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<double>(message.value().begin(), message.value().end()), VALUE);
}


TEST_F(TestSerializerProto, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayString"));
    m_serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestArrayString message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::string>(message.value().begin(), message.value().end()), VALUE);
}


TEST_F(TestSerializerProto, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBytes"));
    m_serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0, {}, 0}, VALUE);
    m_serializer->finished();

    fmq::test::TestArrayBytes message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);

    std::vector<Bytes> result;
    std::for_each(message.value().begin(), message.value().end(), [&result] (const std::string& entry) {
        result.emplace_back(entry.data(), entry.data() + entry.size());
    });
    EXPECT_EQ(result, VALUE);
}

TEST_F(TestSerializerProto, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;
    static const std::uint32_t LAST_VALUE = 5;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayStruct"));
    m_serializer->enterArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, {}, 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE1_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE1_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, {}, 0}, VALUE2_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, {}, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, {}, 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, {}, 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 2}, VALUE2_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, {}, 0});

    m_serializer->exitArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, {}, 0});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, {}, 1}, LAST_VALUE);
    m_serializer->finished();

    fmq::test::TestArrayStruct message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value().size(), 3);
    EXPECT_EQ(message.value(0).struct_int32().value(), VALUE1_INT32);
    EXPECT_EQ(message.value(0).struct_string().value(), VALUE1_STRING);
    EXPECT_EQ(message.value(0).last_value(), VALUE1_UINT32);
    EXPECT_EQ(message.value(1).struct_int32().value(), VALUE2_INT32);
    EXPECT_EQ(message.value(1).struct_string().value(), VALUE2_STRING);
    EXPECT_EQ(message.value(1).last_value(), VALUE2_UINT32);
    EXPECT_EQ(message.value(2).struct_int32().value(), 0);
    EXPECT_EQ(message.value(2).struct_string().value(), "");
    EXPECT_EQ(message.value(2).last_value(), 0);
    EXPECT_EQ(message.last_value(), LAST_VALUE);
}

TEST_F(TestSerializerProto, testArrayEnum)
{
    static const fmq::test::Foo VALUE1 = fmq::test::Foo::FOO_HELLO;
    static const fmq::test::Foo VALUE2 = fmq::test::Foo::FOO_WORLD;
    static const fmq::test::Foo VALUE3 = fmq::test::Foo::FOO_WORLD2;
    static const fmq::test::Foo VALUE4 = (fmq::test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    m_serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "", "value", "", 0, {}, 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    fmq::test::TestArrayEnum message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int32_t>(message.value().begin(), message.value().end()), VALUE);
}

