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


#include "finalmq/serializejson/SerializerJson.h"
#include "finalmq/metadata/MetaData.h"
#include "MockIZeroCopyBuffer.h"
#include "test.pb.h"
#include "finalmq/metadataserialize/variant.fmq.h"


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



class TestSerializerJson : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        static const int MAX_BLOCK_SIZE = 500;

        m_data.resize(MAX_BLOCK_SIZE);
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE, _)).Times(1).WillOnce(Return((char*)m_data.data()));
        EXPECT_CALL(m_mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&m_data, &String::resize));
        m_serializer = std::make_unique<SerializerJson>(m_mockBuffer, MAX_BLOCK_SIZE);
        m_serializerDefault = std::make_unique<SerializerJson>(m_mockBuffer, MAX_BLOCK_SIZE, true, false);
        m_serializerEnumAsInt = std::make_unique<SerializerJson>(m_mockBuffer, MAX_BLOCK_SIZE, false);

        const MetaStruct* structTestVariant = MetaDataGlobal::instance().getStruct("test.TestVariant");
        ASSERT_NE(structTestVariant, nullptr);
        m_fieldValue = structTestVariant->getFieldByName("value");
        ASSERT_NE(m_fieldValue, nullptr);
        ASSERT_EQ(m_fieldValue->typeName, "finalmq.variant.VarValue");
        m_fieldValue2 = structTestVariant->getFieldByName("value2");
        ASSERT_NE(m_fieldValue2, nullptr);
        ASSERT_EQ(m_fieldValue2->typeName, "finalmq.variant.VarValue");
        m_fieldValueInt32 = structTestVariant->getFieldByName("valueInt32");
        ASSERT_NE(m_fieldValueInt32, nullptr);

        const MetaStruct* structVarVariant = MetaDataGlobal::instance().getStruct("finalmq.variant.VarValue");
        ASSERT_NE(structVarVariant, nullptr);

        m_fieldName = structVarVariant->getFieldByName("name");
        m_fieldType = structVarVariant->getFieldByName("type");
        m_fieldInt32 = structVarVariant->getFieldByName("valint32");
        m_fieldString = structVarVariant->getFieldByName("valstring");
        m_fieldList = structVarVariant->getFieldByName("vallist");
        m_fieldListWithoutArray = MetaDataGlobal::instance().getArrayField(*m_fieldList);

        ASSERT_NE(m_fieldName, nullptr);
        ASSERT_NE(m_fieldType, nullptr);
        ASSERT_NE(m_fieldInt32, nullptr);
        ASSERT_NE(m_fieldString, nullptr);
        ASSERT_NE(m_fieldList, nullptr);
        ASSERT_NE(m_fieldListWithoutArray, nullptr);
    }

    virtual void TearDown()
    {
    }

    String                          m_data;
    MockIZeroCopyBuffer             m_mockBuffer;
    std::unique_ptr<IParserVisitor> m_serializer;
    std::unique_ptr<IParserVisitor> m_serializerDefault;
    std::unique_ptr<IParserVisitor> m_serializerEnumAsInt;

    const MetaField* m_fieldValue = nullptr;
    const MetaField* m_fieldValue2 = nullptr;
    const MetaField* m_fieldValueInt32 = nullptr;
    const MetaField* m_fieldName = nullptr;
    const MetaField* m_fieldType = nullptr;
    const MetaField* m_fieldInt32 = nullptr;
    const MetaField* m_fieldString = nullptr;
    const MetaField* m_fieldList = nullptr;
    const MetaField* m_fieldListWithoutArray = nullptr;
};




TEST_F(TestSerializerJson, testBool)
{
    static const bool VALUE = true;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBool"));
    m_serializer->enterBool({MetaTypeId::TYPE_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":true}");
}


TEST_F(TestSerializerJson, testInt32)
{
    static const std::int32_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt32"));
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2}");
}

