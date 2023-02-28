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


const MetaStruct* VariantToVarValue::m_structVarValue = nullptr;


VariantToVarValue::VariantToVarValue(const Variant& variant, IParserVisitor& visitor)
    : m_variant(const_cast<Variant&>(variant))
    , m_visitor(visitor)
{
}


void VariantToVarValue::convert()
{
    if (m_structVarValue == nullptr)
    {
        m_structVarValue = MetaDataGlobal::instance().getStruct("finalmq.variant.VarValue");
    }
    assert(m_structVarValue);
    m_variant.accept(*this);
}


// IVariantVisitor

void VariantToVarValue::enterLeaf(Variant& var, int type, ssize_t /*index*/, int level, ssize_t /*size*/, const std::string& name)
{
    const Variant& variant = var;

    static const MetaField* fieldStruct = m_structVarValue->getFieldByName("vallist");
    assert(fieldStruct);
    static const MetaField* fieldStructWithoutArray = fieldStruct->fieldWithoutArray;
    assert(fieldStructWithoutArray);

    if (level > 0)
    {
        m_visitor.enterStruct(*fieldStructWithoutArray);
    }

    if (!name.empty())
    {
        static const MetaField* fieldName = m_structVarValue->getFieldByName("name");
        assert(fieldName);
        m_visitor.enterString(*fieldName, name.data(), name.size());
    }

    static const MetaField* fieldType = m_structVarValue->getFieldByName("type");
    assert(fieldType);
    m_visitor.enterEnum(*fieldType, type);

    switch (type)
    {
    case TYPE_NONE:
        break;
    case TYPE_BOOL:
        {
            static const MetaField* fieldBool = m_structVarValue->getFieldByName("valbool");
            assert(fieldBool);
            const bool* data = variant;
            assert(data);
            m_visitor.enterBool(*fieldBool, *data);
        }
        break;
    case TYPE_INT32:
        {
            static const MetaField* fieldInt32 = m_structVarValue->getFieldByName("valint32");
            assert(fieldInt32);
            const std::int32_t* data = variant;
            assert(data);
            m_visitor.enterInt32(*fieldInt32, *data);
        }
        break;
    case TYPE_UINT32:
        {
            static const MetaField* fieldUInt32 = m_structVarValue->getFieldByName("valuint32");
            assert(fieldUInt32);
            const std::uint32_t* data = variant;
            assert(data);
            m_visitor.enterUInt32(*fieldUInt32, *data);
        }
        break;
    case TYPE_INT64:
        {
            static const MetaField* fieldInt64 = m_structVarValue->getFieldByName("valint64");
            assert(fieldInt64);
            const std::int64_t* data = variant;
            assert(data);
            m_visitor.enterInt64(*fieldInt64, *data);
        }
        break;
    case TYPE_UINT64:
        {
            static const MetaField* fieldUInt64 = m_structVarValue->getFieldByName("valuint64");
            assert(fieldUInt64);
            const std::uint64_t* data = variant;
            assert(data);
            m_visitor.enterUInt64(*fieldUInt64, *data);
        }
        break;
    case TYPE_FLOAT:
        {
            static const MetaField* fieldFloat = m_structVarValue->getFieldByName("valfloat");
            assert(fieldFloat);
            const float* data = variant;
            assert(data);
            m_visitor.enterFloat(*fieldFloat, *data);
        }
        break;
    case TYPE_DOUBLE:
        {
            static const MetaField* fieldDouble = m_structVarValue->getFieldByName("valdouble");
            assert(fieldDouble);
            const double* data = variant;
            assert(data);
            m_visitor.enterDouble(*fieldDouble, *data);
        }
        break;
    case TYPE_STRING:
        {
            static const MetaField* fieldString = m_structVarValue->getFieldByName("valstring");
            assert(fieldString);
            const std::string* data = variant;
            assert(data);
            m_visitor.enterString(*fieldString, data->data(), data->size());
        }
        break;
    case TYPE_BYTES:
        {
            static const MetaField* fieldBytes = m_structVarValue->getFieldByName("valbytes");
            assert(fieldBytes);
            const Bytes* data = variant;
            assert(data);
            m_visitor.enterBytes(*fieldBytes, data->data(), data->size());
        }
        break;

    case TYPE_ARRAY_BOOL:
        {
            static const MetaField* fieldArrBool = m_structVarValue->getFieldByName("valarrbool");
            assert(fieldArrBool);
            const std::vector<bool>* data = variant;
            assert(data);
            m_visitor.enterArrayBool(*fieldArrBool, *data);
        }
        break;
    case TYPE_ARRAY_INT32:
        {
            static const MetaField* fieldArrInt32 = m_structVarValue->getFieldByName("valarrint32");
            assert(fieldArrInt32);
            const std::vector<std::int32_t>* data = variant;
            assert(data);
            m_visitor.enterArrayInt32(*fieldArrInt32, data->data(), data->size());
        }
        break;
    case TYPE_ARRAY_UINT32:
        {
            static const MetaField* fieldArrUInt32 = m_structVarValue->getFieldByName("valarruint32");
            assert(fieldArrUInt32);
            const std::vector<std::uint32_t>* data = variant;
            assert(data);
            m_visitor.enterArrayUInt32(*fieldArrUInt32, data->data(), data->size());
        }
        break;
    case TYPE_ARRAY_INT64:
        {
            static const MetaField* fieldArrInt64 = m_structVarValue->getFieldByName("valarrint64");
            assert(fieldArrInt64);
            const std::vector<std::int64_t>* data = variant;
            assert(data);
            m_visitor.enterArrayInt64(*fieldArrInt64, data->data(), data->size());
        }
        break;
    case TYPE_ARRAY_UINT64:
        {
            static const MetaField* fieldArrUInt64 = m_structVarValue->getFieldByName("valarruint64");
            assert(fieldArrUInt64);
            const std::vector<std::uint64_t>* data = variant;
            assert(data);
            m_visitor.enterArrayUInt64(*fieldArrUInt64, data->data(), data->size());
        }
        break;
    case TYPE_ARRAY_FLOAT:
        {
            static const MetaField* fieldArrFloat = m_structVarValue->getFieldByName("valarrfloat");
            assert(fieldArrFloat);
            const std::vector<float>* data = variant;
            assert(data);
            m_visitor.enterArrayFloat(*fieldArrFloat, data->data(), data->size());
        }
        break;
    case TYPE_ARRAY_DOUBLE:
        {
            static const MetaField* fieldArrDouble = m_structVarValue->getFieldByName("valarrdouble");
            assert(fieldArrDouble);
            const std::vector<double>* data = variant;
            assert(data);
            m_visitor.enterArrayDouble(*fieldArrDouble, data->data(), data->size());
        }
        break;
    case TYPE_ARRAY_STRING:
        {
            static const MetaField* fieldArrString = m_structVarValue->getFieldByName("valarrstring");
            assert(fieldArrString);
            const std::vector<std::string>* data = variant;
            assert(data);
            m_visitor.enterArrayString(*fieldArrString, *data);
        }
        break;
    case TYPE_ARRAY_BYTES:
        {
            static const MetaField* fieldArrBytes = m_structVarValue->getFieldByName("valarrbytes");
            assert(fieldArrBytes);
            const std::vector<Bytes>* data = variant;
            assert(data);
            m_visitor.enterArrayBytes(*fieldArrBytes, *data);
        }
        break;
    }

    if (level > 0)
    {
        m_visitor.exitStruct(*fieldStructWithoutArray);
    }
}

