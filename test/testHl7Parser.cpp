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
#include "MockIHl7ParserVisitor.h"

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
//        m_parser = std::unique_ptr<Hl7Parser>(new Hl7Parser(m_mockHl7ParserVisitor));
    }

    virtual void TearDown()
    {
    }

    std::unique_ptr<Hl7Parser> m_parser;
//    MockIHl7ParserVisitor m_mockHl7ParserVisitor;
};



TEST_F(TestHl7Parser, test)
{
    std::string hl7 = "H";
    {
        Sequence seq;
        //EXPECT_CALL(m_mockHl7ParserVisitor, enterObject()).Times(1);
        //EXPECT_CALL(m_mockHl7ParserVisitor, syntaxError(json.c_str() + 5, _)).Times(1);
        //EXPECT_CALL(m_mockHl7ParserVisitor, finished()).Times(1);
    }
//    const char* res = m_parser->parse(hl7.c_str());
//    EXPECT_EQ(res, nullptr);
}

