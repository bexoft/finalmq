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


#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/hl7/Hl7Parser.h"

using ::testing::_;
using ::testing::StrEq;
using ::testing::Sequence;

using namespace finalmq;




class TestHl7Parser : public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_parser = std::unique_ptr<Hl7Parser>(new Hl7Parser());
    }

    virtual void TearDown()
    {
    }

    std::unique_ptr<Hl7Parser> m_parser;
};



TEST_F(TestHl7Parser, testStartParseWrongMessageStart)
{
    std::string hl7 = "WrongMessageStart";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(false, res);
}

TEST_F(TestHl7Parser, testStartParseMessageTooShort)
{
    std::string hl7 = "MSG|";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(false, res);
}

TEST_F(TestHl7Parser, testStartParseMessageTooShortWithSize)
{
    std::string hl7 = "MSG|";
    bool res = m_parser->startParse(hl7.c_str(), hl7.size());
    EXPECT_EQ(false, res);
}

TEST_F(TestHl7Parser, testStartParse)
{
    std::string hl7 = "MSG|^~\\&";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);
}

TEST_F(TestHl7Parser, testParseToken)
{
    std::string hl7 = "MSG|^~\\&|a\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    const char* pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 11, pos);

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");

    pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 11, pos);
}

TEST_F(TestHl7Parser, testParseTokenWithGivenSize)
{
    std::string hl7 = "MSG|^~\\&|a\x0d";
    bool res = m_parser->startParse(hl7.c_str(), hl7.size());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    const char* pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 11, pos);

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");

    pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 11, pos);
}

TEST_F(TestHl7Parser, testParseTokenWithStartCharacters)
{
    std::string hl7 = "\x0BMSG|^~\\&|a\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    const char* pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 12, pos);

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");

    pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 12, pos);
}

TEST_F(TestHl7Parser, testParseTokenWithStartAndEndCharacters)
{
    std::string hl7 = "\x0BMSG|^~\\&|a\x0d\x1c\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    const char* pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 12, pos);

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "\x1c");

    pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 14, pos);

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");

    pos = m_parser->getCurrentPosition();
    EXPECT_EQ(hl7.c_str() + 14, pos);
}

TEST_F(TestHl7Parser, testParseTokenLevel2)
{
    std::string hl7 = "MSG|^~\\&|a^b|a^b\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "b");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "b");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenLevel3)
{
    std::string hl7 = "MSG|^~\\&|a^b1&b2|\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(3, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "b2");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenLevel3to2)
{
    std::string hl7 = "MSG|^~\\&|a^b1&b2^c|\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(3, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "b2");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "c");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenLevelSkipStructLevel3to2)
{
    std::string hl7 = "MSG|^~\\&|a^b1&b2^c|\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(3, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseTillEndOfStruct(2);
    EXPECT_EQ(2, level);

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "c");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenLevelSkipStructLevel3to1)
{
    std::string hl7 = "MSG|^~\\&|a^b1&b2^c|d\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(3, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseTillEndOfStruct(1);
    EXPECT_EQ(1, level);

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "d");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenLevelSkipStructLevel3to1Last)
{
    std::string hl7 = "MSG|^~\\&|a^b1&b2^c\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(3, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseTillEndOfStruct(1);
    EXPECT_EQ(0, level);

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenLevelSkipStructLevel3to1LastEnd)
{
    std::string hl7 = "MSG|^~\\&|a^b1&b2^c";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(3, token);
    EXPECT_EQ(3, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseTillEndOfStruct(1);
    EXPECT_EQ(-1, level);
}


TEST_F(TestHl7Parser, testParseTokenWrongLevel)
{
    std::string hl7 = "MSG|^~\\&|a^b1&b2^c|d\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    // wrong level
    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "c");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "d");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenArray)
{
    std::string hl7 = "MSG|^~\\&|a^b1~b2~~^c|d\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    std::vector<std::string> arr;
    level = m_parser->parseTokenArray(2, arr);
    EXPECT_EQ(2, level);
    EXPECT_EQ(4, arr.size());
    EXPECT_EQ(arr[0], "b1");
    EXPECT_EQ(arr[1], "b2");
    EXPECT_EQ(arr[2], "");
    EXPECT_EQ(arr[3], "");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "c");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "d");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenArrayWrongLevel)
{
    std::string hl7 = "MSG|^~\\&|a^b1~b2~~&b4^c|d\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    // wrong level
    std::vector<std::string> arr;
    level = m_parser->parseTokenArray(2, arr);
    EXPECT_EQ(2, level);
    EXPECT_EQ(2, level);
    EXPECT_EQ(4, arr.size());
    EXPECT_EQ(arr[0], "b1");
    EXPECT_EQ(arr[1], "b2");
    EXPECT_EQ(arr[2], "");
    EXPECT_EQ(arr[3], "");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "c");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "d");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenWrongArray)
{
    std::string hl7 = "MSG|^~\\&|a^b1~b2~~^c|d\x0d";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(2, level);
    EXPECT_EQ(token, "b1");

    level = m_parser->parseToken(2, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "c");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "d");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}


TEST_F(TestHl7Parser, testParseTokenEscape)
{
    std::string hl7 = "MSG|^~\\&|\\X0D\\\\X0A\\\\X09\\\\F\\\\S\\\\T\\\\R\\\\E\\|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "\r\n\t|^&~\\");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenEscapeSmallHexCode)
{
    std::string hl7 = "MSG|^~\\&|\\X0d\\|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "\r");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenEscapeWrong1)
{
    std::string hl7 = "MSG|^~\\&|\\X0D|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "\r");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenEscapeWrong2)
{
    std::string hl7 = "MSG|^~\\&|\\|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenEscapeWrong3)
{
    std::string hl7 = "MSG|^~\\&|\\F|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenEscapeWrong4)
{
    std::string hl7 = "MSG|^~\\&|\\Xwp\\|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenEscapeWrong5)
{
    std::string hl7 = "MSG|^~\\&|\\X0p\\|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}

TEST_F(TestHl7Parser, testParseTokenEscapeWrong6)
{
    std::string hl7 = "MSG|^~\\&|\\X0|a\rTST\r";
    bool res = m_parser->startParse(hl7.c_str());
    EXPECT_EQ(true, res);

    std::string token;
    int level;
    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "MSG");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "|^~\\&");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(1, level);
    EXPECT_EQ(token, "");

    level = m_parser->parseToken(1, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "a");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(0, level);
    EXPECT_EQ(token, "TST");

    level = m_parser->parseToken(0, token);
    EXPECT_EQ(-1, level);
    EXPECT_EQ(token, "");
}
