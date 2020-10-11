#pragma once

#include "IVariantValue.h"
#include <deque>

class Variant;

// todo: change type to std::vector<std::pair<std::string, Variant>>
typedef std::deque<Variant> List;

const static int VARTYPE_LIST = 2;
class VariantValueList : public IVariantValue
{
public:
    VariantValueList();
    VariantValueList(const VariantValueList& rhs);
    VariantValueList(VariantValueList&& rhs);
    VariantValueList(const List& value);
    VariantValueList(List&& value);

private:
    virtual int getType() const override;
    virtual void* getData() override;
    virtual Variant* getVariant(const std::string& name) override;
    virtual std::shared_ptr<IVariantValue> clone() override;
    virtual bool add(const std::string& name, const Variant& variant) override;
    virtual bool add(const std::string& name, Variant&& variant) override;
    virtual bool add(const Variant& variant) override;
    virtual bool add(Variant&& variant) override;
    virtual int size() const override;
    virtual void visit(IVariantVisitor& visitor, Variant& variant, int index, int level, int size, const std::string& name) override;


    List::iterator find(const std::string& name);

    std::unique_ptr<List>     m_value;
};


template<>
class VariantValueTypeInfo<List&>
{
public:
    typedef VariantValueList Type;
    const static int VARTYPE = VARTYPE_LIST;
};

template<>
class VariantValueTypeInfo<List>
{
public:
    typedef VariantValueList Type;
    const static int VARTYPE = VARTYPE_LIST;
};