void VariantToVarValue::enterStruct(Variant& /*variant*/, int type, ssize_t /*index*/, int level, ssize_t /*size*/, const std::string& name)
{
    static const MetaField* fieldList = m_structVarValue->getFieldByName("vallist");
    assert(fieldList);
    static const MetaField* fieldListWithoutArray = fieldList->fieldWithoutArray;
    assert(fieldListWithoutArray);

    if (level > 0)
    {
        m_visitor.enterStruct(*fieldListWithoutArray);
    }

    if (!name.empty())
    {
        static const MetaField* fieldName = m_structVarValue->getFieldByName("name");
        assert(fieldName);
        m_visitor.enterString(*fieldName, name.data(), name.size());
    }

    static const MetaField* fieldType = m_structVarValue->getFieldByName("type");
    assert(fieldType);
    m_visitor.enterEnum(*fieldType, type);

    m_visitor.enterArrayStruct(*fieldList);
}

void VariantToVarValue::exitStruct(Variant& /*variant*/, int /*type*/, ssize_t /*index*/, int level, ssize_t /*size*/, const std::string& /*name*/)
{
    static const MetaField* fieldList = m_structVarValue->getFieldByName("vallist");
    assert(fieldList);
    m_visitor.exitArrayStruct(*fieldList);
    static const MetaField* fieldListWithoutArray = fieldList->fieldWithoutArray;
    assert(fieldListWithoutArray);

    if (level > 0)
    {
        m_visitor.exitStruct(*fieldListWithoutArray);
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
