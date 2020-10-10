#pragma once

#include "IVariantValue.h"
#include "Variant.h"
#include "metadata/MetaType.h"
#include "assert.h"

#include <functional>
#include <unordered_map>

template<int VARTYPE>
class VariantValuePrimitive : public IVariantValue
{
public:
    VariantValuePrimitive()
        : m_value()
    {
    }

    VariantValuePrimitive(const typename MetaTypeIdInfo<VARTYPE>::Type& value)
        : m_value(value)
    {
    }

    VariantValuePrimitive(typename MetaTypeIdInfo<VARTYPE>::Type&& value)
        : m_value(std::move(value))
    {
    }

private:
    virtual int getType() const override
    {
        return VARTYPE;
    }

    virtual void* getData() override
    {
        return &m_value;
    }

    void* getData(const std::string& name)
    {
        if (name.empty())
        {
            return &m_value;
        }
        return nullptr;
    }

//    virtual std::shared_ptr<IVariantValue> getValue(const std::string& name) override
//    {
//        assert(!(name.empty()));
//        return nullptr;
//    }

    virtual Variant* getVariant(const std::string& name, bool& found, std::string& nameRemaining) override
    {
        found = false;
        nameRemaining = name;
        return nullptr;
    }

    virtual std::shared_ptr<IVariantValue> clone() override
    {
        return std::make_shared<VariantValuePrimitive>(*this);
    }

    virtual bool add(const std::string& name, const Variant& variant) override
    {
        return false;
    }
    virtual bool add(const std::string& name, Variant&& variant) override
    {
        return false;
    }
    virtual bool add(const Variant& variant) override
    {
        return false;
    }
    virtual bool add(Variant&& variant) override
    {
        return false;
    }

    virtual int size() const override
    {
        return 1;
    }
    virtual void visit(IVariantVisitor& visitor, Variant& variant, int index, int level, int size, const std::string& name)
    {
        visitor.enterLeaf(variant, VARTYPE, index, level, size, name);
    }

    typename MetaTypeIdInfo<VARTYPE>::Type m_value;
};


template<class T>
class Convert
{
public:
    typedef std::function<T(const Variant& variant)> FuncConvert;

    static void registerConversion(int varType, FuncConvert funcConvert)
    {
        std::unordered_map<int, FuncConvert>& registry = getRegistry();
        registry[varType] = funcConvert;
    }
    static T convert(const Variant& variant)
    {
        if (MetaTypeInfo<T>::TypeId == variant.getType())
        {
            const T* data = variant;
            assert(data);
            return *data;
        }
        const std::unordered_map<int, FuncConvert>& registry = getRegistry();
        auto it = registry.find(variant.getType());
        if (it != registry.end())
        {
            auto& funcConvert = it->second;
            if (funcConvert)
            {
                return funcConvert(variant);
            }
        }
        return T();
    }

private:
    static std::unordered_map<int, FuncConvert>& getRegistry()
    {
        static std::unordered_map<int, FuncConvert> conversions;
        return conversions;
    }
};


