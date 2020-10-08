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
        MetaDataGlobal::setInstance(nullptr);
    }

};



TEST_F(TestParserProto, testUnknownStruct)
{
    std::string data;

    test::TestMessageBool message;
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, notifyError(data.c_str(), _)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBool");
    EXPECT_EQ(res, false);
}





TEST_F(TestParserProto, testBool)
{
    static const bool VALUE = true;
    MetaStruct structTest;
    structTest.setTypeName("test.TestBool");
    MetaField fieldValue = {MetaTypeId::TYPE_BOOL, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageBool message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_INT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageInt32 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestUInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_UINT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageUInt32 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_INT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageInt64 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestUInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_UINT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageUInt64 message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestFloat");
    MetaField fieldValue = {MetaTypeId::TYPE_FLOAT, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageFloat message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterFloat(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestDouble");
    MetaField fieldValue = {MetaTypeId::TYPE_DOUBLE, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageDouble message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestString");
    MetaField fieldValue = {MetaTypeId::TYPE_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageString message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestBytes");
    MetaField fieldValue = {MetaTypeId::TYPE_BYTES, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageBytes message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structInt32;
    MetaStruct structString;
    structInt32.setTypeName("test.StructInt32");
    structString.setTypeName("test.StructString");
    MetaField fieldInt32 = {MetaTypeId::TYPE_INT32, "", "value", "description"};
    structInt32.addField(fieldInt32);
    MetaField fieldString = {MetaTypeId::TYPE_STRING, "", "value", "description"};
    structString.addField(fieldString);

    MetaStruct structTest;
    structTest.setTypeName("test.TestStruct");
    MetaField fieldStructInt32 = {MetaTypeId::TYPE_STRUCT, "test.StructInt32", "structInt32", "description"};
    structTest.addField(fieldStructInt32);
    MetaField fieldStructString = {MetaTypeId::TYPE_STRUCT, "test.StructString", "structString", "description"};
    structTest.addField(fieldStructString);

    MetaDataGlobal::instance().addStruct(structInt32);
    MetaDataGlobal::instance().addStruct(structString);
    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageStruct message;
    message.mutable_struct_int32()->set_value(VALUE_INT32);
    message.mutable_struct_string()->set_value(VALUE_STRING);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestStruct", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testEnum)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;
    MetaStruct structTest;
    structTest.setTypeName("test.TestEnum");
    MetaField fieldValue = {MetaTypeId::TYPE_ENUM, "test.Foo", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MetaEnum metaEnum;
    metaEnum.setTypeName("test.Foo");
    metaEnum.addEntry({"FOO_WORLD", 0 ,""});
    metaEnum.addEntry({"FOO_HELLO", -2 ,""});
    metaEnum.addEntry({"FOO_WORLD2", 1 ,""});
    MetaDataGlobal::instance().addEnum(std::move(metaEnum));

    std::string data;

    test::TestMessageEnum message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserProto, testEnumNotAvailable)
{
    static const test::Foo VALUE = (test::Foo)123;
    MetaStruct structTest;
    structTest.setTypeName("test.TestEnum");
    MetaField fieldValue = {MetaTypeId::TYPE_ENUM, "test.Foo", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MetaEnum metaEnum;
    metaEnum.setTypeName("test.Foo");
    metaEnum.addEntry({"FOO_WORLD", 0 ,""});
    metaEnum.addEntry({"FOO_HELLO", -2 ,""});
    metaEnum.addEntry({"FOO_WORLD2", 1 ,""});
    MetaDataGlobal::instance().addEnum(std::move(metaEnum));

    std::string data;

    test::TestMessageEnum message;
    message.set_value(VALUE);
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), 0)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayBool");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayBool message;
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
        EXPECT_CALL(mockVisitor, enterArrayBool(MatcherMetaField(fieldValue), std::vector<bool>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_INT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayInt32 message;
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
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(fieldValue), std::vector<std::int32_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayUInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayUInt32 message;
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
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(fieldValue), std::vector<std::uint32_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_INT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayInt64 message;
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
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(fieldValue), std::vector<std::int64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayUInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayUInt64 message;
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
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(fieldValue), std::vector<std::uint64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayFloat");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayFloat message;
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
        EXPECT_CALL(mockVisitor, enterArrayFloat(MatcherMetaField(fieldValue), std::vector<float>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayDouble");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayDouble message;
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
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(fieldValue), std::vector<double>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayString");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayString message;
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
        EXPECT_CALL(mockVisitor, enterArrayString(MatcherMetaField(fieldValue), std::vector<std::string>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayBytes");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data;

    test::TestMessageArrayBytes message;
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
        EXPECT_CALL(mockVisitor, enterArrayBytes(MatcherMetaField(fieldValue), std::vector<Bytes>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
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
    MetaStruct structInt32;
    MetaStruct structString;
    structInt32.setTypeName("test.StructInt32");
    structString.setTypeName("test.StructString");
    MetaField fieldInt32 = {MetaTypeId::TYPE_INT32, "", "value", "description"};
    structInt32.addField(fieldInt32);
    MetaField fieldString = {MetaTypeId::TYPE_STRING, "", "value", "description"};
    structString.addField(fieldString);

    MetaStruct structTest;
    structTest.setTypeName("test.TestStruct");
    MetaField fieldStructInt32 = {MetaTypeId::TYPE_STRUCT, "test.StructInt32", "structInt32", "description"};
    structTest.addField(fieldStructInt32);
    MetaField fieldStructString = {MetaTypeId::TYPE_STRUCT, "test.StructString", "structString", "description"};
    structTest.addField(fieldStructString);

    MetaStruct structArrayTest;
    structArrayTest.setTypeName("test.TestArrayStruct");
    MetaField fieldStruct = {MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "description"};
    structArrayTest.addField(fieldStruct);
    MetaField fieldStructWithoutArray = {MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "description"};

    MetaDataGlobal::instance().addStruct(structInt32);
    MetaDataGlobal::instance().addStruct(structString);
    MetaDataGlobal::instance().addStruct(structTest);
    MetaDataGlobal::instance().addStruct(structArrayTest);

    std::string data;

    test::TestMessageArrayStruct message;
    auto* value = message.mutable_value();
    auto* message1 = value->Add();
    message1->mutable_struct_int32()->set_value(VALUE1_INT32);
    message1->mutable_struct_string()->set_value(VALUE1_STRING);
    auto* message2 = value->Add();
    message2->mutable_struct_int32()->set_value(VALUE2_INT32);
    message2->mutable_struct_string()->set_value(VALUE2_STRING);
    auto* message3 = value->Add();
    message.SerializeToString(&data);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(fieldStruct))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(fieldInt32), VALUE1_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldString), ArrayEq(VALUE1_STRING.data(), VALUE1_STRING.size()), VALUE1_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(fieldInt32), VALUE2_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldString), ArrayEq(VALUE2_STRING.data(), VALUE2_STRING.size()), VALUE2_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayStruct");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserProto, testArrayEnum)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo)123;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayEnum");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MetaEnum metaEnum;
    metaEnum.setTypeName("test.Foo");
    metaEnum.addEntry({"FOO_WORLD", 0 ,""});
    metaEnum.addEntry({"FOO_HELLO", -2 ,""});
    metaEnum.addEntry({"FOO_WORLD2", 1 ,""});
    MetaDataGlobal::instance().addEnum(std::move(metaEnum));

    std::string data;

    test::TestMessageArrayEnum message;
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
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(fieldValue), std::vector<std::int32_t>({VALUE1, VALUE2, VALUE3, 0}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserProto parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayEnum");
    EXPECT_EQ(res, true);
}
