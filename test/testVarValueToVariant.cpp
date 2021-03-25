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
        const MetaStruct* structTestVariant = MetaDataGlobal::instance().getStruct("test.TestVariant");
        ASSERT_NE(structTestVariant, nullptr);
        m_fieldValue = structTestVariant->getFieldByName("value");
        ASSERT_NE(m_fieldValue, nullptr);
        ASSERT_EQ(m_fieldValue->typeName, "finalmq.variant.VarValue");
        
        const MetaStruct* structVarVariant = MetaDataGlobal::instance().getStruct("finalmq.variant.VarValue");
        ASSERT_NE(structVarVariant, nullptr);

        m_fieldName = structVarVariant->getFieldByName("name");
        m_fieldType = structVarVariant->getFieldByName("type");

        m_fieldBool = structVarVariant->getFieldByName("valbool");
        m_fieldInt32 = structVarVariant->getFieldByName("valint32");
        m_fieldUInt32 = structVarVariant->getFieldByName("valuint32");
        m_fieldInt64 = structVarVariant->getFieldByName("valint64");
        m_fieldUInt64 = structVarVariant->getFieldByName("valuint64");
        m_fieldFloat = structVarVariant->getFieldByName("valfloat");
        m_fieldDouble = structVarVariant->getFieldByName("valdouble");
        m_fieldString = structVarVariant->getFieldByName("valstring");
        m_fieldBytes = structVarVariant->getFieldByName("valbytes");

        m_fieldArrBool = structVarVariant->getFieldByName("valarrbool");

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

        ASSERT_NE(m_fieldArrBool, nullptr);
    }

    virtual void TearDown()
    {
    }

    Variant                 m_root;
    VarValueToVariant       m_varValueToVariant;
    IParserVisitor&         m_visitor;
    const MetaField*        m_fieldValue = nullptr;
    const MetaField*        m_fieldName = nullptr;
    const MetaField*        m_fieldType = nullptr;

    const MetaField*        m_fieldBool = nullptr;
    const MetaField*        m_fieldInt32 = nullptr;
    const MetaField*        m_fieldUInt32 = nullptr;
    const MetaField*        m_fieldInt64 = nullptr;
    const MetaField*        m_fieldUInt64 = nullptr;
    const MetaField*        m_fieldDouble = nullptr;
    const MetaField*        m_fieldFloat = nullptr;
    const MetaField*        m_fieldString = nullptr;
    const MetaField*        m_fieldBytes = nullptr;

    const MetaField*        m_fieldArrBool = nullptr;
};




TEST_F(TestVarValueToVariant, testBool)
{
    static const bool VALUE = true;

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterBool(*m_fieldBool, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_BOOL);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    bool* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testInt32)
{
    static const std::int32_t VALUE = -123;

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterInt32(*m_fieldInt32, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT32);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    std::int32_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testUInt32)
{
    static const std::uint32_t VALUE = 123;

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterUInt32(*m_fieldUInt32, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_UINT32);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    std::uint32_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testInt64)
{
    static const std::int64_t VALUE = -123;

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterInt64(*m_fieldInt64, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_INT64);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    std::int64_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testUInt64)
{
    static const std::uint64_t VALUE = 123;

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterUInt64(*m_fieldUInt64, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_UINT64);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    std::uint64_t* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testFloat)
{
    static const float VALUE = -123.4f;

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterFloat(*m_fieldFloat, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_FLOAT);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    float* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testDouble)
{
    static const double VALUE = -123.4;

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterDouble(*m_fieldDouble, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_DOUBLE);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    double* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testString)
{
    static const std::string VALUE = "Hello";

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterString(*m_fieldString, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_STRING);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    std::string* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testBytes)
{
    static const Bytes VALUE = {0x12, 0x00, 0x14};

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterBytes(*m_fieldBytes, VALUE.data(), VALUE.size());
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_BYTES);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    Bytes* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

TEST_F(TestVarValueToVariant, testArrayBool)
{
    static const std::vector<bool> VALUE = { true, false, false, true, false };

    m_visitor.enterStruct(*m_fieldValue);
    m_visitor.enterArrayBool(*m_fieldArrBool, VALUE);
    m_visitor.enterString(*m_fieldName, "");
    m_visitor.enterEnum(*m_fieldType, variant::VarTypeId::T_ARRAY_BOOL);
    m_visitor.exitStruct(*m_fieldValue);

    m_varValueToVariant.convert();

    std::vector<bool>* val = m_root;
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, VALUE);
}

