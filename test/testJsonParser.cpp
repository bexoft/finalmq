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


#include "finalmq/helpers/BexDefines.h"
#include "finalmq/json/JsonParser.h"
#include "MockIJsonParserVisitor.h"

using ::testing::_;
using ::testing::StrEq;
using ::testing::Sequence;

using namespace finalmq;

class TestJsonParser: public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_parser = std::unique_ptr<JsonParser>(new JsonParser(m_mockJsonParserVisitor));
    }

    virtual void TearDown()
    {
    }

    std::unique_ptr<JsonParser> m_parser;
    MockIJsonParserVisitor m_mockJsonParserVisitor;
};



TEST_F(TestJsonParser, testStringSize)
{
    std::string json = "{\"key\":23}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+5,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str(), 5);
    EXPECT_EQ(res, nullptr);
}



TEST_F(TestJsonParser, testEmpty)
{
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(StrEq(""), _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const std::string json = "";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}



TEST_F(TestJsonParser, testNull)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterNull()).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "null";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testNullWithSpaces)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterNull()).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "\t\n\r null\t\n\r ";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testNullEarlyEnd)
{
    std::string json = "nul";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+3, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testNullWrongChar)
{
    std::string json = "nulo";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+3, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testTrue)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterBool(true)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "true";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testFalse)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterBool(false)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "false";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testNoDigit)
{
    std::string json = "Hello";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str(), _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testNegativeNoDigit)
{
    std::string json = "-Hello";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+1, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}



TEST_F(TestJsonParser, testInt32)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterInt32(-1234567)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "-1234567";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testInt32WithSpaces)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterInt32(-1234567)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "\t\n\r -1234567\t\n\r ";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testInt32WrongFormat)
{
    std::string json = "-123-4567";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+4, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testUInt32)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(1234567)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "1234567";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}



TEST_F(TestJsonParser, testUInt32WrongFormat)
{
    std::string json = "123-4567";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+3, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testInt64)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterInt64(-123456789012)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "-123456789012";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}



TEST_F(TestJsonParser, testInt64WrongFormat)
{
    std::string json = "-123-456789012";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+4, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testUInt64)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterUInt64(123456789012)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "123456789012";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}



TEST_F(TestJsonParser, testUInt64WrongFormat)
{
    std::string json = "123-456789012";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+3, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}



TEST_F(TestJsonParser, testDouble)
{
    EXPECT_CALL(m_mockJsonParserVisitor, enterDouble(0.123456789012)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    std::string json = "0.123456789012";
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}



TEST_F(TestJsonParser, testDoubleWrongFormat)
{
    std::string json = "0.12.34.56789012";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+4, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}



TEST_F(TestJsonParser, testEmptyString)
{
    std::string json = "\"\"";
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(json.c_str()+1, 0)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testEmptyStingEarlyEnd)
{
    std::string json = "\"";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+1, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testStingEarlyEnd)
{
    std::string json = "\"Hello ";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+json.size(), _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testEmptyStringWithSpaces)
{
    std::string json = "\t\n\r \"\"\t\n\r ";
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(json.c_str()+5, 0)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testString)
{
    std::string json = "\"Hello World\"";
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(json.c_str()+1, 11)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testStringSimpleEscape)
{
    std::string json = "\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t \"";
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(std::string(" \" \\ / \b \f \n \r \t "))).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testStringSimpleEscapeEarlyEnd)
{
    std::string json = "\"\\t";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+json.size(), _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testStringEscapeU16)
{
    std::string json = "\"\\u00E4\"";
    std::string cmp = {(char)0xc3, (char)0xa4};
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(std::move(cmp))).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testStringEscapeU16Small)
{
    std::string json = "\"\\u00e4\"";
    std::string cmp = {(char)0xc3, (char)0xa4};
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(std::move(cmp))).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testStringEscapeU16Invalid)
{
    std::string json = "\"\\u00H4\"";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+5, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testStringEscapeU16EarlyEnd)
{
    std::string json = "\"\\u00E";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+json.size(), _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testStringEscapeU32)
{
    std::string json = "\"\\uD802\\uDDAA\""; // codepoint=68010=0x109aa
    std::string cmp = {(char)0xf0, (char)0x90, (char)0xa6, (char)0xaa};
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(std::move(cmp))).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testStringEscapeU32Invalid)
{
    std::string json = "\"\\uD801\\uDEH1\"";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+11, _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}


TEST_F(TestJsonParser, testStringEscapeU32EarlyEnd)
{
    std::string json = "\"\\uD801\\uDE01";
    EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+json.size(), _)).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}