TEST_F(TestSerializerJson, testUInt32)
{
    static const std::uint32_t VALUE = 0xFFFFFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt32"));
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":4294967294}");
}

TEST_F(TestSerializerJson, testInt64)
{
    static const std::int64_t VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestInt64"));
    m_serializer->enterInt64({MetaTypeId::TYPE_INT64, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2}");
}

TEST_F(TestSerializerJson, testUInt64)
{
    static const std::uint64_t VALUE = 0xFFFFFFFFFFFFFFFE;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestUInt64"));
    m_serializer->enterUInt64({MetaTypeId::TYPE_UINT64, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"18446744073709551614\"}");
}



TEST_F(TestSerializerJson, testFloat)
{
    static const float VALUE = -2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestFloat"));
    m_serializer->enterFloat({MetaTypeId::TYPE_FLOAT, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2.0}");
}


TEST_F(TestSerializerJson, testDouble)
{
    static const double VALUE = -2.1;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestDouble"));
    m_serializer->enterDouble({MetaTypeId::TYPE_DOUBLE, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":-2.1}");
}


TEST_F(TestSerializerJson, testString)
{
    static const std::string VALUE = "Hello World";

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestString"));
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"Hello World\"}");
}

TEST_F(TestSerializerJson, testBytes)
{
    static const Bytes VALUE = { 0x23, (BytesElement)0xaa, 0x00, 0x6a, 0x40, 0x00 };

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestBytes"));
    m_serializer->enterBytes({MetaTypeId::TYPE_BYTES, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"I6oAakAA\"}");
}



TEST_F(TestSerializerJson, testStruct)
{
    static const std::int32_t VALUE_INT32 = -2;
    static const std::string VALUE_STRING = "Hello World";
    static const std::uint32_t VALUE_UINT32 = 123;


    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestStruct"));
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "desc", 0}, VALUE_INT32);
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "desc", 0});
    m_serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "desc", 0}, VALUE_STRING.data(), VALUE_STRING.size());
    m_serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "desc", 1});
    m_serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "desc", 2}, VALUE_UINT32);

    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"struct_int32\":{\"value\":-2},\"struct_string\":{\"value\":\"Hello World\"},\"last_value\":123}");
}




TEST_F(TestSerializerJson, testEnum)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializer->enterEnum({ MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0 }, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"FOO_HELLO\"}");
}


TEST_F(TestSerializerJson, testEnumAlias)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_WORLD2;

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializer->enterEnum({ MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0 }, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":\"world2\"}");
}


TEST_F(TestSerializerJson, testEnumAsInt)
{
    static const fmq::test::Foo VALUE = fmq::test::Foo::FOO_HELLO;

    m_serializerEnumAsInt->startStruct(*MetaDataGlobal::instance().getStruct("test.TestEnum"));
    m_serializerEnumAsInt->enterEnum({MetaTypeId::TYPE_ENUM, "test.Foo", "value", "", 0}, VALUE);
    m_serializerEnumAsInt->finished();

    ASSERT_EQ(m_data, "{\"value\":-2}");
}


TEST_F(TestSerializerJson, testVariantInt32)
{
    static const std::int32_t VALUE = -2;

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct("test.TestVariant");
    ASSERT_NE(stru, nullptr);
    const MetaStruct* struValue = MetaDataGlobal::instance().getStruct(stru->getFieldByName("value")->typeName);
    m_serializer->startStruct(*stru);
    m_serializer->enterStruct(*stru->getFieldByName("value"));
    m_serializer->enterEnum(*struValue->getFieldByName("type"), "int32");
    m_serializer->enterInt32(*struValue->getFieldByName("valint32"), VALUE);
    m_serializer->exitStruct(*stru->getFieldByName("value"));
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":{\"type\":\"int32\",\"valint32\":-2}}");
}

