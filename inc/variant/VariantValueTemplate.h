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

#pragma once

#include "IVariantValue.h"
#include "Variant.h"
#include "metadata/MetaType.h"
#include "assert.h"

#include <functional>
#include <unordered_map>


namespace finalmq {


template<int VARTYPE>
class VariantValueTemplate : public IVariantValue
{
public:
    VariantValueTemplate()
        : m_value()
    {
    }

    VariantValueTemplate(const typename MetaTypeIdInfo<VARTYPE>::Type& value)
        : m_value(value)
    {
    }

    VariantValueTemplate(typename MetaTypeIdInfo<VARTYPE>::Type&& value)
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

    virtual const void* getData() const override
    {
        return &m_value;
    }

    virtual Variant* getVariant(const std::string& name) override
    {
        return nullptr;
    }

    virtual const Variant* getVariant(const std::string& name) const override
    {
        return nullptr;
    }

    virtual std::shared_ptr<IVariantValue> clone() const override
    {
        return std::make_shared<VariantValueTemplate>(*this);
    }

    virtual bool operator ==(const IVariantValue& rhs) const override
    {
        if (this == &rhs)
        {
            return true;
        }

        if (getType() != rhs.getType())
        {
            return false;
        }

        const typename MetaTypeIdInfo<VARTYPE>::Type* rhsData = static_cast<const typename MetaTypeIdInfo<VARTYPE>::Type*>(rhs.getData());
        assert(rhsData);

        return m_value == *rhsData;
    }

    virtual bool add(const std::string& name, const Variant& /*variant*/) override
    {
        return false;
    }
    virtual bool add(const std::string& name, Variant&& /*variant*/) override
    {
        return false;
    }
    virtual bool add(const Variant& /*variant*/) override
    {
        return false;
    }
    virtual bool add(Variant&& /*variant*/) override
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

}   // namespace finalmq
