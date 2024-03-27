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


#include "finalmq/serializeqt/ParserQt.h"
#include "finalmq/metadata/MetaData.h"
#include "MockIParserVisitor.h"
#include "matchers.h"


using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::ElementsAreArray;

using namespace finalmq;



class TestParserQt : public testing::Test
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



TEST_F(TestParserQt, testUnknownStruct)
{
    std::string data;
    data.resize(10);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, notifyError(data.c_str(), _)).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.BlaBla");
    EXPECT_EQ(res, false);
}





TEST_F(TestParserQt, testBool)
{
    static const bool VALUE = true;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterBool(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBool");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testInt8)
{
    static const int VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt8(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt8");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testUInt8)
{
    static const int VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt8(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt8");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testInt16)
{
    static const int VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt16(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt16");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testUInt16)
{
    static const int VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt16(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt16");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testInt32)
{
    static const int VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testUInt32)
{
    static const int VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt32");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testInt64)
{
    static const std::int64_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 56));
    data.push_back(static_cast<char>(VALUE >> 48));
    data.push_back(static_cast<char>(VALUE >> 40));
    data.push_back(static_cast<char>(VALUE >> 32));
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testUInt64)
{
    static const std::uint64_t VALUE = 130;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 56));
    data.push_back(static_cast<char>(VALUE >> 48));
    data.push_back(static_cast<char>(VALUE >> 40));
    data.push_back(static_cast<char>(VALUE >> 32));
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt64(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testFloat)
{
    static const float VALUE = -1.1f;
    union
    {
        std::uint64_t v;
        double d;
    } u;
    u.d = static_cast<double>(VALUE);

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterFloat(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testDouble)
{
    static const double VALUE = -1.1;
    union
    {
        std::uint64_t v;
        double d;
    } u;
    u.d = VALUE;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(u.v >> 56));
    data.push_back(static_cast<char>(u.v >> 48));
    data.push_back(static_cast<char>(u.v >> 40));
    data.push_back(static_cast<char>(u.v >> 32));
    data.push_back(static_cast<char>(u.v >> 24));
    data.push_back(static_cast<char>(u.v >> 16));
    data.push_back(static_cast<char>(u.v >> 8));
    data.push_back(static_cast<char>(u.v >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterDouble(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestDouble");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testString)
{
    static const std::string VALUE = "Hello World";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestString", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldValue), std::string(VALUE))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestString");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testBytes)
{
    static const std::string VALUE = {'H', 'e', 12, 0, 'A'};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(5);
    data.push_back('H');
    data.push_back('e');
    data.push_back(12);
    data.push_back(0);
    data.push_back('A');

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterBytes(MatcherMetaField(*fieldValue), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestBytes");
    EXPECT_EQ(res, true);
}

TEST_F(TestParserQt, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 0x12345678;

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), std::string(VALUE_STRING))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), VALUE_UINT32)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestStruct");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testEnum)
{
    static const std::int32_t VALUE = -2;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testEnumNotAvailable)
{
    static const std::uint32_t VALUE = 123;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(static_cast<char>(VALUE >> 24));
    data.push_back(static_cast<char>(VALUE >> 16));
    data.push_back(static_cast<char>(VALUE >> 8));
    data.push_back(static_cast<char>(VALUE >> 0));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterEnum(MatcherMetaField(*fieldValue), VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestEnum");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = false;
    static const bool VALUE4 = true;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBool", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(0x09);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBool(MatcherMetaField(*fieldValue), std::vector<bool>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayBool");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayInt8)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 22;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt8", "value");
    ASSERT_NE(fieldValue, nullptr);

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);
    data.push_back(static_cast<char>(VALUE1));
    data.push_back(static_cast<char>(VALUE2));
    data.push_back(static_cast<char>(VALUE3));
    data.push_back(static_cast<char>(VALUE4));

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt8(MatcherMetaField(*fieldValue), std::vector<std::int8_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt8");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayInt16)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt16(MatcherMetaField(*fieldValue), std::vector<std::int16_t>({ VALUE1, VALUE2, VALUE3, VALUE4 }))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt16");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testArrayUInt16)
{
    static const std::uint32_t VALUE1 = 0xFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt16", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt16(MatcherMetaField(*fieldValue), std::vector<std::uint16_t>({ VALUE1, VALUE2, VALUE3, VALUE4 }))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayUInt16");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayInt32)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 2;
    static const std::int32_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt32(MatcherMetaField(*fieldValue), std::vector<std::int32_t>({ VALUE1, VALUE2, VALUE3, VALUE4 }))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt32");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xFFFFFFFE;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 2;
    static const std::uint32_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt32", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(*fieldValue), std::vector<std::uint32_t>({ VALUE1, VALUE2, VALUE3, VALUE4 }))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayUInt32");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayInt64)
{
    static const std::int64_t VALUE1 = -2;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 2;
    static const std::int64_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayInt64(MatcherMetaField(*fieldValue), std::vector<std::int64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayInt64");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xFFFFFFFFFFFFFFFE;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 2;
    static const std::uint64_t VALUE4 = 222;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayUInt64", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayUInt64(MatcherMetaField(*fieldValue), std::vector<std::uint64_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayUInt64");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayFloat)
{
    static const float VALUE1 = -2.1f;
    static const float VALUE2 = 0.0f;
    static const float VALUE3 = 2.1f;
    static const float VALUE4 = 222.1f;
    union
    {
        std::uint64_t v;
        double d;
    } u;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayFloat", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayFloat(MatcherMetaField(*fieldValue), std::vector<float>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayFloat");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testArrayDouble)
{
    static const double VALUE1 = -2.1;
    static const double VALUE2 = 0.0;
    static const double VALUE3 = 2.1;
    static const double VALUE4 = 222.1;
    union
    {
        std::uint64_t v;
        double d;
    } u;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayDouble", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayDouble(MatcherMetaField(*fieldValue), std::vector<double>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayDouble");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::string VALUE4 = "Foo";

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayString", "value");
    ASSERT_NE(fieldValue, nullptr);

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
    data.push_back(static_cast<char>(0xff));
    data.push_back(static_cast<char>(0xff));
    data.push_back(static_cast<char>(0xff));
    data.push_back(static_cast<char>(0xff));
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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStringMove(MatcherMetaField(*fieldValue), std::vector<std::string>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayString");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', '\0', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', '\n', '\0', 'd'};
    static const Bytes VALUE4 = {'F', '\t', '\0', 123, 12};

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayBytes", "value");
    ASSERT_NE(fieldValue, nullptr);

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
    data.push_back(static_cast<char>(0xff));
    data.push_back(static_cast<char>(0xff));
    data.push_back(static_cast<char>(0xff));
    data.push_back(static_cast<char>(0xff));
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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayBytesMove(MatcherMetaField(*fieldValue), std::vector<Bytes>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayBytes");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayStruct)
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
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), std::string(VALUE1_STRING))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldUInt32), VALUE1_LAST)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), VALUE2_INT32)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructInt32))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldString), std::string(VALUE2_STRING))).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructString))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldUInt32), VALUE2_LAST)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldStructWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldStruct))).Times(1);
        EXPECT_CALL(mockVisitor, enterUInt32(MatcherMetaField(*fieldLastValue), LAST_VALUE)).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayStruct");
    EXPECT_EQ(res, true);
}


