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

#include "finalmq/serializevariant/VarValueToVariant.h"

#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValues.h"

#include "finalmq/metadataserialize/variant.fmq.h"
#include "finalmq/serializestruct/SerializerStruct.h"



namespace finalmq {


VarValueToVariant::VarValueToVariant(Variant& variant)
    : m_variant(variant)
{
    m_varValue = std::make_shared<variant::VarValue>();
    m_serializerStruct = std::make_shared<SerializerStruct>(*m_varValue);
}


IParserVisitor& VarValueToVariant::getVisitor()
{
    assert(m_serializerStruct);
    return *m_serializerStruct;
}


void VarValueToVariant::setExitNotification(std::function<void()> func)
{
    assert(m_serializerStruct);
    m_serializerStruct->setExitNotification(std::move(func));
}

void VarValueToVariant::convert()
{
    assert(m_varValue);
    processVarValue(*m_varValue, m_variant);
}


void VarValueToVariant::processVarValue(const variant::VarValue& varValue, Variant& variant)
{
    switch (varValue.type)
    {
    case variant::VarTypeId::T_NONE:
        break;
    case variant::VarTypeId::T_BOOL:
        variant = varValue.valbool;
        break;
    case variant::VarTypeId::T_INT8:
        variant = varValue.valint8;
        break;
    case variant::VarTypeId::T_UINT8:
        variant = varValue.valuint8;
        break;
    case variant::VarTypeId::T_INT16:
        variant = varValue.valint16;
        break;
    case variant::VarTypeId::T_UINT16:
        variant = varValue.valuint16;
        break;
    case variant::VarTypeId::T_INT32:
        variant = varValue.valint32;
        break;
    case variant::VarTypeId::T_UINT32:
        variant = varValue.valuint32;
        break;
    case variant::VarTypeId::T_INT64:
        variant = varValue.valint64;
        break;
    case variant::VarTypeId::T_UINT64:
        variant = varValue.valuint64;
        break;
    case variant::VarTypeId::T_FLOAT:
        variant = varValue.valfloat;
        break;
    case variant::VarTypeId::T_DOUBLE:
        variant = varValue.valdouble;
        break;
    case variant::VarTypeId::T_STRING:
        variant = std::move(varValue.valstring);
        break;
    case variant::VarTypeId::T_BYTES:
        variant = std::move(varValue.valbytes);
        break;
    case variant::VarTypeId::T_STRUCT:
        {
            variant = VariantStruct();
            VariantStruct* variantStruct = variant;
            assert(variantStruct);
            for (size_t i = 0; i < varValue.vallist.size(); ++i)
            {
                const variant::VarValue& varValueElement = varValue.vallist[i];
                variantStruct->emplace_back(varValueElement.name, Variant());
                processVarValue(varValueElement, variantStruct->back().second);
            }
        }
        break;

    case variant::VarTypeId::T_ARRAY_BOOL:
        variant = std::move(varValue.valarrbool);
        break;
    case variant::VarTypeId::T_ARRAY_INT8:
        variant = std::move(varValue.valarrint8);
        break;
    case variant::VarTypeId::T_ARRAY_INT16:
        variant = std::move(varValue.valarrint16);
        break;
    case variant::VarTypeId::T_ARRAY_UINT16:
        variant = std::move(varValue.valarruint16);
        break;
    case variant::VarTypeId::T_ARRAY_INT32:
        variant = std::move(varValue.valarrint32);
        break;
    case variant::VarTypeId::T_ARRAY_UINT32:
        variant = std::move(varValue.valarruint32);
        break;
    case variant::VarTypeId::T_ARRAY_INT64:
        variant = std::move(varValue.valarrint64);
        break;
    case variant::VarTypeId::T_ARRAY_UINT64:
        variant = std::move(varValue.valarruint64);
        break;
    case variant::VarTypeId::T_ARRAY_FLOAT:
        variant = std::move(varValue.valarrfloat);
        break;
    case variant::VarTypeId::T_ARRAY_DOUBLE:
        variant = std::move(varValue.valarrdouble);
        break;
    case variant::VarTypeId::T_ARRAY_STRING:
        variant = std::move(varValue.valarrstring);
        break;
    case variant::VarTypeId::T_ARRAY_BYTES:
        variant = std::move(varValue.valarrbytes);
        break;
    case variant::VarTypeId::T_LIST:
        {
            variant = VariantList();
            VariantList* variantList = variant;
            assert(variantList);
            for (size_t i = 0; i < varValue.vallist.size(); ++i)
            {
                const variant::VarValue& varValueElement = varValue.vallist[i];
                variantList->emplace_back(Variant());
                processVarValue(varValueElement, variantList->back());
            }
        }
        break;
    default:
        break;
    }
}



}   // namespace finalmq
