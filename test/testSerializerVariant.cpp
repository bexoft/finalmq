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


#include "finalmq/serializevariant/SerializerVariant.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/metadata/MetaData.h"


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




class TestSerializerVariant : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        m_serializer = std::make_unique<SerializerVariant>(m_root);
        m_serializerEnumAsInt = std::make_unique<SerializerVariant>(m_root, false);
    }

    virtual void TearDown()
    {
    }

    Variant                         m_root;
    std::unique_ptr<IParserVisitor> m_serializer;
    std::unique_ptr<IParserVisitor> m_serializerEnumAsInt;
};




TEST_F(TestSerializerVariant, testBool)
{
    static const bool VALUE = true;

    m_serializer->enterBool({MetaTypeId::TYPE_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testInt32)
{
    static const std::int32_t VALUE = -2;

    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testInt64)
{
    static const std::int64_t VALUE = -2;

    m_serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    m_serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}



TEST_F(TestSerializerVariant, testFloat)
{
    static const float VALUE = -2;

    m_serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testDouble)
{
    static const double VALUE = -2.1;

    m_serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testString)
{
    static const std::string VALUE = "Hello World";

    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testBytes)
{
    static const Bytes VALUE = {'H','e','l',0,13,'l','o'};

    m_serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;

    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0, 0}, VALUE_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageInt32", "struct_int32", "desc", 0, 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 0, 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0, 0}, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestMessageString", "struct_string", "desc", 0, 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 0, 2}, VALUE_UINT32);

    m_serializer->finished();

    Variant VALUE = VariantStruct({ {"struct_int32", VariantStruct({ {"value", VALUE_INT32} })}, {"struct_string", VariantStruct({ {"value", VALUE_STRING} })},  {"last_value", VALUE_UINT32} });
    ASSERT_EQ(m_root == VALUE, true);
}


TEST_F(TestSerializerVariant, testEnum)
{
    static const std::int32_t VALUE = -2;

    m_serializer->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", std::string("FOO_HELLO")}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testEnumAsInt)
{
    static const std::int32_t VALUE = -2;

    m_serializerEnumAsInt->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, VALUE);
    m_serializerEnumAsInt->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}




TEST_F(TestSerializerVariant, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}




TEST_F(TestSerializerVariant, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayInt32({MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayUInt32({MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayFloat)
{
    static const float VALUE1 = -1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayBytes)
{
    static const Bytes VALUE1 = {'H', 'e', 'l', 'l', 'o'};
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = {'W', 'o', 'r', 'l', 'd'};
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}

TEST_F(TestSerializerVariant, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;
    static const std::uint32_t LAST_VALUE = 5;

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
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 1}, LAST_VALUE);

    m_serializer->finished();

    Variant VALUE = VariantStruct({ {"value", VariantList({
        VariantStruct({ {"struct_int32", VariantStruct({ {"value", VALUE1_INT32} })}, {"struct_string", VariantStruct({ {"value", VALUE1_STRING} })},  {"last_value", VALUE1_UINT32} }),
        VariantStruct({ {"struct_int32", VariantStruct({ {"value", VALUE2_INT32} })}, {"struct_string", VariantStruct({ {"value", VALUE2_STRING} })},  {"last_value", VALUE2_UINT32} }),
        VariantStruct()
    })}, {"last_value", LAST_VALUE} });
    ASSERT_EQ(m_root == VALUE, true);
}

TEST_F(TestSerializerVariant, testArrayEnum)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::int32_t VALUE4 = 123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    Variant cmp = VariantStruct({{"value", std::vector<std::string>({"FOO_HELLO","FOO_WORLD","FOO_WORLD2","FOO_WORLD"})}});
    ASSERT_EQ(m_root == cmp, true);
}


TEST_F(TestSerializerVariant, testArrayEnumAsInt)
{
    static const std::int32_t VALUE1 = -2;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::int32_t VALUE4 = 123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializerEnumAsInt->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializerEnumAsInt->finished();

    Variant cmp = VariantStruct({{"value", VALUE}});
    ASSERT_EQ(m_root == cmp, true);
}


