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
#include "finalmq/json/JsonBuilder.h"
#include "MockIZeroCopyBuffer.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::Sequence;

using namespace finalmq;


struct String : public std::string
{
    void resize(size_t size)
    {
        std::string::resize(size);
    }
};


class TestJsonBuilder: public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        static const int MAX_BLOCK_SIZE = 1024;

        m_data.resize(MAX_BLOCK_SIZE);
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE, _)).Times(1).WillOnce(Return((char*)m_data.data()));
        EXPECT_CALL(m_mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&m_data, &String::resize));
        EXPECT_CALL(m_mockBuffer, getRemainingSize()).WillRepeatedly(Return(0));

        m_builder = std::unique_ptr<JsonBuilder>(new JsonBuilder(m_mockBuffer, MAX_BLOCK_SIZE));
    }

    virtual void TearDown()
    {
    }

    std::unique_ptr<IJsonParserVisitor> m_builder;
    String                              m_data;
    MockIZeroCopyBuffer                 m_mockBuffer;
};



TEST_F(TestJsonBuilder, testNull)
{
    m_builder->enterNull();
    m_builder->finished();
    EXPECT_EQ(m_data, "null");
}


TEST_F(TestJsonBuilder, testBoolTrue)
{
    static const bool VALUE = true;

    m_builder->enterBool(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "true");
}


TEST_F(TestJsonBuilder, testBoolFalse)
{
    static const bool VALUE = false;

    m_builder->enterBool(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "false");
}


TEST_F(TestJsonBuilder, testInt32_0)
{
    static const std::int32_t VALUE = 0;

    m_builder->enterInt32(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "0");
}

TEST_F(TestJsonBuilder, testInt32_1234567890)
{
    static const std::int32_t VALUE = 1234567890;

    m_builder->enterInt32(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "1234567890");
}

TEST_F(TestJsonBuilder, testInt32_n1234567890)
{
    static const std::int32_t VALUE = -1234567890;

    m_builder->enterInt32(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "-1234567890");
}

TEST_F(TestJsonBuilder, testUInt32_4234567890)
{
    static const std::uint32_t VALUE = 4234567890;

    m_builder->enterUInt32(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "4234567890");
}


TEST_F(TestJsonBuilder, testInt64_0)
{
    static const std::int64_t VALUE = 0;

    m_builder->enterInt64(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "0");
}

TEST_F(TestJsonBuilder, testInt64_1234567890123456789)
{
    static const std::int64_t VALUE = 1234567890123456789;

    m_builder->enterInt64(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "1234567890123456789");
}

TEST_F(TestJsonBuilder, testInt64_n1234567890123456789)
{
    static const std::int64_t VALUE = -1234567890123456789;

    m_builder->enterInt64(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "-1234567890123456789");
}

TEST_F(TestJsonBuilder, testUInt64_2234567890123456789)
{
    static const std::uint64_t VALUE = 2234567890123456789;

    m_builder->enterUInt64(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "2234567890123456789");
}

TEST_F(TestJsonBuilder, testDouble_22345678901234567)
{
    static const double VALUE = -0.22345678901234567;

    m_builder->enterDouble(VALUE);
    m_builder->finished();
    m_data.resize(m_data.size() - 1);   // remove last digit
    EXPECT_EQ(m_data, "-0.2234567890123456");
}

TEST_F(TestJsonBuilder, testDouble_2234567e123)
{
    static const double VALUE = 2234567e123;

    m_builder->enterDouble(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "2.234567e129");
}

TEST_F(TestJsonBuilder, testDouble_2234567en123)
{
    static const double VALUE = 2234567e-123;

    m_builder->enterDouble(VALUE);
    m_builder->finished();
    EXPECT_EQ(m_data, "2.234567e-117");
}

TEST_F(TestJsonBuilder, testString)
{
    static const std::string VALUE = "Hello";

    m_builder->enterString(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"Hello\"");
}

TEST_F(TestJsonBuilder, testStringEmpty)
{
    static const std::string VALUE = "";

    m_builder->enterString(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"\"");
}


TEST_F(TestJsonBuilder, testStringEscape)
{
    static const std::string VALUE = "\\\"\r\n\f\b\t\x01";

    m_builder->enterString(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, std::string("\"\\\\\\\"\\r\\n\\f\\b\\t\\u0001\""));
}


TEST_F(TestJsonBuilder, testStringEscapeU16)
{
    static const std::string VALUE = {(char)0xc3, (char)0xa4};

    m_builder->enterString(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"\\u00e4\"");
}

TEST_F(TestJsonBuilder, testStringEscapeU32)
{
    static const std::string VALUE = {(char)0xf0, (char)0x90, (char)0xa6, (char)0xaa};

    m_builder->enterString(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"\\ud802\\uddaa\"");
}

TEST_F(TestJsonBuilder, testArray)
{
    m_builder->enterArray();
    m_builder->exitArray();
    m_builder->finished();
    EXPECT_EQ(m_data, "[]");
}

TEST_F(TestJsonBuilder, testObject)
{
    m_builder->enterObject();
    m_builder->exitObject();
    m_builder->finished();
    EXPECT_EQ(m_data, "{}");
}


TEST_F(TestJsonBuilder, testKey)
{
    static const std::string VALUE = "Hello";

    m_builder->enterKey(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"Hello\":");
}

TEST_F(TestJsonBuilder, testKeyEmpty)
{
    static const std::string VALUE = "";

    m_builder->enterKey(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"\":");
}


TEST_F(TestJsonBuilder, testKeyEscape)
{
    static const std::string VALUE = "\\\"\r\n\f\b\t\x01";

    m_builder->enterKey(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, std::string("\"\\\\\\\"\\r\\n\\f\\b\\t\\u0001\":"));
}


TEST_F(TestJsonBuilder, testKeyEscapeU16)
{
    static const std::string VALUE = {(char)0xc3, (char)0xa4};

    m_builder->enterKey(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"\\u00e4\":");
}

TEST_F(TestJsonBuilder, testKeyEscapeU32)
{
    static const std::string VALUE = {(char)0xf0, (char)0x90, (char)0xa6, (char)0xaa};

    m_builder->enterKey(VALUE.c_str(), VALUE.size());
    m_builder->finished();
    EXPECT_EQ(m_data, "\"\\ud802\\uddaa\":");
}

TEST_F(TestJsonBuilder, testObjectExample)
{
    static const std::string KEY1 = "name";
    static const std::string NAME = "Elvis";
    static const std::string KEY2 = "age";
    static const std::string KEY3 = "arr";

    m_builder->enterObject();
    m_builder->enterKey(KEY1.c_str(), KEY1.size());
    m_builder->enterString(NAME.c_str(), NAME.size());
    m_builder->enterKey(KEY2.c_str(), KEY2.size());
    m_builder->enterInt32(42);
    m_builder->enterKey(KEY3.c_str(), KEY3.size());
    m_builder->enterArray();
    m_builder->enterDouble(1.234);
    m_builder->enterDouble(2.345);
    m_builder->enterDouble(3.456);
    m_builder->exitArray();
    m_builder->exitObject();
    m_builder->finished();
    EXPECT_EQ(m_data, "{\"name\":\"Elvis\",\"age\":42,\"arr\":[1.234,2.345,3.456]}");
}
