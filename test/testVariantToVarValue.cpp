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
//using ::testing::Return;
//using ::testing::ReturnRef;
using testing::StrEq;
//using testing::DoAll;

using namespace finalmq;
using variant::VarValue;



MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.typeId == metaField.typeId &&
        arg.typeName == metaField.typeName &&
        arg.name == metaField.name);
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
    //const bool VALUE = true;
    //m_root = VALUE;

    //EXPECT_CALL(*m_mockIParserVisitor, enterStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);
    //EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _, _)).Times(0);
    //EXPECT_CALL(*m_mockIParserVisitor, enterString(MatcherMetaField(*m_fieldName), _)).Times(0);
    //EXPECT_CALL(*m_mockIParserVisitor, enterEnum(MatcherMetaField(*m_fieldType), variant::VarTypeId::T_BOOL)).Times(1);
    //EXPECT_CALL(*m_mockIParserVisitor, enterBool(MatcherMetaField(*m_fieldBool), VALUE)).Times(1);
    //EXPECT_CALL(*m_mockIParserVisitor, exitStruct(MatcherMetaField(*m_fieldListWithoutArray))).Times(1);

    //m_VariantToVarValue.convert();
}

