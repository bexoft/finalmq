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


#include "serializeproto/ParserProto.h"
#include "metadata/MetaData.h"
#include "MockIParserVisitor.h"
#include "test.pb.h"

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



class TestParserProto : public testing::Test
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



TEST_F(TestParserProto, testUnknownStruct)
{
    std::string data;

    fmq::test::TestBool message;
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, notifyError(data.c_str(), _)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.BlaBla");
    EXPECT_EQ(res, false);
}





TEST_F(TestParserProto, testBool)
{
    static const bool VALUE = true;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestBool message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBool");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testInt32)
{
    static const int VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestInt32 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testUInt32)
{
    static const int VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestUInt32 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testInt64)
{
    static const std::int64_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestInt64 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testUInt64)
{
    static const std::uint64_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestUInt64 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testFloat)
{
    static const float VALUE = -1.1;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestFloat message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterFloat(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testDouble)
{
    static const double VALUE = -1.1;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestDouble message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestDouble");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testString)
{
    static const std::string VALUE = "Hello World";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestString message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestString");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testBytes)
{
    static const std::string VALUE = {'H', 'e', 12, 0, 'A'};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestBytes message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBytes");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";

    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    const MetaField* fieldString = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldStructInt32, nullptr);
    ASSERT_NE(fieldStructString, nullptr);
    ASSERT_NE(fieldInt32, nullptr);
    ASSERT_NE(fieldString, nullptr);

    std::string data;

    fmq::test::TestStruct message;
    message.mutable_struct_int32()->set_value(VALUE_INT32);
    message.mutable_struct_string()->set_value(VALUE_STRING);
    message.SerializeToString(&data);

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
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testEnum)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestEnum message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testEnumNotAvailable)
{
    static const fmq::test::Foo VALUE = (fmq::test::Foo)123;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestEnum message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = false;
    static const bool VALUE4 = true;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayBool message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBool(MatcherMetaField(*fieldValue), std::vector<bool>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayBool");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testArrayInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayInt32 message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(*fieldValue), std::vector<std::int32_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xFFFFFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayUInt32 message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(*fieldValue), std::vector<std::uint32_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayUInt32");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testArrayInt64)
{
    static const std::int64_t VALUE1 = -2;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 2;
    static const std::int64_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayInt64 message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(*fieldValue), std::vector<std::int64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xFFFFFFFFFFFFFFFE;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 2;
    static const std::uint64_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayUInt64 message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(*fieldValue), std::vector<std::uint64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testArrayFloat)
{
    static const float VALUE1 = -2.1;
    static const float VALUE2 = 0.0;
    static const float VALUE3 = 2.1;
    static const float VALUE4 = 222.1;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayFloat message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayFloat(MatcherMetaField(*fieldValue), std::vector<float>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testArrayDouble)
{
    static const double VALUE1 = -2.1;
    static const double VALUE2 = 0.0;
    static const double VALUE3 = 2.1;
    static const double VALUE4 = 222.1;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayDouble message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(*fieldValue), std::vector<double>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayDouble");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::string VALUE4 = "Foo";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayString", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayString message;
    auto* value = message.mutable_value();
    *value->Add() = VALUE1;
    *value->Add() = VALUE2;
    *value->Add() = VALUE3;
    *value->Add() = VALUE4;
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayString(MatcherMetaField(*fieldValue), std::vector<std::string>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayString");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', '\0', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', '\n', '\0', 'd'};
    static const Bytes VALUE4 = {'F', '\t', '\0', 123, 12};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayBytes message;
    auto* value = message.mutable_value();
    *value->Add() = std::string(reinterpret_cast<const char*>(VALUE1.data()), VALUE1.size());
    *value->Add() = std::string(reinterpret_cast<const char*>(VALUE2.data()), VALUE2.size());
    *value->Add() = std::string(reinterpret_cast<const char*>(VALUE3.data()), VALUE3.size());
    *value->Add() = std::string(reinterpret_cast<const char*>(VALUE4.data()), VALUE4.size());
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBytes(MatcherMetaField(*fieldValue), std::vector<Bytes>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayBytes");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t LAST_VALUE = 5;

    const MetaField* fieldStruct = MetaDataGlobal::instance().getField("test.TestArrayStruct", "value");
    const MetaField* fieldStructWithoutArray = MetaDataGlobal::instance().getArrayField("test.TestArrayStruct", "value");
    const MetaField* fieldStructInt32 = MetaDataGlobal::instance().getField("test.TestStruct", "struct_int32");
    const MetaField* fieldStructString = MetaDataGlobal::instance().getField("test.TestStruct", "struct_string");
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

    std::string data;

    fmq::test::TestArrayStruct message;
    auto* value = message.mutable_value();
    auto* message1 = value->Add();
    message1->mutable_struct_int32()->set_value(VALUE1_INT32);
    message1->mutable_struct_string()->set_value(VALUE1_STRING);
    auto* message2 = value->Add();
    message2->mutable_struct_int32()->set_value(VALUE2_INT32);
    message2->mutable_struct_string()->set_value(VALUE2_STRING);
    auto* message3 = value->Add();
    message.set_last_value(LAST_VALUE);
    message.SerializeToString(&data);

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
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE2_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE2_STRING.data(), VALUE2_STRING.size()), VALUE2_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), LAST_VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayStruct");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testArrayEnum)
{
    static const fmq::test::Foo VALUE1 = fmq::test::Foo::FOO_HELLO;
    static const fmq::test::Foo VALUE2 = fmq::test::Foo::FOO_WORLD;
    static const fmq::test::Foo VALUE3 = fmq::test::Foo::FOO_WORLD2;
    static const fmq::test::Foo VALUE4 = (fmq::test::Foo)123;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;

    fmq::test::TestArrayEnum message;
    auto* value = message.mutable_value();
    value->Add(VALUE1);
    value->Add(VALUE2);
    value->Add(VALUE3);
    value->Add(VALUE4);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(*fieldValue), std::vector<std::int32_t>({VALUE1, VALUE2, VALUE3, 0}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayEnum");
    EXPECT_EQ(res, true);
}
