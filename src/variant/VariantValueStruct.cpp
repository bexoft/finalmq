#include "variant/VariantValueStruct.h"
#include "variant/Variant.h"
#include <utility>
#include <assert.h>


VariantValueStruct::VariantValueStruct()
{
}

VariantValueStruct::VariantValueStruct(const VariantValueStruct& rhs)
    : m_value(std::make_unique<Struct>(*rhs.m_value))
{
}

VariantValueStruct::VariantValueStruct(VariantValueStruct&& rhs)
    : m_value(std::move(rhs.m_value))
{
}

VariantValueStruct::VariantValueStruct(const Struct& value)
    : m_value(std::make_unique<Struct>(value))
{
}

VariantValueStruct::VariantValueStruct(Struct&& value)
    : m_value(std::make_unique<Struct>(std::move(value)))
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



Struct::iterator VariantValueStruct::find(const std::string& name)
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



std::shared_ptr<IVariantValue> VariantValueStruct::clone()
{
    return std::make_shared<VariantValueStruct>(*this);
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

bool VariantValueStruct::add(const Variant& variant)
{
    return false;
}

bool VariantValueStruct::add(Variant&& variant)
{
    return false;
}

int VariantValueStruct::size() const
{
    return m_value->size();
}

void VariantValueStruct::visit(IVariantVisitor& visitor, Variant& variant, int index, int level, int size, const std::string& name)
{
    visitor.enterStruct(variant, VARTYPE_STRUCT, index, level, size, name);
    level++;
    int i = 0;
    int subsize = m_value->size();
    for (auto it = m_value->begin(); it != m_value->end(); ++it)  //std::list<std::string, Variant>::iterator????
    {
        Variant& subVariant = it->second;
        subVariant.visit(visitor, i, level, subsize, it->first);
        i++;
    }
    --level;
    visitor.exitStruct(variant, VARTYPE_STRUCT, index, level, size, name);
}