TEST_F(TestSerializerJson, testVariantInt32Default)
{
    static const std::int32_t VALUE = -2;

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct("test.TestVariant");
    ASSERT_NE(stru, nullptr);
    const MetaStruct* struValue = MetaDataGlobal::instance().getStruct(stru->getFieldByName("value")->typeName);
    m_serializerDefault->startStruct(*stru);
    m_serializerDefault->enterStruct(*stru->getFieldByName("value"));
    m_serializerDefault->enterEnum(*struValue->getFieldByName("type"), "int32");
    m_serializerDefault->enterInt32(*struValue->getFieldByName("valint32"), VALUE);
    m_serializerDefault->exitStruct(*stru->getFieldByName("value"));
    m_serializerDefault->finished();

    ASSERT_EQ(m_data, "{\"value\":{\"type\":\"int32\",\"valint32\":-2},\"valueInt32\":0,\"value2\":{}}");
}


TEST_F(TestSerializerJson, testVariantEmptyDefault)
{
    static const std::int32_t VALUE = -2;

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct("test.TestVariant");
    ASSERT_NE(stru, nullptr);
    const MetaStruct* struValue = MetaDataGlobal::instance().getStruct(stru->getFieldByName("value")->typeName);
    m_serializerDefault->startStruct(*stru);
    m_serializerDefault->finished();

    ASSERT_EQ(m_data, "{\"value\":{},\"valueInt32\":0,\"value2\":{}}");
}



TEST_F(TestSerializerJson, testVariantStructDefault)
{
    // VariantStruct{ {"value", VariantStruct{
    m_serializerDefault->startStruct(*MetaDataGlobal::instance().getStruct("test.TestVariant"));
    m_serializerDefault->enterStruct(*m_fieldValue);
    m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_STRUCT);
    m_serializerDefault->enterArrayStruct(*m_fieldList);
        // {"key1", VariantList{
        m_serializerDefault->enterStruct(*m_fieldListWithoutArray);
        m_serializerDefault->enterString(*m_fieldName, "key1", 4);
        m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_LIST);
        m_serializerDefault->enterArrayStruct(*m_fieldList);
            // 2
            m_serializerDefault->enterStruct(*m_fieldListWithoutArray);
            m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_INT32);
            m_serializerDefault->enterInt32(*m_fieldInt32, 2);
            m_serializerDefault->exitStruct(*m_fieldListWithoutArray);
            // , std::string("Hello")
            m_serializerDefault->enterStruct(*m_fieldListWithoutArray);
            m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_STRING);
            m_serializerDefault->enterString(*m_fieldString, "Hello", 5);
            m_serializerDefault->exitStruct(*m_fieldListWithoutArray);
        // }
        m_serializerDefault->exitArrayStruct(*m_fieldList);
        m_serializerDefault->exitStruct(*m_fieldListWithoutArray);

        // {"key2", VariantStruct{
        m_serializerDefault->enterStruct(*m_fieldListWithoutArray);
        m_serializerDefault->enterString(*m_fieldName, "key2", 4);
        m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_STRUCT);
        m_serializerDefault->enterArrayStruct(*m_fieldList);
            // {"a", 3},
            m_serializerDefault->enterStruct(*m_fieldListWithoutArray);
            m_serializerDefault->enterString(*m_fieldName, "a", 1);
            m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_INT32);
            m_serializerDefault->enterInt32(*m_fieldInt32, 3);
            m_serializerDefault->exitStruct(*m_fieldListWithoutArray);
            // {"b", std::string("Hi")}
            m_serializerDefault->enterStruct(*m_fieldListWithoutArray);
            m_serializerDefault->enterString(*m_fieldName, "b", 1);
            m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_STRING);
            m_serializerDefault->enterString(*m_fieldString, "Hi", 2);
            m_serializerDefault->exitStruct(*m_fieldListWithoutArray);
        // }
        m_serializerDefault->exitArrayStruct(*m_fieldList);
        m_serializerDefault->exitStruct(*m_fieldListWithoutArray);

        // {
        m_serializerDefault->enterStruct(*m_fieldListWithoutArray);
        m_serializerDefault->enterString(*m_fieldName, "key3", 4);
        m_serializerDefault->enterEnum(*m_fieldType, variant::VarTypeId::T_NONE);
        m_serializerDefault->exitStruct(*m_fieldListWithoutArray);
    // }}
    m_serializerDefault->exitArrayStruct(*m_fieldList);
    m_serializerDefault->exitStruct(*m_fieldValue);
    m_serializerDefault->finished();

    std::string cmp = "{\"value\":{\"type\":\"struct\",\"vallist\":[{\"name\":\"key1\",\"type\":\"list\",\"vallist\":[{\"type\":\"int32\",\"valint32\":2},{\"type\":\"string\",\"valstring\":\"Hello\"}]},{\"name\":\"key2\",\"type\":\"struct\",\"vallist\":[{\"name\":\"a\",\"type\":\"int32\",\"valint32\":3},{\"name\":\"b\",\"type\":\"string\",\"valstring\":\"Hi\"}]},{\"name\":\"key3\",\"type\":\"none\"}]},\"valueInt32\":0,\"value2\":{}}";
    ASSERT_EQ(m_data == cmp, true);
}



