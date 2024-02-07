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
#include "finalmq/serializevariant/VarValueToVariant.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/metadataserialize/variant.fmq.h"
#include "MockIParserVisitor.h"
#include "matchers.h"

//#include <thread>
//#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::ElementsAreArray;

using namespace finalmq;



class TestParserVariant : public testing::Test
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





TEST_F(TestParserVariant, testUnknownStruct)
{
    MockIParserVisitor mockVisitor;
    std::string data = "{}";
    {
        testing::InSequence seq;
        //EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestBool");
    ASSERT_EQ(res, true);
}


TEST_F(TestParserVariant, testInt8)
{
    static const std::int8_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt8(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestInt8");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testUInt8)
{
    static const std::uint8_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt8(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestUInt8");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testInt16)
{
    static const std::int32_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt16(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VALUE} });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestInt16");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testUInt16)
{
    static const std::uint16_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt16(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VALUE} });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestUInt16");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testInt32)
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

    Variant root = VariantStruct({ {"value", VALUE} });
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VALUE} });
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testFloat)
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(*fieldValue), VALUE)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldValue), std::move(VALUE))).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), VALUE_LAST)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), ArrayEq(VALUE_STRING.data(), VALUE_STRING.size()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), VALUE_LAST)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testVariantEmpty)
{
    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue))).Times(1);

        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldValueInt32), 0)).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue2))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue2))).Times(1);

        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct{ {"value", Variant()}};
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestVariant");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testVariantString)
{
    std::string VALUE_STRING = "123";

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_STRING)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq(VALUE_STRING.data()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue))).Times(1);

        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldValueInt32), 0)).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue2))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue2))).Times(1);

        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct{ {"value", VALUE_STRING} };
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestVariant");
    EXPECT_EQ(res, true);
}

TEST_F(TestParserVariant, testVariantStruct)
{
    Variant root = VariantStruct{ {"value", VariantStruct{ {"key1", VariantList{std::int32_t(2), std::string("Hello")}}, {"key2", VariantStruct{{"a", std::int32_t(3)}, {"b", std::string("Hi")}}}, {"key3", Variant()} } } };

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        // VariantStruct{ {"value", VariantStruct{
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*m_fieldStruct))).Times(1);
            // {"key1", VariantList{
            EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
            EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key1"), 4)).Times(1);
            EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_VARIANTLIST)).Times(1);
            EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
                // 2
                EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
                EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_INT32)).Times(1);
                EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldInt32), 2)).Times(1);
                EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
                // , std::string("Hello")
                EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
                EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_STRING)).Times(1);
                EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq("Hello"), 5)).Times(1);
                EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
            // }
            EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
            EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);

            // {"key2", VariantStruct{
            EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
            EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key2"), 4)).Times(1);
            EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_VARIANTSTRUCT)).Times(1);
            EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*m_fieldStruct))).Times(1);
                // {"a", 3},
                EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
                EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("a"), 1)).Times(1);
                EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_INT32)).Times(1);
                EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldInt32), 3)).Times(1);
                EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
                // {"b", std::string("Hi")}
                EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
                EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("b"), 1)).Times(1);
                EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_STRING)).Times(1);
                EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq("Hi"), 2)).Times(1);
                EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
            // }
            EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*m_fieldStruct))).Times(1);
            EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);

            // {
            EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
            EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key3"), 4)).Times(1);
            EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_NONE)).Times(1);
            EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldStructWithoutArray))).Times(1);
            // }}
        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*m_fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue))).Times(1);

        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldValueInt32), 0)).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*m_fieldValue2))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*m_fieldIndex), VarValueType2Index::VARVALUETYPE_NONE)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*m_fieldValue2))).Times(1);

        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestVariant");
    EXPECT_EQ(res, true);
}




TEST_F(TestParserVariant, testArrayNoArray)
{
    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), 0)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), 0)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayBool");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayInt8)
{
    static const std::int8_t VALUE1 = -2;
    static const std::int8_t VALUE2 = 0;
    static const std::int8_t VALUE3 = 2;
    static const std::int8_t VALUE4 = 22;
    std::vector<std::int8_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt8(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayInt8");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayInt16)
{
    static const std::int16_t VALUE1 = -2;
    static const std::int16_t VALUE2 = 0;
    static const std::int16_t VALUE3 = 2;
    static const std::int16_t VALUE4 = 222;
    std::vector<std::int16_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt16(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VALUE} });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayInt16");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserVariant, testArrayUInt16)
{
    static const std::uint16_t VALUE1 = 0xFFFE;
    static const std::uint16_t VALUE2 = 0;
    static const std::uint16_t VALUE3 = 2;
    static const std::uint16_t VALUE4 = 222;
    std::vector<std::uint16_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt16(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VALUE} });
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayUInt16");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;
    std::vector<std::int32_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VALUE} });
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
    std::vector<std::uint32_t> VALUE = { VALUE1, VALUE2, VALUE3, VALUE4 };

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({ {"value", VALUE} });
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserVariant, testArrayFloat)
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayString(MatcherMetaField(*fieldValue), VALUE)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBytes(MatcherMetaField(*fieldValue), VALUE)).Times(1);
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
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), std::string())).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldStructLastValue), 0)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), LAST_VALUE)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
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

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    Variant root = VariantStruct({{"value", VALUE}});
    ParserVariant parser(mockVisitor, root);
    bool res = parser.parseStruct("test.TestArrayEnum");
    EXPECT_EQ(res, true);
}




