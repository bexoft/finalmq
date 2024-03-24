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


#include "finalmq/serializeqt/SerializerQt.h"
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



class TestSerializerQt : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        static const int MAX_BLOCK_SIZE = 2000;

        m_data.resize(MAX_BLOCK_SIZE);
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE, _)).Times(1).WillOnce(Return((char*)m_data.data()));
        EXPECT_CALL(m_mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&m_data, &String::resize));
        EXPECT_CALL(m_mockBuffer, getRemainingSize()).WillRepeatedly(Return(0));

        m_serializer = std::make_unique<SerializerQt>(m_mockBuffer, SerializerQt::Mode::NONE, MAX_BLOCK_SIZE);

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




TEST_F(TestSerializerQt, testBool)
{
    static const bool VALUE = true;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBool"));
    m_serializer->enterBool(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testInt8)
{
    static const std::int8_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt8"));
    m_serializer->enterInt8(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testUInt8)
{
    static const std::uint8_t VALUE = 0xFE;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt8"));
    m_serializer->enterUInt8(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testInt16)
{
    static const std::int16_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt16"));
    m_serializer->enterInt16(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testUInt16)
{
    static const std::uint16_t VALUE = 0xFFFE;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt16"));
    m_serializer->enterUInt16(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testInt32)
{
    static const std::int32_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt32"));
    m_serializer->enterInt32(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt32"));
    m_serializer->enterUInt32(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testInt64)
{
    static const std::int64_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt64"));
    m_serializer->enterInt64(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 56));
    data.push_back(static_cast<char>(VALUE >> 48));
    data.push_back(static_cast<char>(VALUE >> 40));
    data.push_back(static_cast<char>(VALUE >> 32));
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt64"));
    m_serializer->enterUInt64(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 56));
    data.push_back(static_cast<char>(VALUE >> 48));
    data.push_back(static_cast<char>(VALUE >> 40));
    data.push_back(static_cast<char>(VALUE >> 32));
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    EXPECT_EQ(m_data, data);
}



TEST_F(TestSerializerQt, testFloat)
{
    static const float VALUE = -2.1f;
    union
    {
        std::uint64_t v;
        double d;
    } u;
    u.d = static_cast<double>(VALUE);

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestFloat"));
    m_serializer->enterFloat(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testDouble)
{
    static const double VALUE = -2.1;
    union
    {
        std::uint64_t v;
        double d;
    } u;
    u.d = VALUE;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestDouble"));
    m_serializer->enterDouble(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testString)
{
    static const std::string VALUE = "Hello World";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestString"));
    m_serializer->enterString(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(11 * 2);
    data.push_back(0);
    data.push_back('H');
    data.push_back(0);
    data.push_back('e');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back(' ');
    data.push_back(0);
    data.push_back('W');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back('r');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('d');

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBytes"));
    m_serializer->enterBytes(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(7);
    data.push_back('H');
    data.push_back('e');
    data.push_back('l');
    data.push_back(0);
    data.push_back(13);
    data.push_back('l');
    data.push_back('o');

    EXPECT_EQ(m_data, data);
}



TEST_F(TestSerializerQt, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    const MetaField* fieldLastValue = MetaDataGlobal::instance().getField("test.TestStruct", "last_value");
    ASSERT_NE(fieldStructInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);
    ASSERT_NE(fieldLastValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    m_serializer->enterStruct(*fieldStructInt32);
    m_serializer->enterInt32(*fieldInt32, VALUE_INT32);
    m_serializer->exitStruct(*fieldStructInt32);
    m_serializer->enterStruct(*fieldStructString);
    m_serializer->enterString(*fieldString, std::move(VALUE_STRING));
    m_serializer->exitStruct(*fieldStructString);
    m_serializer->enterUInt32(*fieldLastValue, VALUE_UINT32);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE_INT32 >> 24));
    data.push_back(static_cast<char>(VALUE_INT32 >> 16));
    data.push_back(static_cast<char>(VALUE_INT32 >> 8));
    data.push_back(static_cast<char>(VALUE_INT32 >> 0));
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(11 * 2);
    data.push_back(0);
    data.push_back('H');
    data.push_back(0);
    data.push_back('e');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back(' ');
    data.push_back(0);
    data.push_back('W');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back('r');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('d');
    data.push_back(static_cast<char>(VALUE_UINT32 >> 24));
    data.push_back(static_cast<char>(VALUE_UINT32 >> 16));
    data.push_back(static_cast<char>(VALUE_UINT32 >> 8));
    data.push_back(static_cast<char>(VALUE_UINT32 >> 0));

    EXPECT_EQ(m_data, data);
}



TEST_F(TestSerializerQt, testEnum)
{
    static const std::int32_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializer->enterEnum(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testVariantEmpty)
{
    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_NONE);
    m_serializer->exitStruct(*m_fieldValue);
    m_serializer->finished();

    std::string data;
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 24));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 16));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 8));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 0));
    data.push_back(0);

    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);

    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 24));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 16));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 8));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 0));
    data.push_back(0);

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testVariantString)
{
    std::string VALUE_STRING = "123";

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializer->enterStruct(*m_fieldValue);
    m_serializer->enterInt32(*m_fieldIndex, VarValueType2Index::VARVALUETYPE_STRING);
    m_serializer->enterString(*m_fieldString, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct(*m_fieldValue);
    m_serializer->finished();

    std::string data;
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_STRING >> 24));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_STRING >> 16));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_STRING >> 8));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_STRING >> 0));
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(3 * 2);
    data.push_back(0);
    data.push_back('1');
    data.push_back(0);
    data.push_back('2');
    data.push_back(0);
    data.push_back('3');

    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);

    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 24));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 16));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 8));
    data.push_back(static_cast<char>(VarValueType2Index::VARVALUETYPE_NONE >> 0));
    data.push_back(0);

    EXPECT_EQ(m_data, data);
}



