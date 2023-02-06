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
#include "finalmq/hl7/Hl7Builder.h"
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


class TestHl7Builder: public testing::Test
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

        m_builder = std::unique_ptr<Hl7Builder>(new Hl7Builder(m_mockBuffer, MAX_BLOCK_SIZE));
    }

    virtual void TearDown()
    {
    }

    std::unique_ptr<IHl7BuilderVisitor>  m_builder;
    String                              m_data;
    MockIZeroCopyBuffer                 m_mockBuffer;
};


/*
TEST_F(TestHl7Builder, testTwoEmptySegments)
{
    m_builder->enterStruct();
    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->exitStruct();
    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->exitStruct();
    m_builder->finished();
    EXPECT_EQ(m_data, "MSH|^~\\&\rTST\r");
}

TEST_F(TestHl7Builder, testSubStructs)
{
    m_builder->enterStruct();
    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->enterString("a1");
    m_builder->enterString("a2");

    m_builder->enterStruct();
    m_builder->enterString("b1");
    m_builder->enterString("b2");

    m_builder->enterStruct();
    m_builder->enterString("c1");
    m_builder->enterString("c2");
    m_builder->enterString("c3");
    m_builder->enterString("c4");
    m_builder->exitStruct();

    m_builder->enterString("b3");
    m_builder->enterString("b4");
    m_builder->exitStruct();

    m_builder->enterString("a3");
    m_builder->enterString("a4");
    m_builder->exitStruct();

    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->exitStruct();
    m_builder->finished();
    EXPECT_EQ(m_data, "MSH|^~\\&|a1|a2|b1^b2^c1&c2&c3&c4^b3^b4|a3|a4\rTST\r");
}

TEST_F(TestHl7Builder, testExit3StructsFromInnerLevel)
{
    m_builder->enterStruct();
    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->enterString("a1");
    m_builder->enterString("a2");

    m_builder->enterStruct();
    m_builder->enterString("b1");
    m_builder->enterString("b2");

    m_builder->enterStruct();
    m_builder->enterString("c1");
    m_builder->enterString("c2");
    m_builder->enterString("c3");
    m_builder->enterString("c4");
    m_builder->exitStruct();
    m_builder->exitStruct();
    m_builder->exitStruct();

    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->exitStruct();
    m_builder->finished();
    EXPECT_EQ(m_data, "MSH|^~\\&|a1|a2|b1^b2^c1&c2&c3&c4\rTST\r");
}

TEST_F(TestHl7Builder, testExit2StructsFromInnerLevel)
{
    m_builder->enterStruct();
    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->enterString("a1");
    m_builder->enterString("a2");

    m_builder->enterStruct();
    m_builder->enterString("b1");
    m_builder->enterString("b2");

    m_builder->enterStruct();
    m_builder->enterString("c1");
    m_builder->enterString("c2");
    m_builder->enterString("c3");
    m_builder->enterString("c4");
    m_builder->exitStruct();
    m_builder->exitStruct();

    m_builder->enterString("a3");
    m_builder->exitStruct();

    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->exitStruct();
    m_builder->finished();
    EXPECT_EQ(m_data, "MSH|^~\\&|a1|a2|b1^b2^c1&c2&c3&c4|a3\rTST\r");
}


TEST_F(TestHl7Builder, testSubStructsRemoveEmpty)
{
    m_builder->enterStruct();
    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->enterString("a1");
    m_builder->enterString("a2");
    m_builder->enterEmpty();
    m_builder->enterEmpty();

    m_builder->enterStruct();
    m_builder->enterString("b1");
    m_builder->enterString("b2");
    m_builder->enterEmpty();
    m_builder->enterEmpty();

    m_builder->enterStruct();
    m_builder->enterString("c1");
    m_builder->enterString("c2");
    m_builder->enterString("c3");
    m_builder->enterString("c4");
    m_builder->enterEmpty();
    m_builder->enterEmpty();
    m_builder->exitStruct();

    m_builder->enterEmpty();
    m_builder->enterEmpty();
    m_builder->enterString("b3");
    m_builder->enterString("b4");
    m_builder->enterEmpty();
    m_builder->enterEmpty();
    m_builder->exitStruct();

    m_builder->enterEmpty();
    m_builder->enterEmpty();
    m_builder->enterString("a3");
    m_builder->enterString("a4");
    m_builder->enterEmpty();
    m_builder->enterEmpty();
    m_builder->exitStruct();

    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->enterEmpty();
    m_builder->enterEmpty();
    m_builder->exitStruct();

    m_builder->finished();
    EXPECT_EQ(m_data, "MSH|^~\\&|a1|a2|||b1^b2^^^c1&c2&c3&c4^^^b3^b4|||a3|a4\rTST\r");
}


TEST_F(TestHl7Builder, testArray)
{
    m_builder->enterStruct();
    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->enterString("a1")
        ;
    m_builder->enterArray();
    m_builder->enterString("a2.1");
    m_builder->enterString("a2.2");
    m_builder->enterString("a2.3");
    m_builder->exitArray();

    m_builder->enterStruct();
    m_builder->enterString("b1");

    m_builder->enterArray();
    m_builder->enterString("b2.1");
    m_builder->enterString("b2.2");
    m_builder->enterString("b2.3");
    m_builder->enterEmpty();
    m_builder->enterEmpty();
    m_builder->exitArray();

    m_builder->enterStruct();
    m_builder->enterString("c1");

    m_builder->enterArray();
    m_builder->enterString("c2.1");
    m_builder->enterString("c2.2");
    m_builder->enterString("c2.3");
    m_builder->exitArray();

    m_builder->enterString("c3");
    m_builder->enterString("c4");
    m_builder->exitStruct();

    m_builder->enterString("b3");
    m_builder->enterString("b4");
    m_builder->exitStruct();

    m_builder->enterString("a3");
    m_builder->enterString("a4");
    m_builder->exitStruct();

    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->exitStruct();
    m_builder->finished();
    EXPECT_EQ(m_data, "MSH|^~\\&|a1|a2.1~a2.2~a2.3|b1^b2.1~b2.2~b2.3~~^c1&c2.1~c2.2~c2.3&c3&c4^b3^b4|a3|a4\rTST\r");
}


TEST_F(TestHl7Builder, testTypes)
{
    m_builder->enterStruct();
    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->enterUInt64(123);
    m_builder->enterDouble(0.123);

    m_builder->enterStruct();
    m_builder->enterInt64(-2);
    m_builder->enterInt64(12);

    m_builder->enterStruct();
    m_builder->enterInt64(1);
    m_builder->enterInt64(2);
    m_builder->enterInt64(-3);
    m_builder->enterInt64(-4);
    m_builder->exitStruct();

    m_builder->enterDouble(1.1);
    m_builder->enterDouble(2.1);
    m_builder->exitStruct();

    m_builder->enterUInt64(1);
    m_builder->enterUInt64(2);
    m_builder->exitStruct();

    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->exitStruct();
    m_builder->finished();
    EXPECT_EQ(m_data, "MSH|^~\\&|123|0.123|-2^12^1&2&-3&-4^1.1^2.1|1|2\rTST\r");
}

TEST_F(TestHl7Builder, testEscape)
{
    m_builder->enterStruct();

    m_builder->enterString("MSH");
    m_builder->enterString("");
    m_builder->enterString("");
    m_builder->enterString("\r\n\t|^&~\\");
    m_builder->enterString("a");
    m_builder->exitStruct();

    m_builder->enterStruct();
    m_builder->enterString("TST");
    m_builder->exitStruct();

    m_builder->finished();

    EXPECT_EQ(m_data, "MSH|^~\\&|\\X0D\\\\X0A\\\\X09\\\\F\\\\S\\\\T\\\\R\\\\E\\|a\rTST\r");
}
*/
