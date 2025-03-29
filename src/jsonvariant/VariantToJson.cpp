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


#include "finalmq/jsonvariant/VariantToJson.h"


namespace finalmq {


VariantToJson::VariantToJson(IZeroCopyBuffer& buffer, int maxBlockSize)
    : m_jsonBuilder(buffer, maxBlockSize)
{
}

VariantToJson::~VariantToJson()
{
}

void VariantToJson::parse(const Variant& variant)
{
    const_cast<Variant&>(variant).accept(*this);
    m_jsonBuilder.finished();
}


void VariantToJson::enterLeaf(Variant& variant, int type, ssize_t /*index*/, int /*level*/, ssize_t /*size*/, const std::string& name, bool parentIsStruct)
{
    if (parentIsStruct)
    {
        m_jsonBuilder.enterKey(name.c_str(), name.size());
    }

    switch (type)
    {
    case TYPE_NONE:
        m_jsonBuilder.enterNull();
        break;
    case TYPE_BOOL:
        m_jsonBuilder.enterBool(variant);
        break;
    case TYPE_INT8:
        m_jsonBuilder.enterInt32(variant);
        break;
    case TYPE_UINT8:
        m_jsonBuilder.enterUInt32(variant);
        break;
    case TYPE_INT16:
        m_jsonBuilder.enterInt32(variant);
        break;
    case TYPE_UINT16:
        m_jsonBuilder.enterUInt32(variant);
        break;
    case TYPE_INT32:
        m_jsonBuilder.enterInt32(variant);
        break;
    case TYPE_UINT32:
        m_jsonBuilder.enterUInt32(variant);
        break;
    case TYPE_INT64:
        m_jsonBuilder.enterInt64(variant);
        break;
    case TYPE_UINT64:
        m_jsonBuilder.enterUInt64(variant);
        break;
    case TYPE_FLOAT:
        m_jsonBuilder.enterDouble(variant);
        break;
    case TYPE_DOUBLE:
        m_jsonBuilder.enterDouble(variant);
        break;
    case TYPE_STRING:
        m_jsonBuilder.enterString(variant);
        break;
    case TYPE_BYTES:
        m_jsonBuilder.enterString(variant);
        break;
    case TYPE_ENUM:
        m_jsonBuilder.enterString(variant);
        break;
    default:
        break;
    }
}

void VariantToJson::enterStruct(Variant& /*variant*/, int /*type*/, ssize_t /*index*/, int /*level*/, ssize_t /*size*/, const std::string& name, bool parentIsStruct)
{
    if (parentIsStruct)
    {
        m_jsonBuilder.enterKey(name.c_str(), name.size());
    }
    m_jsonBuilder.enterObject();
}

void VariantToJson::exitStruct(Variant& /*variant*/, int /*type*/, ssize_t /*index*/, int /*level*/, ssize_t /*size*/, const std::string& /*name*/, bool /*parentIsStruct*/)
{
    m_jsonBuilder.exitObject();
}

void VariantToJson::enterList(Variant& /*variant*/, int /*type*/, ssize_t /*index*/, int /*level*/, ssize_t /*size*/, const std::string& name, bool parentIsStruct)
{
    if (parentIsStruct)
    {
        m_jsonBuilder.enterKey(name.c_str(), name.size());
    }
    m_jsonBuilder.enterArray();
}

void VariantToJson::exitList(Variant& /*variant*/, int /*type*/, ssize_t /*index*/, int /*level*/, ssize_t /*size*/, const std::string& /*name*/, bool /*parentIsStruct*/)
{
    m_jsonBuilder.exitArray();
}


}   // namespace finalmq