TEST_F(TestParserQt, testFixedArrayStruct)
{
    const MetaField* fieldFixedArrayString = MetaDataGlobal::instance().getField("test.TestFixedArrayStruct", "a1");
    const MetaField* fieldFixedArrayInner = MetaDataGlobal::instance().getField("test.TestFixedArrayStruct", "a2");
    const MetaField* fieldFixedArrayInnerWithoutArray = MetaDataGlobal::instance().getArrayField("test.TestFixedArrayStruct", "a2");
    const MetaField* fieldFixedArrayUInt32 = MetaDataGlobal::instance().getField("test.TestFixedArrayStruct", "a3");
    const MetaField* fieldInnerString = MetaDataGlobal::instance().getField("test.TestInnerFixedArrayStruct", "b1");
    const MetaField* fieldInnerFixedArray = MetaDataGlobal::instance().getField("test.TestInnerFixedArrayStruct", "b2");
    const MetaField* fieldInnerFixedArrayWithoutArray = MetaDataGlobal::instance().getArrayField("test.TestInnerFixedArrayStruct", "b2");
    const MetaField* fieldInnerInt32 = MetaDataGlobal::instance().getField("test.TestInnerFixedArrayStruct", "b3");
    const MetaField* fieldInt32 = MetaDataGlobal::instance().getField("test.TestInt32", "value");

    ASSERT_NE(fieldFixedArrayString, nullptr);
    ASSERT_NE(fieldFixedArrayInner, nullptr);
    ASSERT_NE(fieldFixedArrayInnerWithoutArray, nullptr);
    ASSERT_NE(fieldFixedArrayUInt32, nullptr);
    ASSERT_NE(fieldInnerString, nullptr);
    ASSERT_NE(fieldInnerFixedArray, nullptr);
    ASSERT_NE(fieldInnerFixedArrayWithoutArray, nullptr);
    ASSERT_NE(fieldInnerInt32, nullptr);
    ASSERT_NE(fieldInt32, nullptr);

    std::string data;
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x02);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x03);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x04);

    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x05);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x06);

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);

        EXPECT_CALL(mockVisitor, enterArrayStringMove(MatcherMetaField(*fieldFixedArrayString), std::vector<std::string>({ "", ""}))).Times(1);

        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*fieldFixedArrayInner))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldFixedArrayInnerWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldInnerString), std::string(""))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*fieldInnerFixedArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldInnerFixedArrayWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), 1)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldInnerFixedArrayWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldInnerFixedArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInnerInt32), 2)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldFixedArrayInnerWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldFixedArrayInnerWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterString(MatcherMetaField(*fieldInnerString), std::string(""))).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayStruct(MatcherMetaField(*fieldInnerFixedArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterStruct(MatcherMetaField(*fieldInnerFixedArrayWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInt32), 3)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldInnerFixedArrayWithoutArray))).Times(1);
        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldInnerFixedArray))).Times(1);
        EXPECT_CALL(mockVisitor, enterInt32(MatcherMetaField(*fieldInnerInt32), 4)).Times(1);
        EXPECT_CALL(mockVisitor, exitStruct(MatcherMetaField(*fieldFixedArrayInnerWithoutArray))).Times(1);

        EXPECT_CALL(mockVisitor, exitArrayStruct(MatcherMetaField(*fieldFixedArrayInner))).Times(1);

        EXPECT_CALL(mockVisitor, enterArrayUInt32(MatcherMetaField(*fieldFixedArrayUInt32), std::vector<std::uint32_t>({ 5, 6 }))).Times(1);

        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestFixedArrayStruct");
    EXPECT_EQ(res, true);
}



TEST_F(TestParserQt, testArrayEnum)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::int32_t VALUE4 = 123;

    const MetaField* fieldValue = MetaDataGlobal::instance().getField("test.TestArrayEnum", "value");
    ASSERT_NE(fieldValue, nullptr);

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

    MockIParserVisitor mockVisitor;

    {
        testing::InSequence seq;
        EXPECT_CALL(mockVisitor, startStruct(_)).Times(1);
        EXPECT_CALL(mockVisitor, enterArrayEnum(MatcherMetaField(*fieldValue), std::vector<std::int32_t>({VALUE1, VALUE2, VALUE3, VALUE4}))).Times(1);
        EXPECT_CALL(mockVisitor, finished()).Times(1);
    }

    ParserQt parser(mockVisitor, data.data(), data.size());
    bool res = parser.parseStruct("test.TestArrayEnum");
    EXPECT_EQ(res, true);
}

