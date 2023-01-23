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


#include "finalmq/serializestruct/ParserStruct.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/metadataserialize/variant.fmq.h"
#include "test.fmq.h"
#include "MockIParserVisitor.h"


using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::ElementsAreArray;

using namespace finalmq;


MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.typeId == metaField.typeId &&
            arg.typeName == metaField.typeName &&
            arg.name == metaField.name);
}


MATCHER_P2(ArrayEq, compareArray, n, "")
{
    return (memcmp(arg, compareArray, n * sizeof(*arg)) == 0);
}



class TestParserStruct : public testing::Test
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
        m_fieldType = structVarVariant->getFieldByName("type");
        m_fieldInt32 = structVarVariant->getFieldByName("valint32");
        m_fieldString = structVarVariant->getFieldByName("valstring");
        m_fieldList = structVarVariant->getFieldByName("vallist");
        m_fieldListWithoutArray = MetaDataGlobal::instance().getArrayField(*m_fieldList);

        ASSERT_NE(m_fieldName, nullptr);
        ASSERT_NE(m_fieldType, nullptr);
        ASSERT_NE(m_fieldInt32, nullptr);
        ASSERT_NE(m_fieldString, nullptr);
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
    const MetaField* m_fieldType = nullptr;
    const MetaField* m_fieldInt32 = nullptr;
    const MetaField* m_fieldString = nullptr;
    const MetaField* m_fieldList = nullptr;
    const MetaField* m_fieldListWithoutArray = nullptr;
};



TEST_F(TestParserStruct, testBool)
{
    static const bool VALUE = true;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestBool root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testInt32)
{
    static const std::int32_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestInt32 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testUInt32)
{
    static const std::uint32_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestUInt32 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testInt64)
{
    static const std::int64_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestInt64 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testUInt64)
{
    static const std::uint64_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestUInt64 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testFloat)
{
    static const float VALUE = -1.1f;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterFloat(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestFloat root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testDouble)
{
    static const double VALUE = -1.1;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestDouble root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testString)
{
    static const std::string VALUE = "Hello World";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestString root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testBytes)
{
    Bytes VALUE = {'H', 'e', 12, 0, 'A'};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestBytes root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_LAST = 12;

    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
    const MetaField* fieldLastValue = MetaDataGlobal::instance().getField("test.TestStruct", "last_value");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldStructInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldLastValue, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), VALUE_LAST)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestStruct root = {
        {VALUE_INT32},
        {VALUE_STRING},
        VALUE_LAST
    };
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testStructNullableNotNull)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_LAST = 12;

    const MetaField* fieldStructNullableInt32 = MetaDataGlobal::instance().getField("test.TestStructNullable", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStructNullable", "struct_string");
    const MetaField* fieldLastValue = MetaDataGlobal::instance().getField("test.TestStructNullable", "last_value");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldStructNullableInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldLastValue, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructNullableInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructNullableInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), VALUE_LAST)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestStructNullable root = {
        std::make_shared<test::TestInt32>(VALUE_INT32),
        {VALUE_STRING},
        VALUE_LAST
    };
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testStructNullableNull)
{
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_LAST = 12;

    const MetaField* fieldStructNullableInt32 = MetaDataGlobal::instance().getField("test.TestStructNullable", "struct_int32");
    const MetaField* fieldStructNullableString = MetaDataGlobal::instance().getField("test.TestStructNullable", "struct_string");
    const MetaField* fieldStructNullableLastValue = MetaDataGlobal::instance().getField("test.TestStructNullable", "last_value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldStructNullableInt32, nullptr);
    ASSERT_NE(fieldStructNullableString, nullptr);
    ASSERT_NE(fieldStructNullableLastValue, nullptr);
    ASSERT_NE(fieldString, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStructNull(MatcherMetaField(*fieldStructNullableInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructNullableString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructNullableString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructNullableLastValue), VALUE_LAST)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestStructNullable root = {
        nullptr,
        {VALUE_STRING},
        VALUE_LAST
    };
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}





TEST_F(TestParserStruct, testEnumAsInt)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), (std::int32_t)VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestEnum root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}




TEST_F(TestParserStruct, testEnumNotAvailableInt)
{
    static const test::Foo VALUE = (test::Foo::Enum)42;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), (std::int32_t)VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestEnum root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}




TEST_F(TestParserStruct, testVariantEmpty)
{
    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue))).Times(1);

        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldValueInt32), 0)).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue2))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue2))).Times(1);

        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestVariant root = { };
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    EXPECT_EQ(res, true);
}


