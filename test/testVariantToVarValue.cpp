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


#include "finalmq/serializevariant/VariantToVarValue.h"
#include "finalmq/metadataserialize/variant.fmq.h"
#include "finalmq/metadata/MetaData.h"

#include "finalmq/variant/VariantValues.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"

#include "MockIParserVisitor.h"

using ::testing::_;
using ::testing::InSequence;
using testing::StrEq;

using namespace finalmq;
using variant::VarValue;



MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.typeId == metaField.typeId &&
        arg.typeName == metaField.typeName &&
        arg.name == metaField.name);
}


MATCHER_P2(ArrayEq, compareArray, n, "")
{
    return (memcmp(arg, compareArray, n * sizeof(*arg)) == 0);
}



class TestVariantToVarValue : public testing::Test
{
public:
    TestVariantToVarValue()
        : m_root()
        , m_mockIParserVisitor(std::make_unique<MockIParserVisitor>())
        , m_VariantToVarValue(m_root, *m_mockIParserVisitor)
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

        m_fieldListWithoutArray = MetaDataGlobal::instance().getArrayField(*m_fieldList);

        ASSERT_NE(m_fieldName, nullptr);
        ASSERT_NE(m_fieldType, nullptr);

        ASSERT_NE(m_fieldBool, nullptr);
        ASSERT_NE(m_fieldInt8, nullptr);
        ASSERT_NE(m_fieldUInt8, nullptr);
        ASSERT_NE(m_fieldInt16, nullptr);
        ASSERT_NE(m_fieldUInt16, nullptr);
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

        ASSERT_NE(m_fieldListWithoutArray, nullptr);
    }

    virtual void TearDown()
    {
    }

    Variant                                 m_root;
    std::unique_ptr<MockIParserVisitor>     m_mockIParserVisitor;
    VariantToVarValue                       m_VariantToVarValue;
    const MetaField* m_fieldValue = nullptr;
    const MetaField* m_fieldName = nullptr;
    const MetaField* m_fieldType = nullptr;

    const MetaField* m_fieldBool = nullptr;
    const MetaField* m_fieldInt8 = nullptr;
    const MetaField* m_fieldUInt8 = nullptr;
    const MetaField* m_fieldInt16 = nullptr;
    const MetaField* m_fieldUInt16 = nullptr;
    const MetaField* m_fieldInt32 = nullptr;
    const MetaField* m_fieldUInt32 = nullptr;
    const MetaField* m_fieldInt64 = nullptr;
    const MetaField* m_fieldUInt64 = nullptr;
    const MetaField* m_fieldDouble = nullptr;
    const MetaField* m_fieldFloat = nullptr;
    const MetaField* m_fieldString = nullptr;
    const MetaField* m_fieldBytes = nullptr;
    const MetaField* m_fieldList = nullptr;

    const MetaField* m_fieldArrBool = nullptr;
    const MetaField* m_fieldArrInt8 = nullptr;
    const MetaField* m_fieldArrInt16 = nullptr;
    const MetaField* m_fieldArrUInt16 = nullptr;
    const MetaField* m_fieldArrInt32 = nullptr;
    const MetaField* m_fieldArrUInt32 = nullptr;
    const MetaField* m_fieldArrInt64 = nullptr;
    const MetaField* m_fieldArrUInt64 = nullptr;
    const MetaField* m_fieldArrDouble = nullptr;
    const MetaField* m_fieldArrFloat = nullptr;
    const MetaField* m_fieldArrString = nullptr;
    const MetaField* m_fieldArrBytes = nullptr;

    const MetaField* m_fieldListWithoutArray = nullptr;
};




TEST_F(TestVariantToVarValue, testBool)
{
    const bool VALUE = true;
    m_root = VALUE;

    {
        InSequence seq;
    
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_BOOL)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterBool(MatcherMetaField(*m_fieldBool), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testInt8)
{
    const std::int8_t VALUE = -123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT8)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterInt8(MatcherMetaField(*m_fieldInt8), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testUInt8)
{
    const std::uint8_t VALUE = 123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_UINT8)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterUInt8(MatcherMetaField(*m_fieldUInt8), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testInt16)
{
    const std::int16_t VALUE = -123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT16)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterInt16(MatcherMetaField(*m_fieldInt16), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testUInt16)
{
    const std::uint16_t VALUE = 123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_UINT16)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterUInt16(MatcherMetaField(*m_fieldUInt16), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testInt32)
{
    const std::int32_t VALUE = -123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterInt32(MatcherMetaField(*m_fieldInt32), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testUInt32)
{
    const std::uint32_t VALUE = 123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_UINT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterUInt32(MatcherMetaField(*m_fieldUInt32), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testInt64)
{
    const std::int64_t VALUE = -123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT64)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterInt64(MatcherMetaField(*m_fieldInt64), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testUInt64)
{
    const std::uint64_t VALUE = 123;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_UINT64)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterUInt64(MatcherMetaField(*m_fieldUInt64), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testFloat)
{
    const float VALUE = -123.4f;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_FLOAT)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterFloat(MatcherMetaField(*m_fieldFloat), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testDouble)
{
    const double VALUE = 123.4;
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_DOUBLE)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterDouble(MatcherMetaField(*m_fieldDouble), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testString)
{
    std::string VALUE = "Hello World";
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRING)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq(VALUE.data()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testBytes)
{
    Bytes VALUE = { 0x12, 0x00, 0x14 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_BYTES)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterBytes(MatcherMetaField(*m_fieldBytes), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testStruct)
{
    static const std::int32_t VALUE_INT32 = 123;
    std::string VALUE_STRING = "Hello World";

    const MetaField* fieldEntry = MetaDataGlobal::instance().getArrayField(*m_fieldList);
    ASSERT_NE(fieldEntry, nullptr);

    m_root = VariantStruct{ {"key1", VALUE_INT32}, {"key2", VALUE_STRING} };

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRUCT)).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, enterArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, enterStruct(MatcherMetaField(*fieldEntry))).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key1"), 4)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterInt32(MatcherMetaField(*m_fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, exitStruct(MatcherMetaField(*fieldEntry))).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, enterStruct(MatcherMetaField(*fieldEntry))).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), StrEq("key2"), 4)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRING)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq(VALUE_STRING.data()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, exitStruct(MatcherMetaField(*fieldEntry))).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, exitArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
    }

    m_VariantToVarValue.convert();
}


