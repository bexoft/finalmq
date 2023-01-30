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


#include "finalmq/serializevariant/ParserVariant.h"
#include "finalmq/serializevariant/VariantToVarValue.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValueStruct.h"

#include <assert.h>

namespace finalmq {


static const std::string STR_VARVALUE = "finalmq.variant.VarValue";



ParserVariant::ParserVariant(IParserVisitor& visitor, const Variant& variant)
    : m_visitor(visitor)
    , m_root(variant)
{
}





bool ParserVariant::parseStruct(const std::string& typeName)
{
    bool ok = false;
    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(typeName);
    if (stru)
    {
        m_visitor.startStruct(*stru);
        parseStruct(*stru, m_root);
        ok = true;
    }
    else
    {
        m_visitor.notifyError(nullptr, "typename not found");
    }

    m_visitor.finished();
    return ok;
}



void ParserVariant::processField(const Variant* sub, const MetaField* field)
{

    switch (field->typeId)
    {
    case TYPE_BOOL:
        m_visitor.enterBool(*field, *sub);
        break;
    case TYPE_INT32:
        m_visitor.enterInt32(*field, *sub);
        break;
    case TYPE_UINT32:
        m_visitor.enterUInt32(*field, *sub);
        break;
    case TYPE_INT64:
        m_visitor.enterInt64(*field, *sub);
        break;
    case TYPE_UINT64:
        m_visitor.enterUInt64(*field, *sub);
        break;
    case TYPE_FLOAT:
        m_visitor.enterFloat(*field, *sub);
        break;
    case TYPE_DOUBLE:
        m_visitor.enterDouble(*field, *sub);
        break;
    case TYPE_STRING:
        {
            const std::string* str = *sub;
            if (str)
            {
                m_visitor.enterString(*field, str->c_str(), str->size());
            }
            else
            {
                m_visitor.enterString(*field, *sub);
            }
        }
        break;
    case TYPE_BYTES:
    {
        const Bytes* bytes = *sub;
        if (bytes)
        {
            m_visitor.enterBytes(*field, bytes->data(), bytes->size());
        }
        else
        {
            m_visitor.enterBytes(*field, *sub);
        }
    }
        break;
    case TYPE_STRUCT:
        if ((sub->getType() != VARTYPE_NONE) || !(field->flags & METAFLAG_NULLABLE))
        {
            m_visitor.enterStruct(*field);            
            if (field->typeName == STR_VARVALUE)
            {
                VariantToVarValue variantToVarValue(*sub, m_visitor);
                variantToVarValue.convert();
            }
            else
            {
                const MetaStruct* stru = MetaDataGlobal::instance().getStruct(*field);
                if (stru)
                {
                    parseStruct(*stru, *sub);
                }
                else
                {
                    m_visitor.notifyError(nullptr, "typename not found");
                }
            }
            m_visitor.exitStruct(*field);
        }
        else
        {
            m_visitor.enterStructNull(*field);
        }
        break;
    case TYPE_ENUM:
        {
            const std::string* str = *sub;
            if (str)
            {
                m_visitor.enterEnum(*field, str->c_str(), str->size());
            }
            else
            {
                std::int32_t value = *sub;
                m_visitor.enterEnum(*field, value);
            }
        }
        break;
    case TYPE_ARRAY_BOOL:
        {
            const std::vector<bool>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayBool(*field, *value);
            }
            else
            {
                m_visitor.enterArrayBool(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_INT32:
        {
            const std::vector<std::int32_t>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayInt32(*field, value->data(), value->size());
            }
            else
            {
                m_visitor.enterArrayInt32(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_UINT32:
        {
            const std::vector<std::uint32_t>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayUInt32(*field, value->data(), value->size());
            }
            else
            {
                m_visitor.enterArrayUInt32(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_INT64:
        {
            const std::vector<std::int64_t>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayInt64(*field, value->data(), value->size());
            }
            else
            {
                m_visitor.enterArrayInt64(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_UINT64:
        {
            const std::vector<std::uint64_t>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayUInt64(*field, value->data(), value->size());
            }
            else
            {
                m_visitor.enterArrayUInt64(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_FLOAT:
        {
            const std::vector<float>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayFloat(*field, value->data(), value->size());
            }
            else
            {
                m_visitor.enterArrayFloat(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_DOUBLE:
        {
            const std::vector<double>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayDouble(*field, value->data(), value->size());
            }
            else
            {
                m_visitor.enterArrayDouble(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_STRING:
        {
            const std::vector<std::string>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayString(*field, *value);
            }
            else
            {
                m_visitor.enterArrayString(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_BYTES:
        {
            const std::vector<Bytes>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayBytes(*field, *value);
            }
            else
            {
                m_visitor.enterArrayBytes(*field, *sub);
            }
        }
        break;
    case TYPE_ARRAY_STRUCT:
        {
            m_visitor.enterArrayStruct(*field);
            const VariantList* list = *sub;
            if (list)
            {
                const MetaField* fieldWithoutArray = MetaDataGlobal::instance().getArrayField(*field);
                assert(fieldWithoutArray);
                const MetaStruct* stru = MetaDataGlobal::instance().getStruct(*fieldWithoutArray);
                if (stru)
                {
                    ssize_t size = list->size();
                    for (ssize_t i = 0; i < size; ++i)
                    {
                        const Variant& element = list->at(i);
                        m_visitor.enterStruct(*fieldWithoutArray);
                        parseStruct(*stru, element);
                        m_visitor.exitStruct(*fieldWithoutArray);

                    }
                }
            }
            m_visitor.exitArrayStruct(*field);
        }
        break;
    case TYPE_ARRAY_ENUM:
        {
            bool done = false;
            const std::vector<std::string>* value = *sub;
            if (value)
            {
                m_visitor.enterArrayEnum(*field, *value);
                done = true;
            }
            else
            {
                const std::vector<std::int32_t>* value = *sub;
                if (value)
                {
                    m_visitor.enterArrayEnum(*field, value->data(), value->size());
                    done = true;
                }
            }
            if (!done)
            {
                m_visitor.enterArrayEnum(*field, std::vector<std::int32_t>());
            }
        }
        break;
    default:
        assert(false);
        break;
    }
}



void ParserVariant::processEmptyField(const MetaField* field)
{

    switch (field->typeId)
    {
    case TYPE_BOOL:
        m_visitor.enterBool(*field, bool());
        break;
    case TYPE_INT32:
        m_visitor.enterInt32(*field, std::int32_t());
        break;
    case TYPE_UINT32:
        m_visitor.enterUInt32(*field, std::uint32_t());
        break;
    case TYPE_INT64:
        m_visitor.enterInt64(*field, std::int64_t());
        break;
    case TYPE_UINT64:
        m_visitor.enterUInt64(*field, std::uint64_t());
        break;
    case TYPE_FLOAT:
        m_visitor.enterFloat(*field, float());
        break;
    case TYPE_DOUBLE:
        m_visitor.enterDouble(*field, double());
        break;
    case TYPE_STRING:
        m_visitor.enterString(*field, std::string());
        break;
    case TYPE_BYTES:
        m_visitor.enterBytes(*field, Bytes());
        break;
    case TYPE_STRUCT:
        if (!(field->flags & METAFLAG_NULLABLE))
        {
            m_visitor.enterStruct(*field);
            if (field->typeName == STR_VARVALUE)
            {
                VariantToVarValue variantToVarValue(Variant(), m_visitor);
                variantToVarValue.convert();
            }
            else
            {
                const MetaStruct* stru = MetaDataGlobal::instance().getStruct(*field);
                if (stru)
                {
                    parseStruct(*stru);
                }
                else
                {
                    m_visitor.notifyError(nullptr, "typename not found");
                }
            }
            m_visitor.exitStruct(*field);
        }
        else
        {
            m_visitor.enterStructNull(*field);
        }
        break;
    case TYPE_ENUM:
        m_visitor.enterEnum(*field, std::int32_t());
        break;
    case TYPE_ARRAY_BOOL:
        m_visitor.enterArrayBool(*field, std::vector<bool>());
        break;
    case TYPE_ARRAY_INT32:
        m_visitor.enterArrayInt32(*field, std::vector<std::int32_t>());
        break;
    case TYPE_ARRAY_UINT32:
        m_visitor.enterArrayUInt32(*field, std::vector<std::uint32_t>());
        break;
    case TYPE_ARRAY_INT64:
        m_visitor.enterArrayInt64(*field, std::vector<std::int64_t>());
        break;
    case TYPE_ARRAY_UINT64:
        m_visitor.enterArrayUInt64(*field, std::vector<std::uint64_t>());
        break;
    case TYPE_ARRAY_FLOAT:
        m_visitor.enterArrayFloat(*field, std::vector<float>());
        break;
    case TYPE_ARRAY_DOUBLE:
        m_visitor.enterArrayDouble(*field, std::vector<double>());
        break;
    case TYPE_ARRAY_STRING:
        m_visitor.enterArrayString(*field, std::vector<std::string>());
        break;
    case TYPE_ARRAY_BYTES:
        m_visitor.enterArrayBytes(*field, std::vector<Bytes>());
        break;
    case TYPE_ARRAY_STRUCT:
        m_visitor.enterArrayStruct(*field);
        m_visitor.exitArrayStruct(*field);
        break;
    case TYPE_ARRAY_ENUM:
        m_visitor.enterArrayEnum(*field, std::vector<std::int32_t>());
        break;
    default:
        assert(false);
        break;
    }
}


void ParserVariant::parseStruct(const MetaStruct& stru, const Variant& variant)
{
    ssize_t size = stru.getFieldsSize();
    for (ssize_t i = 0; i < size; ++i)
    {
        const MetaField* field = stru.getFieldByIndex(i);
        assert(field);
        const Variant* sub = variant.getVariant(field->name);
        if (sub)
        {
            processField(sub, field);
        }
        else
        {
            processEmptyField(field);
        }
    }
}


void ParserVariant::parseStruct(const MetaStruct& stru)
{
    ssize_t size = stru.getFieldsSize();
    for (ssize_t i = 0; i < size; ++i)
    {
        const MetaField* field = stru.getFieldByIndex(i);
        assert(field);
        processEmptyField(field);
    }
}

}   // namespace finalmq