TEST_F(TestJsonParser, testStringUnknownEscape)
{
    std::string json = "\"\\z\"";
    EXPECT_CALL(m_mockJsonParserVisitor, enterString(std::string("\\z"))).Times(1);
    EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}


TEST_F(TestJsonParser, testEmptyArray)
{
    std::string json = "[]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testEmptyArrayWithSpaces)
{
    std::string json = "\t\n\r [\t\n\r ]\t\n\r ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testArrayWithOneValue)
{
    std::string json = "[123]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
TEST_F(TestJsonParser, testArrayWithOneValueWithSpaces)
{
    std::string json = "\t\n\r [\t\n\r 123\t\n\r ]\t\n\r ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
TEST_F(TestJsonParser, testArrayWithTwoValues)
{
    std::string json = "[123,\"Hello\"]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterString(json.c_str()+6, 5)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
TEST_F(TestJsonParser, testArrayWithTwoValuesWithSpaces)
{
    std::string json = " [ 123 , \"Hello\" ] ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterString(json.c_str()+6+4, 5)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
TEST_F(TestJsonParser, testArrayTolerateLastCommaTwoValues)
{
    std::string json = "[123,\"Hello\",]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterString(json.c_str()+6, 5)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testArrayTolerateLastCommaWithSpacesTwoValues)
{
    std::string json = " [ 123 , \"Hello\" , ] ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterString(json.c_str()+6+4, 5)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testArrayTolerateLastCommaOneValue)
{
    std::string json = "[123,]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testArrayTolerateLastCommaOneValueWithSpaces)
{
    std::string json = " [ 123 , ] ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testEmptyArrayOneComma)
{
    std::string json = "[,]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+1, _)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testEmptyArrayOneCommaWithSpaces)
{
    std::string json = " [ , ] ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+1+2, _)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testArrayTwoCommas)
{
    std::string json = "[123,,]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+5, _)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testArrayTwoCommasWithSpaces)
{
    std::string json = " [ 123 , , ] ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+5+4, _)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testArrayMissingComma)
{
    std::string json = "[123 12]";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+5, _)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testArrayEarlyEnd)
{
    std::string json = "[123,";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterArray()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+5, _)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testEmptyObjectNoEnd)
{
    const std::string json = "{";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(StrEq(""), _)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}

TEST_F(TestJsonParser, testEmptyObject)
{
    std::string json = "{}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testEmptyObjectWithSpaces)
{
    std::string json = "\t\n\r {\t\n\r }\t\n\r ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}

TEST_F(TestJsonParser, testObjectWithOneEntry)
{
    std::string json = "{\"key\":123}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
TEST_F(TestJsonParser, testObjectWithOneEntryWithSpaces)
{
    std::string json = " { \"key\" : 123 } ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
TEST_F(TestJsonParser, testObjectMissingColen)
{
    std::string json = "{\"key\" 123}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+7,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectMissingKeyString)
{
    std::string json = "{key:123}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+1,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectMissingKeyStringWithSpaces)
{
    std::string json = " { key : 123 } ";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+3,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectMissingComma)
{
    std::string json = "{\"key\":123 \"next\":123}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+11,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectWithOneEntryTolerateLastComma)
{
    std::string json = "{\"key\":123,}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
TEST_F(TestJsonParser, testObjectTwoCommas)
{
    std::string json = "{\"key\":123,,\"next\":123}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+11,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectTwoCommasWithSpaces)
{
    std::string json = "{\"key\":123 , ,\"next\":123}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+11+2,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectEarlyEndInKey)
{
    std::string json = "{\"key";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+5,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectInvalidValue)
{
    std::string json = "{\"key\":12-12";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(json.c_str()+2, 3)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, syntaxError(json.c_str()+9,_)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_EQ(res, nullptr);
}
TEST_F(TestJsonParser, testObjectKeyWithEscape)
{
    std::string json = "{\"ke\\ty\":123}";
    {
        Sequence seq;
        EXPECT_CALL(m_mockJsonParserVisitor, enterObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterKey(std::string("ke\ty"))).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, enterUInt32(123)).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, exitObject()).Times(1);
        EXPECT_CALL(m_mockJsonParserVisitor, finished()).Times(1);
    }
    const char* res = m_parser->parse(json.c_str());
    EXPECT_NE(res, nullptr);
    ssize_t size = res - json.c_str();
    EXPECT_EQ(size, json.size());
}
