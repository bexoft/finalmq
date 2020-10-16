#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "serializejson/SerializerJson.h"
#include "metadata/MetaData.h"
#include "MockIZeroCopyBuffer.h"
#include "test.pb.h"


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



class TestSerializerJson : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        static const int MAX_BLOCK_SIZE = 500;

        m_data.resize(MAX_BLOCK_SIZE);
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE)).Times(1).WillOnce(Return((char*)m_data.data()));
        EXPECT_CALL(m_mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&m_data, &String::resize));

        m_serializer = std::make_unique<SerializerJson>(m_mockBuffer, MAX_BLOCK_SIZE);
        m_serializerEnumAsInt = std::make_unique<SerializerJson>(m_mockBuffer, MAX_BLOCK_SIZE, false);
    }

    virtual void TearDown()
    {
    }

    String                          m_data;
    MockIZeroCopyBuffer             m_mockBuffer;
    std::unique_ptr<IParserVisitor> m_serializer;
    std::unique_ptr<IParserVisitor> m_serializerEnumAsInt;
};




TEST_F(TestSerializerJson, testBool)
{
    static const bool VALUE = true;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestBool", ""});
    m_serializer->enterBool({MetaTypeId::TYPE_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestBool", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":true}");
}


TEST_F(TestSerializerJson, testInt32)
{
    static const std::int32_t VALUE = -2;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2}");
}

TEST_F(TestSerializerJson, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":4294967294}");
}

TEST_F(TestSerializerJson, testInt64)
{
    static const std::int64_t VALUE = -2;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""});
    m_serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2}");
}

TEST_F(TestSerializerJson, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""});
    m_serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"18446744073709551614\"}");
}



TEST_F(TestSerializerJson, testFloat)
{
    static const float VALUE = -2;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""});
    m_serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2.0}");
}


TEST_F(TestSerializerJson, testDouble)
{
    static const double VALUE = -2.1;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""});
    m_serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2.1}");
}


TEST_F(TestSerializerJson, testString)
{
    static const std::string VALUE = "Hello World";

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", ""});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"Hello World\"}");
}

TEST_F(TestSerializerJson, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""});
    m_serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"Hel\\u0000\\rlo\"}");
}



TEST_F(TestSerializerJson, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;


    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"struct_int32\":{\"value\":-2},\"struct_string\":{\"value\":\"Hello World\"},\"last_value\":123}");
}




TEST_F(TestSerializerJson, testEnum)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    m_serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"FOO_HELLO\"}");
}


TEST_F(TestSerializerJson, testEnumAsInt)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    m_serializerEnumAsInt->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    m_serializerEnumAsInt->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, VALUE);
    m_serializerEnumAsInt->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    m_serializerEnumAsInt->finished();

    ASSERT_EQ(m_data, "{\"value\":-2}");
}




TEST_F(TestSerializerJson, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", ""});
    m_serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[true,false,true]}");
}




TEST_F(TestSerializerJson, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", ""});
    m_serializer->enterArrayInt32({MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1,0,1]}");
}

TEST_F(TestSerializerJson, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", ""});
    m_serializer->enterArrayUInt32({MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[4294967294,0,1]}");
}

TEST_F(TestSerializerJson, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", ""});
    m_serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1,0,1]}");
}

TEST_F(TestSerializerJson, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", ""});
    m_serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"18446744073709551614\",\"0\",\"1\"]}");
}

TEST_F(TestSerializerJson, testArrayFloat)
{
    static const float VALUE1 = -1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", ""});
    m_serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1.0,0.0,1.0]}");
}

TEST_F(TestSerializerJson, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", ""});
    m_serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1.1,0.0,1.1]}");
}


TEST_F(TestSerializerJson, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayString", ""});
    m_serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayString", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"Hello\",\"\",\"World\"]}");
}

TEST_F(TestSerializerJson, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", ""});
    m_serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0}, VALUE);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"Hello\",\"\",\"World\"]}");
}

TEST_F(TestSerializerJson, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;


    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", "", "desc"});
    m_serializer->enterArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc"});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE1_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE1_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE2_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE2_UINT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    m_serializer->exitArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc"});
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", "", "desc"});

    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[{\"struct_int32\":{\"value\":-2},\"struct_string\":{\"value\":\"Hello World\"},\"last_value\":123},{\"struct_int32\":{\"value\":345},\"struct_string\":{\"value\":\"foo\"},\"last_value\":12345678},{}]}");
}

TEST_F(TestSerializerJson, testArrayEnum)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
    m_serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"FOO_HELLO\",\"FOO_WORLD\",\"FOO_WORLD2\",\"FOO_WORLD\"]}");
}


TEST_F(TestSerializerJson, testArrayEnumAsInt)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializerEnumAsInt->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
    m_serializerEnumAsInt->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializerEnumAsInt->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
    m_serializerEnumAsInt->finished();

    ASSERT_EQ(m_data, "{\"value\":[-2,0,1,123]}");
}