TEST_F(TestVariantToVarValue, testArrayBool)
{
    std::vector<bool> VALUE = { true, false, false, true, false };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_BOOL)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayBool(MatcherMetaField(*m_fieldArrBool), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayInt8)
{
    std::vector<std::int8_t> VALUE = { -123, 0, 123, 24, -24 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_INT8)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayInt8(MatcherMetaField(*m_fieldArrInt8), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayInt16)
{
    std::vector<std::int16_t> VALUE = { -123, 0, 123, 12345, -12345 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_INT16)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayInt16(MatcherMetaField(*m_fieldArrInt16), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayUInt16)
{
    std::vector<std::uint16_t> VALUE = { 123, 0, 12345, 8000, 43744 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_UINT16)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayUInt16(MatcherMetaField(*m_fieldArrUInt16), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayInt32)
{
    std::vector<std::int32_t> VALUE = { -123, 0, 123, 12345, -12345 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_INT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayInt32(MatcherMetaField(*m_fieldArrInt32), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayUInt32)
{
    std::vector<std::uint32_t> VALUE = { 123, 0, 12345, 8000, 982374 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_UINT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayUInt32(MatcherMetaField(*m_fieldArrUInt32), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayInt64)
{
    std::vector<std::int64_t> VALUE = { -123, 0, 123, 12345, -12345 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_INT64)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayInt64(MatcherMetaField(*m_fieldArrInt64), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayUInt64)
{
    std::vector<std::uint64_t> VALUE = { 123, 0, 12345, 8000, 982374 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_UINT64)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayUInt64(MatcherMetaField(*m_fieldArrUInt64), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayFloat)
{
    std::vector<float> VALUE = { -123.4f, 0.0f, 123.4f, 872.28f, -12.13f };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_FLOAT)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayFloat(MatcherMetaField(*m_fieldArrFloat), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayDouble)
{
    std::vector<double> VALUE = { -123.4, 0.0, 123.4, 872.28, -12.13 };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_DOUBLE)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayDouble(MatcherMetaField(*m_fieldArrDouble), ArrayEq(VALUE.data(), VALUE.size()), VALUE.size())).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayString)
{
    std::vector<std::string> VALUE = { "Hello", "", "World", "Foo", "42" };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_STRING)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayString(MatcherMetaField(*m_fieldArrString), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}

TEST_F(TestVariantToVarValue, testArrayBytes)
{
    std::vector<Bytes> VALUE = { { 0x12, 0x00, 0x14 }, {}, { (BytesElement)0xff, (BytesElement)0xaa, (BytesElement)0xee } };
    m_root = VALUE;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_ARRAY_BYTES)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterArrayBytes(MatcherMetaField(*m_fieldArrBytes), VALUE)).Times(1);
    }

    m_VariantToVarValue.convert();
}


TEST_F(TestVariantToVarValue, testList)
{
    static const std::int32_t VALUE_INT32 = 123;
    std::string VALUE_STRING = "Hello World";

    const MetaField* fieldEntry = MetaDataGlobal::instance().getArrayField(*m_fieldList);
    ASSERT_NE(fieldEntry, nullptr);

    m_root = VariantList{ VALUE_INT32, VALUE_STRING };

    {
        InSequence seq;

        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_LIST)).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, enterArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, enterStruct(MatcherMetaField(*fieldEntry))).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_INT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterInt32(MatcherMetaField(*m_fieldInt32), VALUE_INT32)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, exitStruct(MatcherMetaField(*fieldEntry))).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, enterStruct(MatcherMetaField(*fieldEntry))).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_STRING)).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldString), StrEq(VALUE_STRING.data()), VALUE_STRING.size())).Times(1);
        EXPECT_CALL(*m_mockIParserVisitor, exitStruct(MatcherMetaField(*fieldEntry))).Times(1);

        EXPECT_CALL(*m_mockIParserVisitor, exitArrayStruct(MatcherMetaField(*m_fieldList))).Times(1);
    }

    m_VariantToVarValue.convert();
}
