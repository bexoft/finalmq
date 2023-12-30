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


#include "finalmq/serializevariant/VarValueToVariant.h"
#include "finalmq/metadataserialize/variant.fmq.h"
#include "finalmq/metadata/MetaData.h"

#include "finalmq/variant/VariantValues.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"



//using ::testing::_;
//using ::testing::Return;
//using ::testing::ReturnRef;
//using testing::Invoke;
//using testing::DoAll;

using namespace finalmq;
using variant::VarValue;




class TestVarValueToVariant : public testing::Test
{
public:
    TestVarValueToVariant()
        : m_root()
        , m_varValueToVariant(m_root)
        , m_visitor(m_varValueToVariant.getVisitor())
    {
    }

protected:
    virtual void SetUp()
    {
        const MetaStruct* structVarVariant = MetaDataGlobal::instance().getStruct("finalmq.variant.VarValue");
        ASSERT_NE(structVarVariant, nullptr);

        m_fieldName = structVarVariant->getFieldByName("name");
        m_fieldType = structVarVariant->getFieldByName("type");

        m_fieldBool = structVarVariant->getFieldByName("valbool");
        m_fieldInt8 = structVarVariant->getFieldByName("valint8");
        m_fieldUInt8 = structVarVariant->getFieldByName("valuint8");
        m_fieldInt16 = structVarVariant->getFieldByName("valint16");
        m_fieldUInt16 = structVarVariant->getFieldByName("valuint16");
        m_fieldInt32 = structVarVariant->getFieldByName("valint32");
        m_fieldUInt32 = structVarVariant->getFieldByName("valuint32");
        m_fieldInt64 = structVarVariant->getFieldByName("valint64");
        m_fieldUInt64 = structVarVariant->getFieldByName("valuint64");
        m_fieldFloat = structVarVariant->getFieldByName("valfloat");
        m_fieldDouble = structVarVariant->getFieldByName("valdouble");
        m_fieldString = structVarVariant->getFieldByName("valstring");
        m_fieldBytes = structVarVariant->getFieldByName("valbytes");
        m_fieldList = structVarVariant->getFieldByName("vallist");

        m_fieldArrBool = structVarVariant->getFieldByName("valarrbool");
        m_fieldArrInt8 = structVarVariant->getFieldByName("valarrint8");
        m_fieldArrInt16 = structVarVariant->getFieldByName("valarrint16");
        m_fieldArrUInt16 = structVarVariant->getFieldByName("valarruint16");
        m_fieldArrInt32 = structVarVariant->getFieldByName("valarrint32");
        m_fieldArrUInt32 = structVarVariant->getFieldByName("valarruint32");
        m_fieldArrInt64 = structVarVariant->getFieldByName("valarrint64");
        m_fieldArrUInt64 = structVarVariant->getFieldByName("valarruint64");
        m_fieldArrFloat = structVarVariant->getFieldByName("valarrfloat");
        m_fieldArrDouble = structVarVariant->getFieldByName("valarrdouble");
        m_fieldArrString = structVarVariant->getFieldByName("valarrstring");
        m_fieldArrBytes = structVarVariant->getFieldByName("valarrbytes");

        ASSERT_NE(m_fieldName, nullptr);
        ASSERT_NE(m_fieldType, nullptr);

        ASSERT_NE(m_fieldBool, nullptr);
        ASSERT_NE(m_fieldInt32, nullptr);
        ASSERT_NE(m_fieldUInt32, nullptr);
        ASSERT_NE(m_fieldInt64, nullptr);
        ASSERT_NE(m_fieldUInt64, nullptr);
        ASSERT_NE(m_fieldFloat, nullptr);
        ASSERT_NE(m_fieldDouble, nullptr);
        ASSERT_NE(m_fieldString, nullptr);
        ASSERT_NE(m_fieldBytes, nullptr);
        ASSERT_NE(m_fieldList, nullptr);

        ASSERT_NE(m_fieldArrBool, nullptr);
        ASSERT_NE(m_fieldArrInt8, nullptr);
        ASSERT_NE(m_fieldArrInt16, nullptr);
        ASSERT_NE(m_fieldArrUInt16, nullptr);
        ASSERT_NE(m_fieldArrInt32, nullptr);
        ASSERT_NE(m_fieldArrUInt32, nullptr);
        ASSERT_NE(m_fieldArrInt64, nullptr);
        ASSERT_NE(m_fieldArrUInt64, nullptr);
        ASSERT_NE(m_fieldArrFloat, nullptr);
        ASSERT_NE(m_fieldArrDouble, nullptr);
        ASSERT_NE(m_fieldArrString, nullptr);
        ASSERT_NE(m_fieldArrBytes, nullptr);
    }

    virtual void TearDown()
    {
    }

    Variant                 m_root;
    VarValueToVariant       m_varValueToVariant;
    IParserVisitor&         m_visitor;

    const MetaField*        m_fieldName = nullptr;
    const MetaField*        m_fieldType = nullptr;

