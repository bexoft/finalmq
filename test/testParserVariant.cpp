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


#include "finalmq/serializevariant/ParserVariant.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/metadata/MetaData.h"
#include "MockIParserVisitor.h"

//#include <thread>
//#include <chrono>

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



class TestParserVariant : public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};





TEST_F(TestParserVariant, testUnknownStruct)
{
    MockIParserVisitor mockVisitor;
    std::string data = "{}";
    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, notifyError(nullptr, _)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    static const bool VALUE = true;
    Variant root = VariantStruct({{"value", VALUE}});

    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.BlaBla");
    ASSERT_EQ(res, false);
}




TEST_F(TestParserVariant, testBool)
{
    static const bool VALUE = true;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestBool");
    ASSERT_EQ(res, true);
}


TEST_F(TestParserVariant, testInt32)
{
    static const std::int32_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testUInt32)
{
    static const std::uint32_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestUInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testInt64)
{
    static const std::int64_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testUInt64)
{
    static const std::uint64_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testFloat)
{
    static const float VALUE = -1.1;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterFloat(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testDouble)
{
    static const double VALUE = -1.1;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestDouble");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testString)
{
    static const std::string VALUE = "Hello World";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestString");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testStdString)
{
    std::string VALUE = "123";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldValue), std::move(VALUE))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", 123}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestString");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testBytes)
{
    Bytes VALUE = {'H', 'e', 12, 0, 'A'};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestBytes");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_LAST = 12;

    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
    const MetaField* fieldStructLastValue = MetaDataGlobal::instance().getField("test.TestStruct", "last_value");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldStructInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldStructLastValue, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestStruct", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), VALUE_LAST)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({
            {"struct_int32",  VariantStruct({ {"value", VALUE_INT32}  })},
            {"struct_string", VariantStruct({ {"value", VALUE_STRING} })},
            {"last_value", VALUE_LAST}
    });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}




TEST_F(TestParserVariant, testUndefinedStructs)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_LAST = 12;

    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
    const MetaField* fieldStructLastValue = MetaDataGlobal::instance().getField("test.TestStruct", "last_value");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldStructInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldStructLastValue, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestStruct", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), VALUE_LAST)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({
             {"undefined",     VariantStruct({ {"value", VALUE_INT32}  })},
             {"struct_int32",  VariantStruct({ {"value", VALUE_INT32}  })},
             {"undefined",     VariantStruct({ {"value", VALUE_INT32}  })},
             {"struct_string", VariantStruct({ {"value", VALUE_STRING} })},
             {"undefined",     VariantStruct({ {"value", VALUE_INT32}  })},
             {"last_value", VALUE_LAST}
    });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}






TEST_F(TestParserVariant, testEnumAsInt)
{
    static const std::int32_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testEnumAsString)
{
    static const std::string VALUE = "FOO_HELLO";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}




TEST_F(TestParserVariant, testEnumNotAvailableInt)
{
    static const std::int32_t VALUE = 42;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}

TEST_F(TestParserVariant, testEnumNotAvailableString)
{
    static const std::string VALUE = "blabla";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}

TEST_F(TestParserVariant, testArrayNoArray)
{
    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VariantList()}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testArrayUnknownValue)
{
    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"blabla", VariantList()}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = false;
    static const bool VALUE4 = true;
    std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayBool");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;
    std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xFFFFFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;
    std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayUInt32");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayInt64)
{
    static const std::int64_t VALUE1 = -2;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 2;
    static const std::int64_t VALUE4 = 222;
    std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xFFFFFFFFFFFFFFF;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 2;
    static const std::uint64_t VALUE4 = 222;
    std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayFloat)
{
    static const float VALUE1 = -2.1;
    static const float VALUE2 = 0.0;
    static const float VALUE3 = 2.1;
    static const float VALUE4 = 222.1;
    std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayFloat(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testArrayDouble)
{
    static const double VALUE1 = -2.1;
    static const double VALUE2 = 0.0;
    static const double VALUE3 = 2.1;
    static const double VALUE4 = 222.1;
    std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayDouble");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::string VALUE4 = "Foo";
    std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayString", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayString(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayString");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', '\0', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', '\n', '\0', 'd'};
    static const Bytes VALUE4 = {'F', '\t', '\0', 123, 12};
    std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBytes(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayBytes");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayStruct)
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
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
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
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), std::string())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), LAST_VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VariantList({
        VariantStruct({ {"struct_int32", VariantStruct({ {"value", VALUE1_INT32} })}, {"struct_string", VariantStruct({ {"value", VALUE1_STRING} })}, {"last_value", VALUE1_LAST_VALUE} }),
        VariantStruct({ {"struct_string", VariantStruct({ {"value", VALUE2_STRING} })}, {"struct_int32", VariantStruct({ {"value", VALUE2_INT32} })}, {"last_value", VALUE2_LAST_VALUE} }),
        VariantStruct(),
    })}, {"last_value", LAST_VALUE} });

    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayStruct");
    EXPECT_EQ(res, true);
}




TEST_F(TestParserVariant, testArrayEnumString)
{
    static const std::string VALUE1 = "FOO_HELLO";
    static const std::string VALUE2 = "FOO_WORLD";
    static const std::string VALUE3 = "FOO_WORLD2";
    static const std::string VALUE4 = "blabla";
    std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayEnum");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testArrayEnumInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::int32_t VALUE4 = 123;
    std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayEnum");
    EXPECT_EQ(res, true);
}




