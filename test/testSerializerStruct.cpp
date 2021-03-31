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


#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/metadata/MetaData.h"
#include "test.fmq.h"


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

    test::TestBool root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBool"));
    serializer->enterBool({MetaTypeId::TYPE_BOOL, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestBool cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testInt32)
{
    static const std::int32_t VALUE = -2;

    test::TestInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt32"));
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    test::TestUInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt32"));
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestUInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testInt64)
{
    static const std::int64_t VALUE = -2;

    test::TestInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt64"));
    serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    test::TestUInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt64"));
    serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestUInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}



TEST_F(TestSerializerStruct, testFloat)
{
    static const float VALUE = -2;

    test::TestFloat root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestFloat"));
    serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestFloat cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testDouble)
{
    static const double VALUE = -2.1;

    test::TestDouble root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestDouble"));
    serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestDouble cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testString)
{
    static const std::string VALUE = "Hello World";

    test::TestString root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestString"));
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestString cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    test::TestBytes root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBytes"));
    serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestBytes cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    test::TestStruct root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, 0}, VALUE_INT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, 0}, VALUE_STRING.data(), VALUE_STRING.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 2}, VALUE_UINT32);

    serializer->finished();

    test::TestStruct cmp;
    cmp.struct_int32.value = VALUE_INT32;
    cmp.struct_string.value = VALUE_STRING;
    cmp.last_value = VALUE_UINT32;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnum)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnumStdString)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, 0}, std::string(VALUE.toString()));
    serializer->finished();

    test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testEnumString)
{
    static const test::Foo VALUE = test::Foo::FOO_HELLO;

    test::TestEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0, 0}, VALUE.toString().c_str(), VALUE.toString().size());
    serializer->finished();

    test::TestEnum cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}




TEST_F(TestSerializerStruct, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayBool root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBool"));
    serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestArrayBool cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}




TEST_F(TestSerializerStruct, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt32"));
    serializer->enterArrayInt32({MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayUInt32 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt32"));
    serializer->enterArrayUInt32({MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayUInt32 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt64"));
    serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayUInt64 root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt64"));
    serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayUInt64 cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayFloat)
{
    static const float VALUE1 = -1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayFloat root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayFloat"));
    serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayFloat cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayDouble root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayDouble"));
    serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayDouble cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayString root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayString"));
    serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestArrayString cmp;
    cmp.value = VALUE;
    ASSERT_EQ(root, cmp);
}

TEST_F(TestSerializerStruct, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    test::TestArrayBytes root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBytes"));
    serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0, 0}, VALUE);
    serializer->finished();

    test::TestArrayBytes cmp;
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
    static const std::uint32_t LAST_VALUE = 5;

    test::TestArrayStruct root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayStruct"));
    serializer->enterArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, 0});

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, 0}, VALUE1_INT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, 0}, VALUE1_STRING.data(), VALUE1_STRING.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 2}, VALUE1_UINT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, 0}, VALUE2_INT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, 0}, VALUE2_STRING.data(), VALUE2_STRING.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 2}, VALUE2_UINT32);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "desc", 0, 0});

    serializer->exitArrayStruct({MetaTypeId::TYPE_ARRAY_STRUCT, "test.TestStruct", "value", "desc", 0, 0});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 1}, LAST_VALUE);

    serializer->finished();

    test::TestArrayStruct cmp = {{
                                          {{VALUE1_INT32}, {VALUE1_STRING}, VALUE1_UINT32},
                                          {{VALUE2_INT32}, {VALUE2_STRING}, VALUE2_UINT32},
                                          {}
    }, LAST_VALUE};
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayEnum)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo::Enum)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    test::TestArrayEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, 0}, VALUE.data(), VALUE.size());
    serializer->finished();

    test::TestArrayEnum cmp = {{VALUE1, VALUE2, VALUE3, VALUE2}};
    ASSERT_EQ(root, cmp);
}


TEST_F(TestSerializerStruct, testArrayEnumString)
{
    static const test::Foo VALUE1 = test::Foo::FOO_HELLO;
    static const test::Foo VALUE2 = test::Foo::FOO_WORLD;
    static const test::Foo VALUE3 = test::Foo::FOO_WORLD2;
    static const test::Foo VALUE4 = (test::Foo::Enum)123;
    static const std::vector<std::string> VALUEString = {VALUE1.toString(), VALUE2.toString(), VALUE3.toString(), VALUE4.toString()};

    test::TestArrayEnum root;
    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerStruct>(root);

    serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0, 0}, VALUEString);
    serializer->finished();

    test::TestArrayEnum cmp = {{VALUE1, VALUE2, VALUE3, VALUE2}};
    ASSERT_EQ(root, cmp);
}