    const MetaField*        m_fieldBool = nullptr;
    const MetaField*        m_fieldInt8 = nullptr;
    const MetaField*        m_fieldUInt8 = nullptr;
    const MetaField*        m_fieldInt16 = nullptr;
    const MetaField*        m_fieldUInt16 = nullptr;
    const MetaField*        m_fieldInt32 = nullptr;
    const MetaField*        m_fieldUInt32 = nullptr;
    const MetaField*        m_fieldInt64 = nullptr;
    const MetaField*        m_fieldUInt64 = nullptr;
    const MetaField*        m_fieldDouble = nullptr;
    const MetaField*        m_fieldFloat = nullptr;
    const MetaField*        m_fieldString = nullptr;
    const MetaField*        m_fieldBytes = nullptr;
    const MetaField*        m_fieldList = nullptr;

    const MetaField*        m_fieldArrBool = nullptr;
    const MetaField*        m_fieldArrInt8 = nullptr;
    const MetaField*        m_fieldArrInt16 = nullptr;
    const MetaField*        m_fieldArrUInt16 = nullptr;
    const MetaField*        m_fieldArrInt32 = nullptr;
    const MetaField*        m_fieldArrUInt32 = nullptr;
    const MetaField*        m_fieldArrInt64 = nullptr;
    const MetaField*        m_fieldArrUInt64 = nullptr;
    const MetaField*        m_fieldArrDouble = nullptr;
    const MetaField*        m_fieldArrFloat = nullptr;
    const MetaField*        m_fieldArrString = nullptr;
    const MetaField*        m_fieldArrBytes = nullptr;
};




TEST_F(TestVarValueToVariant, testBool)
{
    static const bool VALUE = true;

    m_visitor.enterBool(*m_fieldBool, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_BOOL);

    m_varValueToVariant.convert();

    bool* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testInt8)
{
    static const std::int8_t VALUE = -123;

    m_visitor.enterInt8(*m_fieldInt8, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT8);

    m_varValueToVariant.convert();

    std::int8_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testUInt8)
{
    static const std::uint8_t VALUE = 123;

    m_visitor.enterUInt8(*m_fieldUInt8, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_UINT8);

    m_varValueToVariant.convert();

    std::uint8_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testInt16)
{
    static const std::int16_t VALUE = -123;

    m_visitor.enterInt16(*m_fieldInt16, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT16);

    m_varValueToVariant.convert();

    std::int16_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testUInt16)
{
    static const std::uint16_t VALUE = 123;

    m_visitor.enterUInt16(*m_fieldUInt16, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_UINT16);

    m_varValueToVariant.convert();

    std::uint16_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testInt32)
{
    static const std::int32_t VALUE = -123;

    m_visitor.enterInt32(*m_fieldInt32, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT32);

    m_varValueToVariant.convert();

    std::int32_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testUInt32)
{
    static const std::uint32_t VALUE = 123;

    m_visitor.enterUInt32(*m_fieldUInt32, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_UINT32);

    m_varValueToVariant.convert();

    std::uint32_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testInt64)
{
    static const std::int64_t VALUE = -123;

    m_visitor.enterInt64(*m_fieldInt64, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT64);

    m_varValueToVariant.convert();

    std::int64_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testUInt64)
{
    static const std::uint64_t VALUE = 123;

    m_visitor.enterUInt64(*m_fieldUInt64, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_UINT64);

    m_varValueToVariant.convert();

    std::uint64_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testFloat)
{
    static const float VALUE = -123.4f;

    m_visitor.enterFloat(*m_fieldFloat, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_FLOAT);

    m_varValueToVariant.convert();

    float* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testDouble)
{
    static const double VALUE = -123.4;

    m_visitor.enterDouble(*m_fieldDouble, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_DOUBLE);

    m_varValueToVariant.convert();

    double* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testString)
{
    static const std::string VALUE = "Hello";

    m_visitor.enterString(*m_fieldString, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_STRING);

    m_varValueToVariant.convert();

    std::string* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testBytes)
{
    static const Bytes VALUE = { 0x12, 0x00, 0x14 };

    m_visitor.enterBytes(*m_fieldBytes, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_BYTES);

    m_varValueToVariant.convert();

    Bytes* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testStruct)
{
    static const std::int32_t VALUE_INT32 = 123;
    static const std::string VALUE_STRING = "Hello World";

    const MetaField* fieldEntry = MetaDataGlobal::instance().getArrayField(*m_fieldList);
    ASSERT_NE(fieldEntry, nullptr);

    m_visitor.enterArrayStruct(*m_fieldList);

    m_visitor.enterStruct(*fieldEntry);
    m_visitor.enterInt32(*m_fieldInt32, 123);
    m_visitor.enterString(*m_fieldName, "key1");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT32);
    m_visitor.exitStruct(*fieldEntry);

    m_visitor.enterStruct(*fieldEntry);
    m_visitor.enterString(*m_fieldString, VALUE_STRING.data(), VALUE_STRING.size());
    m_visitor.enterString(*m_fieldName, "key2");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_STRING);
    m_visitor.exitStruct(*fieldEntry);

    m_visitor.exitArrayStruct(*m_fieldList);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_STRUCT);

    m_varValueToVariant.convert();

    VariantStruct* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(m_root.getDataValue<std::int32_t>("key1"), VALUE_INT32);
    ASSERT_EQ(m_root.getDataValue<std::string>("key2"), VALUE_STRING);
}

