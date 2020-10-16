#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "serializestruct/SerializerStruct.h"
#include "metadata/MetaField.h"
#include "testStructs.h"


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




class TestSerializerStruct : public testing::Test
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




TEST_F(TestSerializerStruct, testBool)
{
    static const bool VALUE = true;

    fmq::test::TestBool root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestBool", ""});
    serializer->enterBool({MetaTypeId::TYPE_BOOL, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestBool", ""});
    serializer->finished();

    fmq::test::TestBool cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testInt32)
{
    static const std::int32_t VALUE = -2;

    fmq::test::TestInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", ""});
    serializer->finished();

    fmq::test::TestInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    fmq::test::TestUInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt32", ""});
    serializer->finished();

    fmq::test::TestUInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testInt64)
{
    static const std::int64_t VALUE = -2;

    fmq::test::TestInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""});
    serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt64", ""});
    serializer->finished();

    fmq::test::TestInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    fmq::test::TestUInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""});
    serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestUInt64", ""});
    serializer->finished();

    fmq::test::TestUInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}



TEST_F(TestSerializerStruct, testFloat)
{
    static const float VALUE = -2;

    fmq::test::TestFloat root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""});
    serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestFloat", ""});
    serializer->finished();

    fmq::test::TestFloat cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testDouble)
{
    static const double VALUE = -2.1;

    fmq::test::TestDouble root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""});
    serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestDouble", ""});
    serializer->finished();

    fmq::test::TestDouble cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testString)
{
    static const std::string VALUE = "Hello World";

    fmq::test::TestString root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", ""});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", ""});
    serializer->finished();

    fmq::test::TestString cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    fmq::test::TestBytes root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""});
    serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestBytes", ""});
    serializer->finished();

    fmq::test::TestBytes cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    fmq::test::TestStruct root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE_INT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE_STRING.data(), VALUE_STRING.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE_UINT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    serializer->finished();

    fmq::test::TestStruct cmp;
    cmp.struct_int32.value = VALUE_INT32;
    cmp.struct_string.value = VALUE_STRING;
    cmp.last_value = VALUE_UINT32;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnum)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    fmq::test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    serializer->finished();

    fmq::test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnumStdString)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    fmq::test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, std::string(VALUE.toString()));
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    serializer->finished();

    fmq::test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnumString)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    fmq::test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, VALUE.toString().c_str(), VALUE.toString().size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestEnum", ""});
    serializer->finished();

    fmq::test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}




TEST_F(TestSerializerStruct, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayBool root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", ""});
    serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBool", ""});
    serializer->finished();

    fmq::test::TestArrayBool cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}




TEST_F(TestSerializerStruct, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", ""});
    serializer->enterArrayInt32({MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt32", ""});
    serializer->finished();

    fmq::test::TestArrayInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayUInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", ""});
    serializer->enterArrayUInt32({MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt32", ""});
    serializer->finished();

    fmq::test::TestArrayUInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", ""});
    serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayInt64", ""});
    serializer->finished();

    fmq::test::TestArrayInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayUInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", ""});
    serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayUInt64", ""});
    serializer->finished();

    fmq::test::TestArrayUInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayFloat)
{
    static const float VALUE1 = -1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayFloat root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", ""});
    serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayFloat", ""});
    serializer->finished();

    fmq::test::TestArrayFloat cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayDouble root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", ""});
    serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0}, VALUE.data(), VALUE.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayDouble", ""});
    serializer->finished();

    fmq::test::TestArrayDouble cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayString root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayString", ""});
    serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayString", ""});
    serializer->finished();

    fmq::test::TestArrayString cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    fmq::test::TestArrayBytes root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", ""});
    serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0}, VALUE);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayBytes", ""});
    serializer->finished();

    fmq::test::TestArrayBytes cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}




TEST_F(TestSerializerStruct, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;

    fmq::test::TestArrayStruct root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", "", "desc"});
    serializer->enterArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0});

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE1_INT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE1_UINT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE2_INT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE2_UINT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0});

    serializer->exitArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0});
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayStruct", "", "desc"});

    serializer->finished();

    fmq::test::TestArrayStruct cmp = {{
                                          {{VALUE1_INT32}, {VALUE1_STRING}, VALUE1_UINT32},
                                          {{VALUE2_INT32}, {VALUE2_STRING}, VALUE2_UINT32},
                                          {}
    }};
    ASSERT_EQ(root, cmp);
}


//TEST_F(TestSerializerVariant, testArrayEnum)
//{
//    static const std::int32_t VALUE1 = -2;
//    static const std::int32_t VALUE2 = 0;
//    static const std::int32_t VALUE3 = 1;
//    static const std::int32_t VALUE4 = 123;
//    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

//    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
//    m_serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
//    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
//    m_serializer->finished();

//    Variant cmp = VariantStruct({{"value", std::vector<std::string>({"FOO_HELLO","FOO_WORLD","FOO_WORLD2","FOO_WORLD"})}});
//    ASSERT_EQ(m_root == cmp, true);
//}


//TEST_F(TestSerializerVariant, testArrayEnumAsInt)
//{
//    static const std::int32_t VALUE1 = -2;
//    static const std::int32_t VALUE2 = 0;
//    static const std::int32_t VALUE3 = 1;
//    static const std::int32_t VALUE4 = 123;
//    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

//    m_serializerEnumAsInt->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
//    m_serializerEnumAsInt->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
//    m_serializerEnumAsInt->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestArrayEnum", ""});
//    m_serializerEnumAsInt->finished();

//    Variant cmp = VariantStruct({{"value", VALUE}});
//    ASSERT_EQ(m_root == cmp, true);
//}


