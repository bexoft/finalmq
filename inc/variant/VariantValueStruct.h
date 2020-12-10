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
#include "metadata/MetaType.h"
#include "VariantValueConvert.h"

#include <deque>

namespace finalmq {


class Variant;

typedef std::deque<std::pair<std::string, Variant>> VariantStruct;

const static int VARTYPE_STRUCT = TYPE_STRUCT;
class VariantValueStruct : public IVariantValue
{
public:
    VariantValueStruct();
    VariantValueStruct(const VariantValueStruct& rhs);
    VariantValueStruct(VariantValueStruct&& rhs) noexcept;
    VariantValueStruct(const VariantStruct& value);
    VariantValueStruct(VariantStruct&& value);

private:
    virtual int getType() const override;
    virtual void* getData() override;
    virtual const void* getData() const override;
    virtual Variant* getVariant(const std::string& name) override;
    virtual const Variant* getVariant(const std::string& name) const override;
    virtual std::shared_ptr<IVariantValue> clone() const override;
    virtual bool operator ==(const IVariantValue& rhs) const override;
    virtual bool add(const std::string& name, const Variant& variant) override;
    virtual bool add(const std::string& name, Variant&& variant) override;
    virtual bool add(const Variant& variant) override;
    virtual bool add(Variant&& variant) override;
    virtual ssize_t size() const override;
    virtual void visit(IVariantVisitor& visitor, Variant& variant, ssize_t index, int level, ssize_t size, const std::string& name) override;


    VariantStruct::iterator find(const std::string& name);

    std::unique_ptr<VariantStruct>     m_value;
};



template <>
class MetaTypeInfo<VariantStruct>
{
public:
    static const int TypeId = MetaTypeId::TYPE_STRUCT;
};


template<>
class VariantValueTypeInfo<VariantStruct&>
{
public:
    typedef VariantValueStruct VariantValueType;
    const static int VARTYPE = VARTYPE_STRUCT;
    typedef Convert<VariantStruct> ConvertType;
};

template<>
class VariantValueTypeInfo<VariantStruct>
{
public:
    typedef VariantValueStruct VariantValueType;
    const static int VARTYPE = VARTYPE_STRUCT;
    typedef Convert<VariantStruct> ConvertType;
};

}   // namespace finalmq
