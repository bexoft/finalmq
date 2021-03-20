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

#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/Variant.h"
#include <utility>
#include <assert.h>


namespace finalmq {



VariantValueStruct::VariantValueStruct()
{
}

VariantValueStruct::VariantValueStruct(const VariantValueStruct& rhs)
    : m_value(std::make_unique<VariantStruct>(*rhs.m_value))
{
}

VariantValueStruct::VariantValueStruct(VariantValueStruct&& rhs) noexcept
    : m_value(std::move(rhs.m_value))
{
}

VariantValueStruct::VariantValueStruct(const VariantStruct& value)
    : m_value(std::make_unique<VariantStruct>(value))
{
}

VariantValueStruct::VariantValueStruct(VariantStruct&& value)
    : m_value(std::make_unique<VariantStruct>(std::move(value)))
{
}

int VariantValueStruct::getType() const
{
    return VARTYPE_STRUCT;
}

void* VariantValueStruct::getData()
{
    return m_value.get();
}

const void* VariantValueStruct::getData() const
{
    return const_cast<VariantValueStruct*>(this)->getData();
}


VariantStruct::iterator VariantValueStruct::find(const std::string& name)
{
    for (auto it = m_value->begin(); it != m_value->end(); ++it)
    {
        if (it->first == name)
        {
            return it;
        }
    }
    return m_value->end();
}



Variant* VariantValueStruct::getVariant(const std::string& name)
{
    if (name.empty())
    {
        return nullptr;
    }

    std::string partname;
    std::string restname;

    //sperate first key ansd second key
    size_t cntp = name.find('.');
    if (cntp != std::string::npos)
    {
        partname = name.substr(0, cntp);
        cntp++;
        restname = name.substr(cntp);
    }
    else
    {
        partname = name;
    }

    // check if next key is in map (if not -> nullptr)
    auto it = find(partname);   // auto: std::unordered_map<std::string, Variant>::iterator
    if (it == m_value->end())
    {
        return nullptr;
    }

    // m_value[name].getValue( with remaining name )
    return it->second.getVariant(restname);
}


const Variant* VariantValueStruct::getVariant(const std::string& name) const
{
    return const_cast<VariantValueStruct*>(this)->getVariant(name);
}


std::shared_ptr<IVariantValue> VariantValueStruct::clone() const
{
    return std::make_shared<VariantValueStruct>(*this);
}


bool VariantValueStruct::operator ==(const IVariantValue& rhs) const
{
    if (this == &rhs)
    {
        return true;
    }

    if (getType() != rhs.getType())
    {
        return false;
    }

    const VariantStruct* rhsData = static_cast<const VariantStruct*>(rhs.getData());
    assert(rhsData);

    assert(m_value);

    return *m_value == *rhsData;
}



bool VariantValueStruct::add(const std::string& name, const Variant& variant)
{
    auto it = find(name);
    if (it == m_value->end())
    {
        m_value->push_back(std::make_pair(name, variant));
        return true;
    }
    return false;
}

bool VariantValueStruct::add(const std::string& name, Variant&& variant)
{
    auto it = find(name);
    if (it == m_value->end())
    {
        m_value->push_back(std::make_pair(name, std::move(variant)));
        return true;
    }
    return false;
}

bool VariantValueStruct::add(const Variant& /*variant*/)
{
    return false;
}

bool VariantValueStruct::add(Variant&& /*variant*/)
{
    return false;
}

ssize_t VariantValueStruct::size() const
{
    return m_value->size();
}

void VariantValueStruct::accept(IVariantVisitor& visitor, Variant& variant, ssize_t index, int level, ssize_t size, const std::string& name)
{
    visitor.enterStruct(variant, VARTYPE_STRUCT, index, level, size, name);
    level++;
    ssize_t i = 0;
    ssize_t subsize = m_value->size();
    for (auto it = m_value->begin(); it != m_value->end(); ++it)  //std::list<std::string, Variant>::iterator????
    {
        Variant& subVariant = it->second;
        subVariant.accept(visitor, i, level, subsize, it->first);
        i++;
    }
    --level;
    visitor.exitStruct(variant, VARTYPE_STRUCT, index, level, size, name);
}

}   // namespace finalmq
