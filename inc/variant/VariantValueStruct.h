#pragma once

#include "IVariantValue.h"
#include <list>

class Variant;

// todo: change type to std::vector<std::pair<std::string, Variant>>
typedef std::list<std::pair<std::string, Variant>> Struct;

const static int VARTYPE_STRUCT = 1;
class VariantValueStruct : public IVariantValue
{
public:
    VariantValueStruct();
    VariantValueStruct(const VariantValueStruct& rhs);
    VariantValueStruct(VariantValueStruct&& rhs);
    VariantValueStruct(const Struct& value);
    VariantValueStruct(Struct&& value);

private:
    virtual int getType() const override;
    virtual void* getData() override;
    //virtual std::shared_ptr<IVariantValue> getValue(const std::string& name) override;
    virtual Variant* getVariant(const std::string& name) override;
    virtual std::shared_ptr<IVariantValue> clone() override;
    virtual bool add(const std::string& name, const Variant& variant) override;
    virtual bool add(const std::string& name, Variant&& variant) override;
    virtual bool add(const Variant& variant) override;
    virtual bool add(Variant&& variant) override;
    virtual int size() const override;
    virtual void visit(IVariantVisitor& visitor, Variant& variant, int index, int level, int size, const std::string& name) override;


    Struct::iterator find(const std::string& name);

    std::unique_ptr<Struct>     m_value;
};


template<>
class VariantValueTypeInfo<Struct&>
{
public:
    typedef VariantValueStruct Type;
    const static int VARTYPE = VARTYPE_STRUCT;
};

template<>
class VariantValueTypeInfo<Struct>
{
public:
    typedef VariantValueStruct Type;
    const static int VARTYPE = VARTYPE_STRUCT;
};

