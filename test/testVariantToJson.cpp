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
#include "finalmq/jsonvariant/VariantToJson.h"
#include "finalmq/variant/Variant.h"
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


class TestVariantToJson : public testing::Test
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

        m_jsonToVariant = std::make_unique<VariantToJson>(m_mockBuffer, MAX_BLOCK_SIZE);
    }

    virtual void TearDown()
    {
    }

    std::unique_ptr<VariantToJson>      m_jsonToVariant;
    String                              m_data;
    MockIZeroCopyBuffer                 m_mockBuffer;
};



TEST_F(TestVariantToJson, testNull)
{
    Variant variant;
    m_jsonToVariant->parse(variant);
    EXPECT_EQ(m_data, "null");
}


TEST_F(TestVariantToJson, testBoolTrue)
{
    static const bool VALUE = true;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "true");
}


TEST_F(TestVariantToJson, testBoolFalse)
{
    static const bool VALUE = false;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "false");
}


TEST_F(TestVariantToJson, testInt32_0)
{
    static const std::int32_t VALUE = 0;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "0");
}

TEST_F(TestVariantToJson, testInt32_1234567890)
{
    static const std::int32_t VALUE = 1234567890;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "1234567890");
}

TEST_F(TestVariantToJson, testInt32_n1234567890)
{
    static const std::int32_t VALUE = -1234567890;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "-1234567890");
}

TEST_F(TestVariantToJson, testUInt32_4234567890)
{
    static const std::uint32_t VALUE = 4234567890;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "4234567890");
}

TEST_F(TestVariantToJson, testInt64_0)
{
    static const std::int64_t VALUE = 0;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "0");
}

TEST_F(TestVariantToJson, testInt64_1234567890123456789)
{
    static const std::int64_t VALUE = 1234567890123456789;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "1234567890123456789");
}

TEST_F(TestVariantToJson, testInt64_n1234567890123456789)
{
    static const std::int64_t VALUE = -1234567890123456789;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "-1234567890123456789");
}

TEST_F(TestVariantToJson, testUInt64_2234567890123456789)
{
    static const std::uint64_t VALUE = 2234567890123456789;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "2234567890123456789");
}

TEST_F(TestVariantToJson, testDouble_22345678901234567)
{
    static const double VALUE = -0.22345678901234567;

    m_jsonToVariant->parse(VALUE);
    m_data.resize(m_data.size() - 1);   // remove last digit
    EXPECT_EQ(m_data, "-0.2234567890123456");
}

TEST_F(TestVariantToJson, testDouble_2234567e123)
{
    static const double VALUE = 2234567e123;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "2.234567e129");
}

TEST_F(TestVariantToJson, testDouble_2234567en123)
{
    static const double VALUE = 2234567e-123;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "2.234567e-117");
}

TEST_F(TestVariantToJson, testString)
{
    static const std::string VALUE = "Hello";

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "\"Hello\"");
}

TEST_F(TestVariantToJson, testStringEmpty)
{
    static const std::string VALUE = "";

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "\"\"");
}


TEST_F(TestVariantToJson, testStringEscape)
{
    static const std::string VALUE = "\\\"\r\n\f\b\t\x01";

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, std::string("\"\\\\\\\"\\r\\n\\f\\b\\t\\u0001\""));
}


TEST_F(TestVariantToJson, testStringEscapeU16)
{
    static const std::string VALUE = { (char)0xc3, (char)0xa4 };

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "\"\\u00e4\"");
}

TEST_F(TestVariantToJson, testStringEscapeU32)
{
    static const std::string VALUE = { (char)0xf0, (char)0x90, (char)0xa6, (char)0xaa };

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "\"\\ud802\\uddaa\"");
}

TEST_F(TestVariantToJson, testArray)
{
    static const VariantList VALUE;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "[]");
}

TEST_F(TestVariantToJson, testObject)
{
    static const VariantStruct VALUE;

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "{}");
}




TEST_F(TestVariantToJson, testObjectExample)
{
    static const VariantStruct VALUE{ {"name", "Elvis"}, {"age", 42u}, {"arr", VariantList{1.234, 2.345, VariantStruct{{"x",123u}}, 3.456}}, {"struct", VariantStruct{{"a", VariantList{"Hello", 45u}},{"b", 23u}}}};

    m_jsonToVariant->parse(VALUE);
    EXPECT_EQ(m_data, "{\"name\":\"Elvis\",\"age\":42,\"arr\":[1.234,2.345,{\"x\":123},3.456],\"struct\":{\"a\":[\"Hello\",45],\"b\":23}}");
}
