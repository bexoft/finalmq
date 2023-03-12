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

#include "finalmq/variant/Variant.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"


namespace finalmq {



Variant::Variant()
{
}

Variant::Variant(std::shared_ptr<IVariantValue> value)
    : m_value(value)
{

}


Variant::Variant(const Variant& rhs)
    : m_value((rhs.m_value) ? rhs.m_value->clone() : nullptr)
{
}

const Variant& Variant::operator =(const Variant& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    if (rhs.m_value)
    {
        m_value = rhs.m_value->clone();
    }
    else
    {
        m_value = nullptr;
    }
    return *this;
}

Variant::Variant(Variant&& rhs) noexcept
    : m_value(std::move(rhs.m_value))
{

}

Variant& Variant::operator =(Variant&& rhs) noexcept
{
    if (this == &rhs)
    {
        return *this;
    }
    m_value = std::move(rhs.m_value);
    return *this;
}


void Variant::accept(IVariantVisitor& visitor, ssize_t index, int level, ssize_t size, const std::string& name)
{

    if (m_value)
    {
        m_value->accept(visitor, *this, index, level, size, name);
    }
    else
    {
       visitor.enterLeaf(*this, VARTYPE_NONE, index, level, size, name);
    }
}



Variant* Variant::getVariant(const std::string& name)
{
    if (name.empty())
    {
        return this;
    }
    if (m_value == nullptr)
    {
        return nullptr;
    }

    return m_value->getVariant(name);
}


const Variant* Variant::getVariant(const std::string& name) const
{
    return const_cast<Variant*>(this)->getVariant(name);
}


bool Variant::operator ==(const Variant& rhs) const
{
    if (this == &rhs)
    {
        return true;
    }

    if (m_value == rhs.m_value)
    {
        return true;
    }

    if (m_value == nullptr || rhs.m_value == nullptr)
    {
        return false;
    }

    return *m_value == *rhs.m_value;
}



bool Variant::add(const std::string& name, const Variant& variant)
{
    if (m_value)
    {
        return m_value->add(name, variant);
    }
    return false;
}
bool Variant::add(const std::string& name, Variant&& variant)
{
    if (m_value)
    {
        return m_value->add(name, std::move(variant));
    }
    return false;
}

bool Variant::add(const Variant& variant)
{
    if (m_value)
    {
        return m_value->add(variant);
    }
    return false;
}
bool Variant::add(Variant&& variant)
{
    if (m_value)
    {
        return m_value->add(std::move(variant));
    }
    return false;
}

ssize_t Variant::size() const
{
    if (m_value)
    {
        return m_value->size();
    }
    return 0;
}


Variant& Variant::getOrCreate(const std::string& name)
{
    if (name.empty())
    {
        return *this;
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

    // check if number
    if (!partname.empty() && partname[0] >= '0' && partname[0] <= '9')
    {
        ssize_t index = atoll(partname.c_str());
        assert(index >= 0);
        ssize_t sizeNew = index + 1;
        assert(sizeNew > 0);
        if (!m_value || m_value->getType() != VARTYPE_LIST)
        {
            m_value = std::make_shared<VariantValueList>();
        }
        while (m_value->size() < sizeNew)
        {
            m_value->add(Variant());
        }
        Variant* varSub = m_value->getVariant(partname);
        assert(varSub != nullptr);
        return varSub->getOrCreate(restname);
    }
    else
    {
        // remove "", if available in partname
        if ((partname.size() >= 2) && (partname[0] == '\"') && (partname.back() == '\"'))
        {
            partname = partname.substr(1, partname.size() - 2);
        }
        if (!m_value || m_value->getType() != VARTYPE_STRUCT)
        {
            m_value = std::make_shared<VariantValueStruct>();
        }
        Variant* varSub = m_value->getVariant(partname);
        if (varSub == nullptr)
        {
            m_value->add(partname, Variant());
            varSub = m_value->getVariant(partname);
        }
        assert(varSub != nullptr);
        return varSub->getOrCreate(restname);
    }
}




}   // namespace finalmq