TEST_F(TestSerializerJson, testArrayBool)
{
    static const bool VALUE1 = true;
    static const bool VALUE2 = false;
    static const bool VALUE3 = true;
    static const std::vector<bool> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBool"));
    m_serializer->enterArrayBool({MetaTypeId::TYPE_ARRAY_BOOL, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[true,false,true]}");
}




TEST_F(TestSerializerJson, testArrayInt32)
{
    static const std::int32_t VALUE1 = -1;
    static const std::int32_t VALUE2 = 0;
    static const std::int32_t VALUE3 = 1;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt32"));
    m_serializer->enterArrayInt32({MetaTypeId::TYPE_ARRAY_INT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1,0,1]}");
}

TEST_F(TestSerializerJson, testArrayUInt32)
{
    static const std::uint32_t VALUE1 = 0xfffffffe;
    static const std::uint32_t VALUE2 = 0;
    static const std::uint32_t VALUE3 = 1;
    static const std::vector<std::uint32_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt32"));
    m_serializer->enterArrayUInt32({MetaTypeId::TYPE_ARRAY_UINT32, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[4294967294,0,1]}");
}

TEST_F(TestSerializerJson, testArrayInt64)
{
    static const std::int64_t VALUE1 = -1;
    static const std::int64_t VALUE2 = 0;
    static const std::int64_t VALUE3 = 1;
    static const std::vector<std::int64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayInt64"));
    m_serializer->enterArrayInt64({MetaTypeId::TYPE_ARRAY_INT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1,0,1]}");
}

TEST_F(TestSerializerJson, testArrayUInt64)
{
    static const std::uint64_t VALUE1 = 0xfffffffffffffffe;
    static const std::uint64_t VALUE2 = 0;
    static const std::uint64_t VALUE3 = 1;
    static const std::vector<std::uint64_t> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayUInt64"));
    m_serializer->enterArrayUInt64({MetaTypeId::TYPE_ARRAY_UINT64, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"18446744073709551614\",\"0\",\"1\"]}");
}

TEST_F(TestSerializerJson, testArrayFloat)
{
    static const float VALUE1 = -1;
    static const float VALUE2 = 0;
    static const float VALUE3 = 1;
    static const std::vector<float> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayFloat"));
    m_serializer->enterArrayFloat({MetaTypeId::TYPE_ARRAY_FLOAT, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1.0,0.0,1.0]}");
}

TEST_F(TestSerializerJson, testArrayDouble)
{
    static const double VALUE1 = -1.1;
    static const double VALUE2 = 0;
    static const double VALUE3 = 1.1;
    static const std::vector<double> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayDouble"));
    m_serializer->enterArrayDouble({MetaTypeId::TYPE_ARRAY_DOUBLE, "", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[-1.1,0.0,1.1]}");
}


TEST_F(TestSerializerJson, testArrayString)
{
    static const std::string VALUE1 = "Hello";
    static const std::string VALUE2 = "";
    static const std::string VALUE3 = "World";
    static const std::vector<std::string> VALUE = {VALUE1, VALUE2, VALUE3};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayString"));
    m_serializer->enterArrayString({MetaTypeId::TYPE_ARRAY_STRING, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"Hello\",\"\",\"World\"]}");
}

TEST_F(TestSerializerJson, testArrayBytes)
{
    static const Bytes VALUE1 = { 0x23, (BytesElement)0xaa, 0x00, 0x6a, 0x40, 0x00 };
    static const Bytes VALUE2 = {};
    static const Bytes VALUE3 = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, (BytesElement)0x88,(BytesElement)0x99, (BytesElement)0xaa, (BytesElement)0xbb, (BytesElement)0xcc, (BytesElement)0xdd, (BytesElement)0xee, (BytesElement)0xff };
    static const Bytes VALUE4 = { 0x00 };
    static const std::vector<Bytes> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayBytes"));
    m_serializer->enterArrayBytes({MetaTypeId::TYPE_ARRAY_BYTES, "", "value", "", 0}, VALUE);
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"I6oAakAA\",\"\",\"ABEiM0RVZneImaq7zN3u/w==\",\"AA==\"]}");
}

TEST_F(TestSerializerJson, testArrayStruct)
{
    static const std::int32_t VALUE1_INT32 = -2;
    static const std::string VALUE1_STRING = "Hello World";
    static const std::uint32_t VALUE1_UINT32 = 123;
    static const std::int32_t VALUE2_INT32 = 345;
    static const std::string VALUE2_STRING = "foo";
    static const std::uint32_t VALUE2_UINT32 = 12345678;


    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayStruct"));
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

    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[{\"struct_int32\":{\"value\":-2},\"struct_string\":{\"value\":\"Hello World\"},\"last_value\":123},{\"struct_int32\":{\"value\":345},\"struct_string\":{\"value\":\"foo\"},\"last_value\":12345678},{}]}");
}

TEST_F(TestSerializerJson, testArrayEnum)
{
    static const fmq::test::Foo VALUE1 = fmq::test::Foo::FOO_HELLO;
    static const fmq::test::Foo VALUE2 = fmq::test::Foo::FOO_WORLD;
    static const fmq::test::Foo VALUE3 = fmq::test::Foo::FOO_WORLD2;
    static const fmq::test::Foo VALUE4 = (fmq::test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializer->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    m_serializer->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializer->finished();

    ASSERT_EQ(m_data, "{\"value\":[\"FOO_HELLO\",\"FOO_WORLD\",\"world2\",\"FOO_WORLD\"]}");
}


TEST_F(TestSerializerJson, testArrayEnumAsInt)
{
    static const fmq::test::Foo VALUE1 = fmq::test::Foo::FOO_HELLO;
    static const fmq::test::Foo VALUE2 = fmq::test::Foo::FOO_WORLD;
    static const fmq::test::Foo VALUE3 = fmq::test::Foo::FOO_WORLD2;
    static const fmq::test::Foo VALUE4 = (fmq::test::Foo)123;
    static const std::vector<std::int32_t> VALUE = {VALUE1, VALUE2, VALUE3, VALUE4};

    m_serializerEnumAsInt->startStruct(*MetaDataGlobal::instance().getStruct("test.TestArrayEnum"));
    m_serializerEnumAsInt->enterArrayEnum({MetaTypeId::TYPE_ARRAY_ENUM, "test.Foo", "value", "", 0}, VALUE.data(), VALUE.size());
    m_serializerEnumAsInt->finished();

    ASSERT_EQ(m_data, "{\"value\":[-2,0,1,123]}");
}


