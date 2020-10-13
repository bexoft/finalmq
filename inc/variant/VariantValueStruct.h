#pragma once

#include "IVariantValue.h"
#include "metadata/MetaType.h"
#include "VariantValueConvert.h"

#include <deque>

class Variant;

typedef std::deque<std::pair<std::string, Variant>> VariantStruct;

const static int VARTYPE_STRUCT = TYPE_STRUCT;
class VariantValueStruct : public IVariantValue
{
public:
    VariantValueStruct();
    VariantValueStruct(const VariantValueStruct& rhs);
    VariantValueStruct(VariantValueStruct&& rhs);
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
    virtual int size() const override;
    virtual void visit(IVariantVisitor& visitor, Variant& variant, int index, int level, int size, const std::string& name) override;


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