TEST_F(TestParserStruct, testVariantString)
{
    std::string VALUE_STRING = "123";

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRING)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq(VALUE_STRING.data()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue))).Times(1);

        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldValueInt32), 0)).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue2))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue2))).Times(1);

        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestVariant root{ VALUE_STRING,  0, {} };
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    EXPECT_EQ(res, true);
}

TEST_F(TestParserStruct, testVariantStruct)
{
    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        // VariantStruct{ {"value", VariantStruct{
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRUCT)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
        // {"key1", VariantList{
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key1"), 4)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_LIST)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
        // 2
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldInt32), 2)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        // , std::string("Hello")
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRING)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq("Hello"), 5)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        // }
        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);

        // {"key2", VariantStruct{
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key2"), 4)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRUCT)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
        // {"a", 3},
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("a"), 1)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldInt32), 3)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        // {"b", std::string("Hi")}
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("b"), 1)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRING)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq("Hi"), 2)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        // }
        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);

        // {
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key3"), 4)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
        // }}
        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue))).Times(1);

        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldValueInt32), 0)).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue2))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue2))).Times(1);

        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestVariant root{ VariantStruct{ {"key1", VariantList{std::int32_t(2), std::string("Hello")}}, {"key2", VariantStruct{{"a", std::int32_t(3)}, {"b", std::string("Hi")}}}, {"key3", Variant()} }, 0, {} };
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    EXPECT_EQ(res, true);
}





TEST_F(TestParserStruct, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = false;
    static const bool VALUE4 = true;
    std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayBool root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testArrayInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;
    std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayInt32 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xFFFFFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;
    std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayUInt32 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testArrayInt64)
{
    static const std::int64_t VALUE1 = -2;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 2;
    static const std::int64_t VALUE4 = 222;
    std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayInt64 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xFFFFFFFFFFFFFFF;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 2;
    static const std::uint64_t VALUE4 = 222;
    std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayUInt64 root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testArrayFloat)
{
    static const float VALUE1 = -2.1f;
    static const float VALUE2 = 0.0f;
    static const float VALUE3 = 2.1f;
    static const float VALUE4 = 222.1f;
    std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayFloat(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayFloat root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}


TEST_F(TestParserStruct, testArrayDouble)
{
    static const double VALUE1 = -2.1;
    static const double VALUE2 = 0.0;
    static const double VALUE3 = 2.1;
    static const double VALUE4 = 222.1;
    std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayDouble root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::string VALUE4 = "Foo";
    std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayString", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayString(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayString root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', '\0', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', '\n', '\0', 'd'};
    static const Bytes VALUE4 = {'F', '\t', '\0', 123, 12};
    std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBytes(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayBytes root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}



TEST_F(TestParserStruct, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_LAST_VALUE = 12;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_LAST_VALUE = 120;
    static const std::uint32_t LAST_VALUE = 5;

    const MetaField* fieldStruct = MetaDataGlobal::instance().getField("test.TestArrayStruct", "value");
    const MetaField* fieldStructWithoutArray = MetaDataGlobal::instance().getArrayField("test.TestArrayStruct", "value");
    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
    const MetaField* fieldStructLastValue = MetaDataGlobal::instance().getField("test.TestStruct", "last_value");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    const MetaField* fieldLastValue = MetaDataGlobal::instance().getField("test.TestArrayStruct", "last_value");
    ASSERT_NE(fieldStruct, nullptr);
    ASSERT_NE(fieldStructWithoutArray, nullptr);
    ASSERT_NE(fieldStructInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldStructLastValue, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);
    ASSERT_NE(fieldLastValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*fieldStruct))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE1_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE1_STRING.data(), VALUE1_STRING.size()), VALUE1_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), VALUE1_LAST_VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE2_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE2_STRING.data(), VALUE2_STRING.size()), VALUE2_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), VALUE2_LAST_VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq("", 0), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), LAST_VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayStruct root = {{
        {{VALUE1_INT32},{VALUE1_STRING},VALUE1_LAST_VALUE},
        {{VALUE2_INT32},{VALUE2_STRING},VALUE2_LAST_VALUE},
        {}
    }, LAST_VALUE};

    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}





TEST_F(TestParserStruct, testArrayEnumInt32)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo::Enum)123;
    std::vector<test::Foo> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    test::TestArrayEnum root = {VALUE};
    ParserStruct parser(mockVisitor, root);
    bool res = parser.parseStruct();
    ASSERT_EQ(res, true);
}