TEST_F(TestVarValueToVariant, testArrayBool)
{
    static const std::vector<bool> VALUE = { true, false, false, true, false };

    m_visitor.enterArrayBool(*m_fieldArrBool, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_BOOL);

    m_varValueToVariant.convert();

    std::vector<bool>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayInt8)
{
    static const std::vector<std::int8_t> VALUE = { -123, 0, 123, 12, -12 };

    m_visitor.enterArrayInt8(*m_fieldArrInt8, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_INT8);

    m_varValueToVariant.convert();

    std::vector<std::int8_t>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayInt16)
{
    static const std::vector<std::int16_t> VALUE = { -123, 0, 123, 12345, -12345 };

    m_visitor.enterArrayInt16(*m_fieldArrInt16, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_INT16);

    m_varValueToVariant.convert();

    std::vector<std::int16_t>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayUInt16)
{
    static const std::vector<std::uint16_t> VALUE = { 123, 0, 12345, 8000, 55374 };

    m_visitor.enterArrayUInt16(*m_fieldArrUInt16, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_UINT16);

    m_varValueToVariant.convert();

    std::vector<std::uint16_t>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayInt32)
{
    static const std::vector<std::int32_t> VALUE = { -123, 0, 123, 12345, -12345 };

    m_visitor.enterArrayInt32(*m_fieldArrInt32, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_INT32);

    m_varValueToVariant.convert();

    std::vector<std::int32_t>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayUInt32)
{
    static const std::vector<std::uint32_t> VALUE = { 123, 0, 12345, 8000, 982374 };

    m_visitor.enterArrayUInt32(*m_fieldArrUInt32, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_UINT32);

    m_varValueToVariant.convert();

    std::vector<std::uint32_t>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayInt64)
{
    static const std::vector<std::int64_t> VALUE = { -123, 0, 123, 12345, -12345 };

    m_visitor.enterArrayInt64(*m_fieldArrInt64, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_INT64);

    m_varValueToVariant.convert();

    std::vector<std::int64_t>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayUInt64)
{
    static const std::vector<std::uint64_t> VALUE = { 123, 0, 12345, 8000, 982374 };

    m_visitor.enterArrayUInt64(*m_fieldArrUInt64, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_UINT64);

    m_varValueToVariant.convert();

    std::vector<std::uint64_t>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayFloat)
{
    static const std::vector<float> VALUE = { -123.4f, 0.0f, 123.4f, 872.28f, -12.13f};

    m_visitor.enterArrayFloat(*m_fieldArrFloat, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_FLOAT);

    m_varValueToVariant.convert();

    std::vector<float>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayDouble)
{
    static const std::vector<double> VALUE = { -123.4, 0.0, 123.4, 872.28, -12.13 };

    m_visitor.enterArrayDouble(*m_fieldArrDouble, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_DOUBLE);

    m_varValueToVariant.convert();

    std::vector<double>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayString)
{
    static const std::vector<std::string> VALUE = { "Hello", "", "World", "Foo", "42" };

    m_visitor.enterArrayString(*m_fieldArrString, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_STRING);

    m_varValueToVariant.convert();

    std::vector<std::string>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayBytes)
{
    static const std::vector<Bytes> VALUE = { { 0x12, 0x00, 0x14 }, {}, { (BytesElement)0xff, (BytesElement)0xaa, (BytesElement)0xee } };

    m_visitor.enterArrayBytes(*m_fieldArrBytes, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_BYTES);

    m_varValueToVariant.convert();

    std::vector<Bytes>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testList)
{
    static const std::int32_t VALUE_INT32 = 123;
    static const std::string VALUE_STRING = "Hello World";

    const MetaField* fieldEntry = MetaDataGlobal::instance().getArrayField(*m_fieldList);
    ASSERT_NE(fieldEntry, nullptr);

    m_visitor.enterArrayStruct(*m_fieldList);

    m_visitor.enterStruct(*fieldEntry);
    m_visitor.enterInt32(*m_fieldInt32, 123);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT32);
    m_visitor.exitStruct(*fieldEntry);

    m_visitor.enterStruct(*fieldEntry);
    m_visitor.enterString(*m_fieldString, VALUE_STRING.data(), VALUE_STRING.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_STRING);
    m_visitor.exitStruct(*fieldEntry);

    m_visitor.exitArrayStruct(*m_fieldList);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_LIST);

    m_varValueToVariant.convert();

    VariantList* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(m_root.getDataValue<std::int32_t>("0"), VALUE_INT32);
    ASSERT_EQ(m_root.getDataValue<std::string>("1"), VALUE_STRING);
}

