#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "serializevariant/ParserVariant.h"
#include "variant/VariantValueStruct.h"
#include "variant/VariantValueList.h"
#include "variant/VariantValues.h"
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
        MetaDataGlobal::setInstance(nullptr);
    }

    MetaField prepareMetaDataforEnum()
    {
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

        return fieldValue;
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
    bool res = parser.parseStruct("test.TestBool");
    ASSERT_EQ(res, false);
}




TEST_F(TestParserVariant, testBool)
{
    static const bool VALUE = true;
    MetaStruct structTest;
    structTest.setTypeName("test.TestBool");
    MetaField fieldValue = {MetaTypeId::TYPE_BOOL, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_INT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestUInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_UINT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_INT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestUInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_UINT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestFloat");
    MetaField fieldValue = {MetaTypeId::TYPE_FLOAT, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterFloat(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestDouble");
    MetaField fieldValue = {MetaTypeId::TYPE_DOUBLE, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestString");
    MetaField fieldValue = {MetaTypeId::TYPE_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestString");
    MetaField fieldValue = {MetaTypeId::TYPE_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldValue), std::move(VALUE))).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestBytes");
    MetaField fieldValue = {MetaTypeId::TYPE_BYTES, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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

    Variant root = VariantStruct({
            {"structInt32",  VariantStruct({ {"value", VALUE_INT32}  })},
            {"structString", VariantStruct({ {"value", VALUE_STRING} })}
    });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}




TEST_F(TestParserVariant, testUndefinedStructs)
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

    Variant root = VariantStruct({
             {"undefined",    VariantStruct({ {"value", VALUE_INT32}  })},
             {"structInt32",  VariantStruct({ {"value", VALUE_INT32}  })},
             {"undefined",    VariantStruct({ {"value", VALUE_INT32}  })},
             {"structString", VariantStruct({ {"value", VALUE_STRING} })},
             {"undefined",    VariantStruct({ {"value", VALUE_INT32}  })},
    });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}






TEST_F(TestParserVariant, testEnumAsInt)
{
    static const std::int32_t VALUE = -2;

    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), VALUE)).Times(1);
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

    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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

    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), VALUE)).Times(1);
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

    MetaStruct structTest;
    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), 0)).Times(1);
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
    MetaStruct structTest;
    MetaField fieldValue = prepareMetaDataforEnum();

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(fieldValue), 0)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayBool");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBool(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_INT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayUInt32");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_INT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayUInt64");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayFloat");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayFloat(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayDouble");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayString");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_STRING, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayString", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayString(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
    MetaStruct structTest;
    structTest.setTypeName("test.TestArrayBytes");
    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "description"};
    structTest.addField(fieldValue);

    MetaDataGlobal::instance().addStruct(structTest);

    MockIParserVisitor mockVisitor;
    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", ""};

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBytes(MatcherMetaField(fieldValue), VALUE)).Times(1);
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
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(fieldInt32), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(fieldString), std::string())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VariantList({
        VariantStruct({ {"structInt32", VariantStruct({ {"value", VALUE1_INT32} })}, {"structString", VariantStruct({ {"value", VALUE1_STRING} })} }),
        VariantStruct({ {"structString", VariantStruct({ {"value", VALUE2_STRING} })}, {"structInt32", VariantStruct({ {"value", VALUE2_INT32} })}  }),
        VariantStruct()
    })} });

    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayStruct");
    EXPECT_EQ(res, true);
}




//TEST_F(TestParserVariant, testArrayEnum)
//{
//    static const std::string VALUE1 = "FOO_HELLO";
//    static const std::string VALUE2 = "FOO_WORLD";
//    static const std::string VALUE3 = "FOO_WORLD2";
//    static const std::string VALUE4 = "blabla";
//    MetaStruct structTest;
//    structTest.setTypeName("test.TestArrayEnum");
//    MetaField fieldValue = {MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "description"};
//    structTest.addField(fieldValue);

//    MetaDataGlobal::instance().addStruct(structTest);

//    MetaEnum metaEnum;
//    metaEnum.setTypeName("test.Foo");
//    metaEnum.addEntry({"FOO_WORLD", 0 ,""});
//    metaEnum.addEntry({"FOO_HELLO", -2 ,""});
//    metaEnum.addEntry({"FOO_WORLD2", 1 ,""});
//    MetaDataGlobal::instance().addEnum(std::move(metaEnum));

//    MockIParserVisitor mockVisitor;
//    MetaField rootStruct = {MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""};

//    {
//        testing::InSequence seq;
//        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(rootStruct))).Times(1);
//        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(fieldValue), std::vector<std::string>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
//        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(rootStruct))).Times(1);
//        EXPECT_CALL(mockVisitor, finished()).Times(1);
//    }

//    std::string data = "{\"value\":[\"FOO_HELLO\",\"FOO_WORLD\",\"FOO_WORLD2\",\"blabla\"]}";
//    ParserJson parser(mockVisitor, data.data(), data.size());
//    bool res = parser.parseStruct("test.TestArrayEnum");
//    EXPECT_EQ(res, true);
//}