TEST_F(TestSerializerQt, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = false;
    static const bool VALUE4 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBool"));
    m_serializer->enterArrayBool(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(0x09);

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testArrayInt8)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 22;
    static const std::vector<std::int8_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt8", "value");
    ASSERT_NE(fieldValue, nullptr);


    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt8"));
    m_serializer->enterArrayInt8(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1));
    data.push_back(static_cast<char>(VALUE2));
    data.push_back(static_cast<char>(VALUE3));
    data.push_back(static_cast<char>(VALUE4));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayInt16)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;
    static const std::vector<std::int16_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt16"));
    m_serializer->enterArrayInt16(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1 >> 8));
    data.push_back(static_cast<char>(VALUE1 >> 0));
    data.push_back(static_cast<char>(VALUE2 >> 8));
    data.push_back(static_cast<char>(VALUE2 >> 0));
    data.push_back(static_cast<char>(VALUE3 >> 8));
    data.push_back(static_cast<char>(VALUE3 >> 0));
    data.push_back(static_cast<char>(VALUE4 >> 8));
    data.push_back(static_cast<char>(VALUE4 >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayUInt16)
{
    static const std::uint32_t VALUE1 = 0xFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;
    static const std::vector<std::uint16_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt16"));
    m_serializer->enterArrayUInt16(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1 >> 8));
    data.push_back(static_cast<char>(VALUE1 >> 0));
    data.push_back(static_cast<char>(VALUE2 >> 8));
    data.push_back(static_cast<char>(VALUE2 >> 0));
    data.push_back(static_cast<char>(VALUE3 >> 8));
    data.push_back(static_cast<char>(VALUE3 >> 0));
    data.push_back(static_cast<char>(VALUE4 >> 8));
    data.push_back(static_cast<char>(VALUE4 >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;
    static const std::vector<std::int32_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt32"));
    m_serializer->enterArrayInt32(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1 >> 24));
    data.push_back(static_cast<char>(VALUE1 >> 16));
    data.push_back(static_cast<char>(VALUE1 >> 8));
    data.push_back(static_cast<char>(VALUE1 >> 0));
    data.push_back(static_cast<char>(VALUE2 >> 24));
    data.push_back(static_cast<char>(VALUE2 >> 16));
    data.push_back(static_cast<char>(VALUE2 >> 8));
    data.push_back(static_cast<char>(VALUE2 >> 0));
    data.push_back(static_cast<char>(VALUE3 >> 24));
    data.push_back(static_cast<char>(VALUE3 >> 16));
    data.push_back(static_cast<char>(VALUE3 >> 8));
    data.push_back(static_cast<char>(VALUE3 >> 0));
    data.push_back(static_cast<char>(VALUE4 >> 24));
    data.push_back(static_cast<char>(VALUE4 >> 16));
    data.push_back(static_cast<char>(VALUE4 >> 8));
    data.push_back(static_cast<char>(VALUE4 >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xFFFFFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;
    static const std::vector<std::uint32_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt32"));
    m_serializer->enterArrayUInt32(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1 >> 24));
    data.push_back(static_cast<char>(VALUE1 >> 16));
    data.push_back(static_cast<char>(VALUE1 >> 8));
    data.push_back(static_cast<char>(VALUE1 >> 0));
    data.push_back(static_cast<char>(VALUE2 >> 24));
    data.push_back(static_cast<char>(VALUE2 >> 16));
    data.push_back(static_cast<char>(VALUE2 >> 8));
    data.push_back(static_cast<char>(VALUE2 >> 0));
    data.push_back(static_cast<char>(VALUE3 >> 24));
    data.push_back(static_cast<char>(VALUE3 >> 16));
    data.push_back(static_cast<char>(VALUE3 >> 8));
    data.push_back(static_cast<char>(VALUE3 >> 0));
    data.push_back(static_cast<char>(VALUE4 >> 24));
    data.push_back(static_cast<char>(VALUE4 >> 16));
    data.push_back(static_cast<char>(VALUE4 >> 8));
    data.push_back(static_cast<char>(VALUE4 >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayInt64)
{
    static const std::int64_t VALUE1 = -2;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 2;
    static const std::int64_t VALUE4 = 222;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt64"));
    m_serializer->enterArrayInt64(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1 >> 56));
    data.push_back(static_cast<char>(VALUE1 >> 48));
    data.push_back(static_cast<char>(VALUE1 >> 40));
    data.push_back(static_cast<char>(VALUE1 >> 32));
    data.push_back(static_cast<char>(VALUE1 >> 24));
    data.push_back(static_cast<char>(VALUE1 >> 16));
    data.push_back(static_cast<char>(VALUE1 >> 8));
    data.push_back(static_cast<char>(VALUE1 >> 0));
    data.push_back(static_cast<char>(VALUE2 >> 56));
    data.push_back(static_cast<char>(VALUE2 >> 48));
    data.push_back(static_cast<char>(VALUE2 >> 40));
    data.push_back(static_cast<char>(VALUE2 >> 32));
    data.push_back(static_cast<char>(VALUE2 >> 24));
    data.push_back(static_cast<char>(VALUE2 >> 16));
    data.push_back(static_cast<char>(VALUE2 >> 8));
    data.push_back(static_cast<char>(VALUE2 >> 0));
    data.push_back(static_cast<char>(VALUE3 >> 56));
    data.push_back(static_cast<char>(VALUE3 >> 48));
    data.push_back(static_cast<char>(VALUE3 >> 40));
    data.push_back(static_cast<char>(VALUE3 >> 32));
    data.push_back(static_cast<char>(VALUE3 >> 24));
    data.push_back(static_cast<char>(VALUE3 >> 16));
    data.push_back(static_cast<char>(VALUE3 >> 8));
    data.push_back(static_cast<char>(VALUE3 >> 0));
    data.push_back(static_cast<char>(VALUE4 >> 56));
    data.push_back(static_cast<char>(VALUE4 >> 48));
    data.push_back(static_cast<char>(VALUE4 >> 40));
    data.push_back(static_cast<char>(VALUE4 >> 32));
    data.push_back(static_cast<char>(VALUE4 >> 24));
    data.push_back(static_cast<char>(VALUE4 >> 16));
    data.push_back(static_cast<char>(VALUE4 >> 8));
    data.push_back(static_cast<char>(VALUE4 >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xFFFFFFFFFFFFFFFE;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 2;
    static const std::uint64_t VALUE4 = 222;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt64"));
    m_serializer->enterArrayUInt64(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1 >> 56));
    data.push_back(static_cast<char>(VALUE1 >> 48));
    data.push_back(static_cast<char>(VALUE1 >> 40));
    data.push_back(static_cast<char>(VALUE1 >> 32));
    data.push_back(static_cast<char>(VALUE1 >> 24));
    data.push_back(static_cast<char>(VALUE1 >> 16));
    data.push_back(static_cast<char>(VALUE1 >> 8));
    data.push_back(static_cast<char>(VALUE1 >> 0));
    data.push_back(static_cast<char>(VALUE2 >> 56));
    data.push_back(static_cast<char>(VALUE2 >> 48));
    data.push_back(static_cast<char>(VALUE2 >> 40));
    data.push_back(static_cast<char>(VALUE2 >> 32));
    data.push_back(static_cast<char>(VALUE2 >> 24));
    data.push_back(static_cast<char>(VALUE2 >> 16));
    data.push_back(static_cast<char>(VALUE2 >> 8));
    data.push_back(static_cast<char>(VALUE2 >> 0));
    data.push_back(static_cast<char>(VALUE3 >> 56));
    data.push_back(static_cast<char>(VALUE3 >> 48));
    data.push_back(static_cast<char>(VALUE3 >> 40));
    data.push_back(static_cast<char>(VALUE3 >> 32));
    data.push_back(static_cast<char>(VALUE3 >> 24));
    data.push_back(static_cast<char>(VALUE3 >> 16));
    data.push_back(static_cast<char>(VALUE3 >> 8));
    data.push_back(static_cast<char>(VALUE3 >> 0));
    data.push_back(static_cast<char>(VALUE4 >> 56));
    data.push_back(static_cast<char>(VALUE4 >> 48));
    data.push_back(static_cast<char>(VALUE4 >> 40));
    data.push_back(static_cast<char>(VALUE4 >> 32));
    data.push_back(static_cast<char>(VALUE4 >> 24));
    data.push_back(static_cast<char>(VALUE4 >> 16));
    data.push_back(static_cast<char>(VALUE4 >> 8));
    data.push_back(static_cast<char>(VALUE4 >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayFloat)
{
    static const float VALUE1 = -2.1f;
    static const float VALUE2 = 0.0f;
    static const float VALUE3 = 2.1f;
    static const float VALUE4 = 222.1f;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};
    union
    {
        std::uint64_t v;
        double d;
    } u;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayFloat"));
    m_serializer->enterArrayFloat(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    u.d = static_cast<double>(VALUE1);
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));
    u.d = static_cast<double>(VALUE2);
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));
    u.d = static_cast<double>(VALUE3);
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));
    u.d = static_cast<double>(VALUE4);
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayDouble)
{
    static const double VALUE1 = -2.1;
    static const double VALUE2 = 0.0;
    static const double VALUE3 = 2.1;
    static const double VALUE4 = 222.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};
    union
    {
        std::uint64_t v;
        double d;
    } u;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayDouble"));
    m_serializer->enterArrayDouble(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    u.d = VALUE1;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));
    u.d = VALUE2;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));
    u.d = VALUE3;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));
    u.d = VALUE4;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::string VALUE4 = "Foo";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayString", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayString"));
    m_serializer->enterArrayString(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(5 * 2);
    data.push_back(0);
    data.push_back('H');
    data.push_back(0);
    data.push_back('e');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(5 * 2);
    data.push_back(0);
    data.push_back('W');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back('r');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('d');
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(3 * 2);
    data.push_back(0);
    data.push_back('F');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back('o');

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testArrayBytes)
{
    static const Bytes VALUE1 = { 'H', 'e', '\0', 'l', 'o' };
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = { 'W', 'o', '\n', '\0', 'd' };
    static const Bytes VALUE4 = { 'F', '\t', '\0', 123, 12 };
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBytes"));
    m_serializer->enterArrayBytes(*fieldValue, VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(5);
    data.push_back('H');
    data.push_back('e');
    data.push_back(0);
    data.push_back('l');
    data.push_back('o');
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(5);
    data.push_back('W');
    data.push_back('o');
    data.push_back('\n');
    data.push_back(0);
    data.push_back('d');
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(5);
    data.push_back('F');
    data.push_back('\t');
    data.push_back(0);
    data.push_back(123);
    data.push_back(12);

    EXPECT_EQ(m_data, data);
}


TEST_F(TestSerializerQt, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_LAST = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_LAST = 456;
    static const std::uint32_t LAST_VALUE = 5;

    const MetaField* fieldStruct = MetaDataGlobal::instance().getField("test.TestArrayStruct", "value");
    const MetaField* fieldStructWithoutArray = MetaDataGlobal::instance().getArrayField("test.TestArrayStruct", "value");
    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
    const MetaField* fieldUInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "last_value");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    const MetaField* fieldLastValue = MetaDataGlobal::instance().getField("test.TestArrayStruct", "last_value");
    ASSERT_NE(fieldStruct, nullptr);
    ASSERT_NE(fieldStructWithoutArray, nullptr);
    ASSERT_NE(fieldStructInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);
    ASSERT_NE(fieldLastValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayStruct"));
    m_serializer->enterArrayStruct(*fieldStruct);

    m_serializer->enterStruct(*fieldStructWithoutArray);
    m_serializer->enterStruct(*fieldStructInt32);
    m_serializer->enterInt32(*fieldInt32, VALUE1_INT32);
    m_serializer->exitStruct(*fieldStructInt32);
    m_serializer->enterStruct(*fieldStructString);
    m_serializer->enterString(*fieldString, VALUE1_STRING.data(), VALUE1_STRING.size());
    m_serializer->exitStruct(*fieldStructString);
    m_serializer->enterUInt32(*fieldUInt32, VALUE1_LAST);
    m_serializer->exitStruct(*fieldStructWithoutArray);

    m_serializer->enterStruct(*fieldStructWithoutArray);
    m_serializer->enterStruct(*fieldStructInt32);
    m_serializer->enterInt32(*fieldInt32, VALUE2_INT32);
    m_serializer->exitStruct(*fieldStructInt32);
    m_serializer->enterStruct(*fieldStructString);
    m_serializer->enterString(*fieldString, VALUE2_STRING.data(), VALUE2_STRING.size());
    m_serializer->exitStruct(*fieldStructString);
    m_serializer->enterUInt32(*fieldUInt32, VALUE2_LAST);
    m_serializer->exitStruct(*fieldStructWithoutArray);

    m_serializer->exitArrayStruct(*fieldStruct);
    m_serializer->enterUInt32(*fieldLastValue, LAST_VALUE);
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x02);
    data.push_back(static_cast<char>(VALUE1_INT32 >> 24));
    data.push_back(static_cast<char>(VALUE1_INT32 >> 16));
    data.push_back(static_cast<char>(VALUE1_INT32 >> 8));
    data.push_back(static_cast<char>(VALUE1_INT32 >> 0));
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(11 * 2);
    data.push_back(0);
    data.push_back('H');
    data.push_back(0);
    data.push_back('e');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back(' ');
    data.push_back(0);
    data.push_back('W');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back('r');
    data.push_back(0);
    data.push_back('l');
    data.push_back(0);
    data.push_back('d');
    data.push_back(static_cast<char>(VALUE1_LAST >> 24));
    data.push_back(static_cast<char>(VALUE1_LAST >> 16));
    data.push_back(static_cast<char>(VALUE1_LAST >> 8));
    data.push_back(static_cast<char>(VALUE1_LAST >> 0));
    data.push_back(static_cast<char>(VALUE2_INT32 >> 24));
    data.push_back(static_cast<char>(VALUE2_INT32 >> 16));
    data.push_back(static_cast<char>(VALUE2_INT32 >> 8));
    data.push_back(static_cast<char>(VALUE2_INT32 >> 0));
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(3 * 2);
    data.push_back(0);
    data.push_back('f');
    data.push_back(0);
    data.push_back('o');
    data.push_back(0);
    data.push_back('o');
    data.push_back(static_cast<char>(VALUE2_LAST >> 24));
    data.push_back(static_cast<char>(VALUE2_LAST >> 16));
    data.push_back(static_cast<char>(VALUE2_LAST >> 8));
    data.push_back(static_cast<char>(VALUE2_LAST >> 0));
    data.push_back(static_cast<char>(LAST_VALUE >> 24));
    data.push_back(static_cast<char>(LAST_VALUE >> 16));
    data.push_back(static_cast<char>(LAST_VALUE >> 8));
    data.push_back(static_cast<char>(LAST_VALUE >> 0));

    EXPECT_EQ(m_data, data);
}

