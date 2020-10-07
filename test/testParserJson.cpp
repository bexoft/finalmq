#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "serializejson/ParserJson.h"
#include "metadata/MetaData.h"
#include "MockIParserVisitor.h"

//#include <thread>
//#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::ElementsAreArray;


MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.type == metaField.type &&
            arg.typeName == metaField.typeName &&
            arg.name == metaField.name);
}


MATCHER_P2(ArrayEq, compareArray, n, "")
{
    return (memcmp(arg, compareArray, n * sizeof(*arg)) == 0);
}



class TestParserJson : public testing::Test
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

    MetaField prepareMetaDataforEnum()
    {
        MetaStruct structTest;
        structTest.setTypeName("test.TestEnum");
        MetaField fieldValue = {MetaType::TYPE_ENUM, "test.Foo", "value", "description"};
        structTest.addField(fieldValue);

        MetaDataGlobal::instance().addStruct(structTest);

        MetaEnum metaEnum;
        metaEnum.setTypeName("test.Foo");
        metaEnum.addEntry({"FOO_WORLD", 0 ,""});
        metaEnum.addEntry({"FOO_HELLO", -2 ,""});
        metaEnum.addEntry({"FOO_WORLD2", 1 ,""});
        MetaDataGlobal::instance().addEnum(std::move(metaEnum));

        return fieldValue;
    }

};





TEST_F(TestParserJson, testUnknownStruct)
{
    MockIParserVisitor mockVisitor;
    std::string data = "{}";
    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, notifyError(data.c_str(), _)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBool");
    EXPECT_EQ(res, false);
}




