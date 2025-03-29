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

#include "finalmq/jsonvariant/JsonToVariant.h"


namespace finalmq
{
JsonToVariant::JsonToVariant(Variant& variant)
    : m_variant(variant)
{
}

const char* JsonToVariant::parse(const char* str, ssize_t size)
{
    JsonParser parser(*this);
    const char* result = parser.parse(str, size);
    return result;
}



void JsonToVariant::syntaxError(const char* /*str*/, const char* /*message*/)
{
}

void JsonToVariant::enterNull()
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = Variant();
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, Variant());
    }
    m_key.clear();
}

void JsonToVariant::enterBool(bool value)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = value;
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, value);
    }
    m_key.clear();
}

void JsonToVariant::enterInt32(std::int32_t value)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = value;
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, value);
    }
    m_key.clear();
}

void JsonToVariant::enterUInt32(std::uint32_t value)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = value;
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, value);
    }
    m_key.clear();
}

void JsonToVariant::enterInt64(std::int64_t value)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = value;
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, value);
    }
    m_key.clear();
}

void JsonToVariant::enterUInt64(std::uint64_t value)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = value;
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, value);
    }
    m_key.clear();
}

void JsonToVariant::enterDouble(double value)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = value;
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, value);
    }
    m_key.clear();
}

void JsonToVariant::enterString(const char* value, ssize_t size)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = std::string(value, value + size);
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, std::string(value, value + size));
    }
    m_key.clear();
}

void JsonToVariant::enterString(std::string&& value)
{
    if (m_stackVariant.size() == 0)
    {
        m_variant = value;
    }
    else
    {
        Variant* variant = m_stackVariant.back();
        assert(variant);
        variant->add(m_key, value);
    }
    m_key.clear();
}

void JsonToVariant::enterArray()
{
    Variant* variant = nullptr;
    if (m_stackVariant.size() == 0)
    {
        m_variant = VariantList();
        variant = &m_variant;
    }
    else
    {
        variant = m_stackVariant.back();
        variant = variant->add(m_key, VariantList());
        assert(variant);
    }
    m_stackVariant.emplace_back(variant);
    m_key.clear();
}

void JsonToVariant::exitArray()
{
    assert(!m_stackVariant.empty());
    m_stackVariant.pop_back();
}

void JsonToVariant::enterObject()
{
    Variant* variant = nullptr;
    if (m_stackVariant.size() == 0)
    {
        m_variant = VariantStruct();
        variant = &m_variant;
    }
    else
    {
        variant = m_stackVariant.back();
        variant = variant->add(m_key, VariantStruct());
        assert(variant);
    }
    m_stackVariant.emplace_back(variant);
    m_key.clear();
}

void JsonToVariant::exitObject()
{
    assert(!m_stackVariant.empty());
    m_stackVariant.pop_back();
}

void JsonToVariant::enterKey(const char* key, ssize_t size)
{
    m_key = std::string(key, key + size);
}

void JsonToVariant::enterKey(std::string&& key)
{
    m_key = std::move(key);
}

void JsonToVariant::finished()
{

}



} // namespace finalmq
