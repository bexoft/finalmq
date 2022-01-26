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


#include "finalmq/logger/LogStream.h"


using testing::StrEq;
using testing::_;

using namespace finalmq;


static const char* MODULENAME = "TestModule";



class MockFuncLogEvent
{
public:
    MOCK_METHOD(void, func, (const LogContext& context, const char* text));
};



class TestLogger : public testing::Test
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




TEST_F(TestLogger, test)
{
    MockFuncLogEvent mock;
    Logger::instance().registerConsumer([&mock](const LogContext& context, const char* text){
        mock.func(context, text);
    });

    EXPECT_CALL(mock, func(_, StrEq("Hello"))).Times(1);
    streamInfo << "Hello";

    Logger::setInstance({});

    EXPECT_CALL(mock, func(_, _)).Times(0);
    streamInfo << "World";
}