TEST_F(TestSerializerQt, testArrayEnum)
{
    static const fmq::test::Foo VALUE1 = fmq::test::Foo::FOO_HELLO;
    static const fmq::test::Foo VALUE2 = fmq::test::Foo::FOO_WORLD;
    static const fmq::test::Foo VALUE3 = fmq::test::Foo::FOO_WORLD2;
    static const fmq::test::Foo VALUE4 = (fmq::test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    m_serializer->enterArrayEnum(*fieldValue, VALUE.data(), VALUE.size());
    m_serializer->finished();

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1 >> 24));
    data.push_back(static_cast<char>(VALUE1 >> 16));
    data.push_back(static_cast<char>(VALUE1 >> 8));
    data.push_back(static_cast<char>(VALUE1 >> 0));
    data.push_back(static_cast<char>(VALUE2 >> 24));
    data.push_back(static_cast<char>(VALUE2 >> 16));
    data.push_back(static_cast<char>(VALUE2 >> 8));
    data.push_back(static_cast<char>(VALUE2 >> 0));
    data.push_back(static_cast<char>(VALUE3 >> 24));
    data.push_back(static_cast<char>(VALUE3 >> 16));
    data.push_back(static_cast<char>(VALUE3 >> 8));
    data.push_back(static_cast<char>(VALUE3 >> 0));
    data.push_back(static_cast<char>(VALUE4 >> 24));
    data.push_back(static_cast<char>(VALUE4 >> 16));
    data.push_back(static_cast<char>(VALUE4 >> 8));
    data.push_back(static_cast<char>(VALUE4 >> 0));

    EXPECT_EQ(m_data, data);
}