TEST_F(TestParserJson, testBool)
{
    static const bool VALUE = true;
    MetaStruct structTest;
    structTest.setTypeName("test.TestBool");
    MetaField fieldValue = {MetaType::TYPE_BOOL, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":true}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBool");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testInt32)
{
    static const int VALUE = -2;
    MetaStruct structTest;
    structTest.setTypeName("test.TestInt32");
    MetaField fieldValue = {MetaType::TYPE_INT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":-2}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testUInt32)
{
    static const int VALUE = 130;
    MetaStruct structTest;
    structTest.setTypeName("test.TestUInt32");
    MetaField fieldValue = {MetaType::TYPE_UINT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":130}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testInt64)
{
    static const std::int64_t VALUE = -2;
    MetaStruct structTest;
    structTest.setTypeName("test.TestInt64");
    MetaField fieldValue = {MetaType::TYPE_INT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":-2}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testUInt64)
{
    static const std::uint64_t VALUE = 130;
    MetaStruct structTest;
    structTest.setTypeName("test.TestUInt64");
    MetaField fieldValue = {MetaType::TYPE_UINT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":130}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testFloat)
{
    static const float VALUE = -1.1;
    MetaStruct structTest;
    structTest.setTypeName("test.TestFloat");
    MetaField fieldValue = {MetaType::TYPE_FLOAT, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    std::string data = "{\"value\":-1.1}";
    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterFloat(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testDouble)
{
    static const double VALUE = -1.1;
    MetaStruct structTest;
    structTest.setTypeName("test.TestDouble");
    MetaField fieldValue = {MetaType::TYPE_DOUBLE, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":-1.1}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestDouble");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testString)
{
    static const std::string VALUE = "Hello World";
    MetaStruct structTest;
    structTest.setTypeName("test.TestString");
    MetaField fieldValue = {MetaType::TYPE_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":\"Hello World\"}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestString");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testStdString)
{
    std::string VALUE = "Hello World";
    MetaStruct structTest;
    structTest.setTypeName("test.TestString");
    MetaField fieldValue = {MetaType::TYPE_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldValue), std::move(VALUE))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":\"Hell\\u006f World\"}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestString");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testBytes)
{
    std::string VALUE = {'H', 'e', 12, 0, 'A'};
    MetaStruct structTest;
    structTest.setTypeName("test.TestBytes");
    MetaField fieldValue = {MetaType::TYPE_BYTES, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(fieldValue), std::move(VALUE))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":\"He\\u000c\\u0000A\"}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBytes");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    MetaStruct structInt32;
    MetaStruct structString;
    structInt32.setTypeName("test.StructInt32");
    structString.setTypeName("test.StructString");
    MetaField fieldInt32 = {MetaType::TYPE_INT32, "", "value", "description"};
    structInt32.addField(fieldInt32);
    MetaField fieldString = {MetaType::TYPE_STRING, "", "value", "description"};
    structString.addField(fieldString);

    MetaStruct structTest;
    structTest.setTypeName("test.TestStruct");
    MetaField fieldStructInt32 = {MetaType::TYPE_STRUCT, "test.StructInt32", "structInt32", "description"};
    structTest.addField(fieldStructInt32);
    MetaField fieldStructString = {MetaType::TYPE_STRUCT, "test.StructString", "structString", "description"};
    structTest.addField(fieldStructString);

    MetaDataGlobal::instance().addStruct(structInt32);
    MetaDataGlobal::instance().addStruct(structString);
    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestStruct", ""};

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

    std::string data = "{\"structInt32\":{\"value\":-2},\"structString\":{\"value\":\"Hello World\"}}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}




TEST_F(TestParserJson, testUndefinedStructs)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    MetaStruct structInt32;
    MetaStruct structString;
    structInt32.setTypeName("test.StructInt32");
    structString.setTypeName("test.StructString");
    MetaField fieldInt32 = {MetaType::TYPE_INT32, "", "value", "description"};
    structInt32.addField(fieldInt32);
    MetaField fieldString = {MetaType::TYPE_STRING, "", "value", "description"};
    structString.addField(fieldString);

    MetaStruct structTest;
    structTest.setTypeName("test.TestStruct");
    MetaField fieldStructInt32 = {MetaType::TYPE_STRUCT, "test.StructInt32", "structInt32", "description"};
    structTest.addField(fieldStructInt32);
    MetaField fieldStructString = {MetaType::TYPE_STRUCT, "test.StructString", "structString", "description"};
    structTest.addField(fieldStructString);

    MetaDataGlobal::instance().addStruct(structInt32);
    MetaDataGlobal::instance().addStruct(structString);
    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestStruct", ""};

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

    std::string data = "{\"undefined\":{\"undefined\":{\"undefined\":{\"undefined\":{}},\"undefined2\":{\"undefined\":{\"undefined\":1234}}}},\"structInt32\":{\"value\":-2},\"undefined3\":{\"undefined\":{}},\"structString\":{\"value\":\"Hello World\",\"undefine4\":1234,\"undefined5\":{\"undefined\":{}}},\"undefined6\":{\"undefined\":{}}}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testUndefinedValues)
{
    static const std::string VALUE = "Hello World";
    MetaStruct structTest;
    structTest.setTypeName("test.TestString");
    MetaField fieldValue = {MetaType::TYPE_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"unknown1\":1234,\"value\":\"Hello World\",\"unknown2\":1234}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestString");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testEnumAsInt)
{
    static const std::int32_t VALUE = -2;

    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":-2}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testEnumAsString)
{
    static const std::string VALUE = "FOO_HELLO";

    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":\"FOO_HELLO\"}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}

TEST_F(TestParserJson, testEnumAsStdString)
{
    std::string VALUE = "FOO_HELLO";

    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), std::move(VALUE))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":\"FOO_HELL\\u004f\"}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testEnumNotAvailableInt)
{
    static const std::int32_t VALUE = 42;
    MetaStruct structTest;

    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":42}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}

TEST_F(TestParserJson, testEnumNotAvailableString)
{
    static const std::string VALUE = "blabla";

    MetaStruct structTest;
    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":\"blabla\"}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}

TEST_F(TestParserJson, testArrayNoArray)
{
    MetaStruct structTest;
    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testArrayUnknownValue)
{
    MetaStruct structTest;
    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"blabla\":[]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = false;
    static const bool VALUE4 = true;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayBool");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_BOOL, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBoolMove(MatcherMetaField(fieldValue), std::vector<bool>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[true,false,false,true]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayBool");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testArrayInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayInt32");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_INT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(fieldValue), std::vector<std::int32_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[-2,0,2,222]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xFFFFFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayUInt32");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_UINT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(fieldValue), std::vector<std::uint32_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[4294967294,0,2,222]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayUInt32");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testArrayInt64)
{
    static const std::int64_t VALUE1 = -2;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 2;
    static const std::int64_t VALUE4 = 222;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayInt64");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_INT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(fieldValue), std::vector<std::int64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[-2,0,2,222]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xFFFFFFFFFFFFFFF;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 2;
    static const std::uint64_t VALUE4 = 222;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayUInt64");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_UINT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(fieldValue), std::vector<std::uint64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[1152921504606846975,0,2,222]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testArrayFloat)
{
    static const float VALUE1 = -2.1;
    static const float VALUE2 = 0.0;
    static const float VALUE3 = 2.1;
    static const float VALUE4 = 222.1;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayFloat");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_FLOAT, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayFloat(MatcherMetaField(fieldValue), std::vector<float>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[-2.1,0,2.1,222.1]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserJson, testArrayDouble)
{
    static const double VALUE1 = -2.1;
    static const double VALUE2 = 0.0;
    static const double VALUE3 = 2.1;
    static const double VALUE4 = 222.1;
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayDouble");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_DOUBLE, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(fieldValue), std::vector<double>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[-2.1,0,2.1,222.1]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayDouble");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::string VALUE4 = "Foo";
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayString");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStringMove(MatcherMetaField(fieldValue), std::vector<std::string>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[\"Hello\",\"\",\"World\",\"Foo\"]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayString");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testArrayBytes)
{
    static const std::string VALUE1 = {'H', 'e', '\0', 'l', 'o'};
    static const std::string VALUE2 = {};
    static const std::string VALUE3 = {'W', 'o', '\n', '\0', 'd'};
    static const std::string VALUE4 = {'F', '\t', '\0', 123, 12};
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayBytes");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_BYTES, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBytesMove(MatcherMetaField(fieldValue), std::vector<std::string>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[\"He\\u0000lo\",\"\",\"Wo\n\\u0000d\",\"F\t\\u0000\\u007b\\u000c\"]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayBytes");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserJson, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    MetaStruct structInt32;
    MetaStruct structString;
    structInt32.setTypeName("test.StructInt32");
    structString.setTypeName("test.StructString");
    MetaField fieldInt32 = {MetaType::TYPE_INT32, "", "value", "description"};
    structInt32.addField(fieldInt32);
    MetaField fieldString = {MetaType::TYPE_STRING, "", "value", "description"};
    structString.addField(fieldString);

    MetaStruct structTest;
    structTest.setTypeName("test.TestStruct");
    MetaField fieldStructInt32 = {MetaType::TYPE_STRUCT, "test.StructInt32", "structInt32", "description"};
    structTest.addField(fieldStructInt32);
    MetaField fieldStructString = {MetaType::TYPE_STRUCT, "test.StructString", "structString", "description"};
    structTest.addField(fieldStructString);

    MetaStruct structArrayTest;
    structArrayTest.setTypeName("test.TestArrayStruct");
    MetaField fieldStruct = {MetaType::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "description"};
    structArrayTest.addField(fieldStruct);
    MetaField fieldStructWithoutArray = {MetaType::TYPE_STRUCT, "test.TestStruct", "", "description"};

    MetaDataGlobal::instance().addStruct(structInt32);
    MetaDataGlobal::instance().addStruct(structString);
    MetaDataGlobal::instance().addStruct(structTest);
    MetaDataGlobal::instance().addStruct(structArrayTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayStruct", ""};

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

    std::string data = "{\"value\":[{\"structInt32\":{\"value\":-2},\"structString\":{\"value\":\"Hello World\"}},{\"structInt32\":{\"value\":345},\"structString\":{\"value\":\"foo\"}},{}]}";

    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayStruct");
    EXPECT_EQ(res, true);
}




TEST_F(TestParserJson, testArrayEnum)
{
    static const std::string VALUE1 = "FOO_HELLO";
    static const std::string VALUE2 = "FOO_WORLD";
    static const std::string VALUE3 = "FOO_WORLD2";
    static const std::string VALUE4 = "blabla";
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayEnum");
    MetaField fieldValue = {MetaType::TYPE_ARRAY_ENUM, "test.Foo", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MetaEnum metaEnum;
    metaEnum.setTypeName("test.Foo");
    metaEnum.addEntry({"FOO_WORLD", 0 ,""});
    metaEnum.addEntry({"FOO_HELLO", -2 ,""});
    metaEnum.addEntry({"FOO_WORLD2", 1 ,""});
    MetaDataGlobal::instance().addEnum(std::move(metaEnum));

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaType::TYPE_STRUCT, "test.TestArrayEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(fieldValue), std::vector<std::string>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    std::string data = "{\"value\":[\"FOO_HELLO\",\"FOO_WORLD\",\"FOO_WORLD2\",\"blabla\"]}";
    ParserJson parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayEnum");
    EXPECT_EQ(res, true);
}




