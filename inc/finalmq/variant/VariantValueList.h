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
#include "finalmq/metadata/MetaType.h"
#include "VariantValueConvert.h"

#include <deque>


namespace finalmq {


class Variant;

typedef std::deque<Variant> VariantList;

const static int VARTYPE_LIST = TYPE_ARRAY_STRUCT;
class SYMBOLEXP VariantValueList : public IVariantValue
{
public:
    VariantValueList();
    VariantValueList(const VariantValueList& rhs);
    VariantValueList(VariantValueList&& rhs) noexcept;
    VariantValueList(const VariantList& value);
    VariantValueList(VariantList&& value);

private:
    virtual int getType() const override;
    virtual void* getData() override;
    virtual const void* getData() const override;
    virtual Variant* getVariant(const std::string& name) override;
    virtual const Variant* getVariant(const std::string& name) const override;
    virtual std::shared_ptr<IVariantValue> clone() const override;
    virtual bool operator ==(const IVariantValue& rhs) const override;
    virtual Variant* add(const std::string& name, const Variant& variant) override;
    virtual Variant* add(const std::string& name, Variant&& variant) override;
    virtual Variant* add(const Variant& variant) override;
    virtual Variant* add(Variant&& variant) override;
    virtual ssize_t size() const override;
    virtual void accept(IVariantVisitor& visitor, Variant& variant, ssize_t index, int level, ssize_t size, const std::string& name, bool parentIsStruct) override;


    VariantList::iterator find(const std::string& name);

    std::unique_ptr<VariantList>     m_value;
};



template <>
class MetaTypeInfo<VariantList>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_STRUCT;
};


template<>
class VariantValueTypeInfo<VariantList&>
{
public:
    typedef VariantValueList VariantValueType;
    const static int VARTYPE = VARTYPE_LIST;
    typedef Convert<VariantList> ConvertType;
};

template<>
class VariantValueTypeInfo<VariantList>
{
public:
    typedef VariantValueList VariantValueType;
    const static int VARTYPE = VARTYPE_LIST;
    typedef Convert<VariantList> ConvertType;
};

}   // namespace finalmq
