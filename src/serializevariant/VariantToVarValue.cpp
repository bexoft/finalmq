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


#include "finalmq/serializevariant/VariantToVarValue.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/variant/VariantValues.h"

#include <assert.h>


namespace finalmq {


const MetaStruct* VariantToVarValue::m_struct = nullptr;


VariantToVarValue::VariantToVarValue(Variant& variant, IParserVisitor& visitor)
    : m_variant(variant)
    , m_visitor(visitor)
{
}


void VariantToVarValue::convert()
{
    if (m_struct == nullptr)
    {
        m_struct = MetaDataGlobal::instance().getStruct("finalmq.variant.VarValue");
    }
    assert(m_struct);
    m_variant.accept(*this);
}


// IVariantVisitor
void VariantToVarValue::enterLeaf(Variant& variant, int type, ssize_t /*index*/, int level, ssize_t /*size*/, const std::string& name)
{
    static const MetaField* fieldStruct = m_struct->getFieldByName("vallist");
    assert(fieldStruct);
    static const MetaField* fieldStructWithoutArray = MetaDataGlobal::instance().getArrayField(*fieldStruct);
    assert(fieldStructWithoutArray);

    if (level > 0)
    {
        m_visitor.enterStruct(*fieldStructWithoutArray);
    }

    if (!name.empty())
    {
        static const MetaField* fieldName = m_struct->getFieldByName("name");
        assert(fieldName);
        m_visitor.enterString(*fieldName, name.data(), name.size());
    }

    static const MetaField* fieldType = m_struct->getFieldByName("type");
    assert(fieldType);
    m_visitor.enterEnum(*fieldType, type);

    switch (type)
    {
    case TYPE_BOOL:
        static const MetaField* fieldBool = m_struct->getFieldByName("valbool");
        assert(fieldBool);
        m_visitor.enterBool(*fieldBool, variant);
        break;
    case TYPE_INT32:
        static const MetaField* fieldInt32 = m_struct->getFieldByName("valint32");
        assert(fieldInt32);
        m_visitor.enterInt32(*fieldInt32, variant);
        break;
    case TYPE_UINT32:
        static const MetaField* fieldUInt32 = m_struct->getFieldByName("valuint32");
        assert(fieldUInt32);
        m_visitor.enterUInt32(*fieldUInt32, variant);
        break;
    case TYPE_INT64:
        static const MetaField* fieldInt64 = m_struct->getFieldByName("valint64");
        assert(fieldInt64);
        m_visitor.enterInt64(*fieldInt64, variant);
        break;
    case TYPE_UINT64:
        static const MetaField* fieldUInt64 = m_struct->getFieldByName("valuint64");
        assert(fieldUInt64);
        m_visitor.enterUInt64(*fieldUInt64, variant);
        break;
    case TYPE_FLOAT:
        static const MetaField* fieldFloat = m_struct->getFieldByName("valfloat");
        assert(fieldFloat);
        m_visitor.enterFloat(*fieldFloat, variant);
        break;
    case TYPE_DOUBLE:
        static const MetaField* fieldDouble = m_struct->getFieldByName("valdouble");
        assert(fieldDouble);
        m_visitor.enterDouble(*fieldDouble, variant);
        break;
    case TYPE_STRING:
        {
            static const MetaField* fieldString = m_struct->getFieldByName("valstring");
            assert(fieldString);
            std::string* data = variant;
            assert(data);
            m_visitor.enterString(*fieldString, std::move(*data));
        }
        break;
    case TYPE_BYTES:
        {
            static const MetaField* fieldBytes = m_struct->getFieldByName("valbytes");
            assert(fieldBytes);
            Bytes* data = variant;
            assert(data);
            m_visitor.enterBytes(*fieldBytes, std::move(*data));
        }
        break;

    case TYPE_ARRAY_BOOL:
        static const MetaField* fieldArrBool = m_struct->getFieldByName("valarrbool");
        assert(fieldArrBool);
        m_visitor.enterArrayBoolMove(*fieldArrBool, variant);
        break;
    case TYPE_ARRAY_INT32:
        static const MetaField* fieldArrInt32 = m_struct->getFieldByName("valarrint32");
        assert(fieldArrInt32);
        m_visitor.enterArrayInt32(*fieldArrInt32, variant);
        break;
    case TYPE_ARRAY_UINT32:
        static const MetaField* fieldArrUInt32 = m_struct->getFieldByName("valarruint32");
        assert(fieldArrUInt32);
        m_visitor.enterArrayUInt32(*fieldArrUInt32, variant);
        break;
    case TYPE_ARRAY_INT64:
        static const MetaField* fieldArrInt64 = m_struct->getFieldByName("valarrint64");
        assert(fieldArrInt64);
        m_visitor.enterArrayInt64(*fieldArrInt64, variant);
        break;
    case TYPE_ARRAY_UINT64:
        static const MetaField* fieldArrUInt64 = m_struct->getFieldByName("valarruint64");
        assert(fieldArrUInt64);
        m_visitor.enterArrayUInt64(*fieldArrUInt64, variant);
        break;
    case TYPE_ARRAY_FLOAT:
        static const MetaField* fieldArrFloat = m_struct->getFieldByName("valarrfloat");
        assert(fieldArrFloat);
        m_visitor.enterArrayFloat(*fieldArrFloat, variant);
        break;
    case TYPE_ARRAY_DOUBLE:
        static const MetaField* fieldArrDouble = m_struct->getFieldByName("valarrdouble");
        assert(fieldArrDouble);
        m_visitor.enterArrayDouble(*fieldArrDouble, variant);
        break;
    case TYPE_ARRAY_STRING:
        static const MetaField* fieldArrString = m_struct->getFieldByName("valarrstring");
        assert(fieldArrString);
        m_visitor.enterArrayString(*fieldArrString, variant);
        break;
    case TYPE_ARRAY_BYTES:
        static const MetaField* fieldArrBytes = m_struct->getFieldByName("valarrbytes");
        assert(fieldArrBytes);
        m_visitor.enterArrayBytes(*fieldArrBytes, variant);
        break;
    }

    if (level > 0)
    {
        m_visitor.exitStruct(*fieldStructWithoutArray);
    }
}

void VariantToVarValue::enterStruct(Variant& /*variant*/, int type, ssize_t /*index*/, int level, ssize_t /*size*/, const std::string& name)
{
    static const MetaField* fieldStruct = m_struct->getFieldByName("vallist");
    assert(fieldStruct);
    static const MetaField* fieldStructWithoutArray = MetaDataGlobal::instance().getArrayField(*fieldStruct);
    assert(fieldStructWithoutArray);

    if (level > 0)
    {
        m_visitor.enterStruct(*fieldStructWithoutArray);
    }

    if (!name.empty())
    {
        static const MetaField* fieldName = m_struct->getFieldByName("name");
        assert(fieldName);
        m_visitor.enterString(*fieldName, name.data(), name.size());
    }

    static const MetaField* fieldType = m_struct->getFieldByName("type");
    assert(fieldType);
    m_visitor.enterEnum(*fieldType, type);

    m_visitor.enterArrayStruct(*fieldStruct);
}

void VariantToVarValue::exitStruct(Variant& /*variant*/, int /*type*/, ssize_t /*index*/, int level, ssize_t /*size*/, const std::string& /*name*/)
{
    static const MetaField* fieldStruct = m_struct->getFieldByName("vallist");
    assert(fieldStruct);
    m_visitor.exitArrayStruct(*fieldStruct);
    static const MetaField* fieldStructWithoutArray = MetaDataGlobal::instance().getArrayField(*fieldStruct);
    assert(fieldStructWithoutArray);

    if (level > 0)
    {
        m_visitor.exitStruct(*fieldStructWithoutArray);
    }
}

void VariantToVarValue::enterList(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name)
{
    enterStruct(variant, type, index, level, size, name);
}

void VariantToVarValue::exitList(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name)
{
    exitStruct(variant, type, index, level, size, name);
}



}   // namespace finalmq
