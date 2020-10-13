#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "serializeproto/SerializerProto.h"
#include "metadata/MetaData.h"
#include "MockIZeroCopyBuffer.h"
#include "test.pb.h"

//#include <thread>
//#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;


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
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE)).Times(1).WillOnce(Return((char*)m_data.data()));
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageBool", ""});
    m_serializer->enterBool({MetaTypeId::TYPE_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageBool", ""});

    test::TestMessageBool message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testInt32)
{
    static const std::int32_t VALUE = -2;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", ""});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", ""});

    test::TestMessageInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageUInt32", ""});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageUInt32", ""});

    test::TestMessageUInt32 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testInt64)
{
    static const std::int64_t VALUE = -2;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt64", ""});
    m_serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt64", ""});

    test::TestMessageInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageUInt64", ""});
    m_serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageUInt64", ""});

    test::TestMessageUInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}



TEST_F(TestSerializerProto, testFloat)
{
    static const float VALUE = -2.1;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageFloat", ""});
    m_serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageFloat", ""});

    test::TestMessageFloat message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testDouble)
{
    static const double VALUE = -2.1;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageDouble", ""});
    m_serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageDouble", ""});

    test::TestMessageDouble message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}


TEST_F(TestSerializerProto, testString)
{
    static const std::string VALUE = "Hello World";

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", ""});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", ""});

    test::TestMessageString message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value(), VALUE);
}

TEST_F(TestSerializerProto, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageBytes", ""});
    m_serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageBytes", ""});

    test::TestMessageBytes message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});

    m_serializer->finished();

    test::TestMessageStruct message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE_INT32);
    EXPECT_EQ(message.struct_string().value(), VALUE_STRING);
    EXPECT_EQ(message.last_value(), VALUE_UINT32);
}



TEST_F(TestSerializerProto, testEnum)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageEnum", ""});
    m_serializer->enterEnum({MetaTypeId::TYPE_ENUM, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageEnum", ""});

    test::TestMessageEnum message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayBool", ""});
    m_serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayBool", ""});

    test::TestMessageArrayBool message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayInt32", ""});
    m_serializer->enterArrayInt32({MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayInt32", ""});

    test::TestMessageArrayInt32 message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayUInt32", ""});
    m_serializer->enterArrayUInt32({MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayUInt32", ""});

    test::TestMessageArrayUInt32 message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayInt64", ""});
    m_serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayInt64", ""});

    test::TestMessageArrayInt64 message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayUInt64", ""});
    m_serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayUInt64", ""});

    test::TestMessageArrayUInt64 message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::uint64_t>(message.value().begin(), message.value().end()), VALUE);
}

TEST_F(TestSerializerProto, testArrayFloat)
{
    static const float VALUE1 = -1.1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1.1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayFloat", ""});
    m_serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayFloat", ""});

    test::TestMessageArrayFloat message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayDouble", ""});
    m_serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayDouble", ""});

    test::TestMessageArrayDouble message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayString", ""});
    m_serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayString", ""});

    test::TestMessageArrayString message;
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

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayBytes", ""});
    m_serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayBytes", ""});

    test::TestMessageArrayBytes message;
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


    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayStruct", "", "desc"});
    m_serializer->enterArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestMessageStruct", "value", "desc"});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE1_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE1_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE2_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE2_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageStruct", "", "desc", 0});

    m_serializer->exitArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestMessageStruct", "value", "desc"});
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayStruct", "", "desc"});

    test::TestMessageArrayStruct message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.value().size(), 3);
    EXPECT_EQ(message.value()[0].struct_int32().value(), VALUE1_INT32);
    EXPECT_EQ(message.value()[0].struct_string().value(), VALUE1_STRING);
    EXPECT_EQ(message.value()[0].last_value(), VALUE1_UINT32);
    EXPECT_EQ(message.value()[1].struct_int32().value(), VALUE2_INT32);
    EXPECT_EQ(message.value()[1].struct_string().value(), VALUE2_STRING);
    EXPECT_EQ(message.value()[1].last_value(), VALUE2_UINT32);
    EXPECT_EQ(message.value()[2].struct_int32().value(), 0);
    EXPECT_EQ(message.value()[2].struct_string().value(), "");
    EXPECT_EQ(message.value()[2].last_value(), 0);
}

TEST_F(TestSerializerProto, testArrayEnum)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayEnum", ""});
    m_serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageArrayEnum", ""});

    test::TestMessageArrayEnum message;
    bool res = message.ParseFromString(m_data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(std::vector<std::int32_t>(message.value().begin(), message.value().end()), VALUE);
}

