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


#include "finalmq/serializeproto/SerializerProto.h"
#include "finalmq/metadata/MetaData.h"
#include "MockIZeroCopyBuffer.h"
#include "test.pb.h"

//#include <thread>
//#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;

using namespace finalmq;


MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.type == metaField.type &&
            arg.typeName == metaField.typeName &&
            arg.name == metaField.name);
}


struct String : public std::string
{
    void resize(size_t size)
    {
        std::string::resize(size);
    }
};



class TestSerializerProto : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        static const int MAX_BLOCK_SIZE = 100;

        m_data.resize(MAX_BLOCK_SIZE);
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE, _)).Times(1).WillOnce(Return((char*)m_data.data()));
        EXPECT_CALL(m_mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&m_data, &String::resize));

        m_serializer = std::make_unique<SerializerProto>(m_mockBuffer, MAX_BLOCK_SIZE);
    }

    virtual void TearDown()
    {
    }

    String                          m_data;
    MockIZeroCopyBuffer             m_mockBuffer;
    std::unique_ptr<IParserVisitor> m_serializer;
};




TEST_F(TestSerializerProto, testBool)
{
    static const bool VALUE = true;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestBool", "", "", 0, 0});
    m_serializer->enterBool({MetaTypeId::TYPE_BOOL, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestBool", "", "", 0, 0});

    fmq::test::TestBool message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testInt32)
{
    static const std::int32_t VALUE = -2;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "", "", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "", "", 0});

    fmq::test::TestInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt32", "", "", 0});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt32", "", "", 0});

    fmq::test::TestUInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testInt64)
{
    static const std::int64_t VALUE = -2;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt64", "", "", 0});
    m_serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt64", "", "", 0});

    fmq::test::TestInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt64", "", "", 0});
    m_serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt64", "", "", 0});

    fmq::test::TestUInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}



TEST_F(TestSerializerProto, testFloat)
{
    static const float VALUE = -2.1f;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestFloat", "", "", 0});
    m_serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestFloat", "", "", 0});

    fmq::test::TestFloat message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testDouble)
{
    static const double VALUE = -2.1;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestDouble", "", "", 0});
    m_serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestDouble", "", "", 0});

    fmq::test::TestDouble message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testString)
{
    static const std::string VALUE = "Hello World";

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "", "", 0});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "", "", 0});

    fmq::test::TestString message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestBytes", "", "", 0});
    m_serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestBytes", "", "", 0});

    fmq::test::TestBytes message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    Bytes result(message.value().data(), message.value().data() + message.value().size());
    EXPECT_EQ(result, VALUE);
}



TEST_F(TestSerializerProto, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, 0}, VALUE_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, 0}, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 2}, VALUE_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});

    m_serializer->finished();

    fmq::test::TestStruct message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE_INT32);
    EXPECT_EQ(message.struct_string().value(), VALUE_STRING);
    EXPECT_EQ(message.last_value(), VALUE_UINT32);
}



TEST_F(TestSerializerProto, testEnum)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", "", "", 0});
    m_serializer->enterEnum({MetaTypeId::TYPE_ENUM, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", "", "", 0});

    fmq::test::TestEnum message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", "", "", 0});
    m_serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", "", "", 0});

    fmq::test::TestArrayBool message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<bool>(message.value().begin(), message.value().end()), VALUE);
}




TEST_F(TestSerializerProto, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", "", "", 0});
    m_serializer->enterArrayInt32({MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", "", "", 0});

    fmq::test::TestArrayInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int32_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", "", "", 0});
    m_serializer->enterArrayUInt32({MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", "", "", 0});

    fmq::test::TestArrayUInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::uint32_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", "", "", 0});
    m_serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", "", "", 0});

    fmq::test::TestArrayInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int64_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", "", "", 0});
    m_serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", "", "", 0});

    fmq::test::TestArrayUInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::uint64_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayFloat)
{
    static const float VALUE1 = -1.1f;
    static const float VALUE2 = 0.0f;
    static const float VALUE3 = 1.1f;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", "", "", 0});
    m_serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", "", "", 0});

    fmq::test::TestArrayFloat message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<float>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", "", "", 0});
    m_serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", "", "", 0});

    fmq::test::TestArrayDouble message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<double>(message.value().begin(), message.value().end()), VALUE);
}


TEST_F(TestSerializerProto, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayString", "", "", 0});
    m_serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayString", "", "", 0});

    fmq::test::TestArrayString message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::string>(message.value().begin(), message.value().end()), VALUE);
}


TEST_F(TestSerializerProto, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", "", "", 0});
    m_serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0, 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", "", "", 0});

    fmq::test::TestArrayBytes message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);

    std::vector<Bytes> result;
    std::for_each(message.value().begin(), message.value().end(), [&result] (const std::string& entry) {
        result.emplace_back(entry.data(), entry.data() + entry.size());
    });
    EXPECT_EQ(result, VALUE);
}

TEST_F(TestSerializerProto, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;
    static const std::uint32_t LAST_VALUE = 5;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", "", "desc", 0, 0});
    m_serializer->enterArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, 0}, VALUE1_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 2}, VALUE1_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, 0}, VALUE2_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 2}, VALUE2_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});

    m_serializer->exitArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, 0});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 1}, LAST_VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", "", "desc", 0, 0});

    fmq::test::TestArrayStruct message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value().size(), 3);
    EXPECT_EQ(message.value(0).struct_int32().value(), VALUE1_INT32);
    EXPECT_EQ(message.value(0).struct_string().value(), VALUE1_STRING);
    EXPECT_EQ(message.value(0).last_value(), VALUE1_UINT32);
    EXPECT_EQ(message.value(1).struct_int32().value(), VALUE2_INT32);
    EXPECT_EQ(message.value(1).struct_string().value(), VALUE2_STRING);
    EXPECT_EQ(message.value(1).last_value(), VALUE2_UINT32);
    EXPECT_EQ(message.value(2).struct_int32().value(), 0);
    EXPECT_EQ(message.value(2).struct_string().value(), "");
    EXPECT_EQ(message.value(2).last_value(), 0);
    EXPECT_EQ(message.last_value(), LAST_VALUE);
}

TEST_F(TestSerializerProto, testArrayEnum)
{
    static const fmq::test::Foo VALUE1 = fmq::test::Foo::FOO_HELLO;
    static const fmq::test::Foo VALUE2 = fmq::test::Foo::FOO_WORLD;
    static const fmq::test::Foo VALUE3 = fmq::test::Foo::FOO_WORLD2;
    static const fmq::test::Foo VALUE4 = (fmq::test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", "", "", 0});
    m_serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", "", "", 0});

    fmq::test::TestArrayEnum message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int32_t>(message.value().begin(), message.value().end()), VALUE);
}

