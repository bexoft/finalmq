#pragma once

#include <string>
#include <memory>

class Variant;



struct IVariantVisitor
{
    virtual ~IVariantVisitor() {}
    virtual void enterLeaf(Variant& variant, int type, int index, int level, int size, const std::string& name) = 0;
    virtual void enterStruct(Variant& variant, int type, int index, int level, int size, const std::string& name) = 0;
    virtual void exitStruct(Variant& variant, int type, int index, int level, int size, const std::string& name) = 0;
    virtual void enterList(Variant& variant, int type, int index, int level, int size, const std::string& name) = 0;
    virtual void exitList(Variant& variant, int type, int index, int level, int size, const std::string& name) = 0;
};



struct IVariantValue
{
    virtual ~IVariantValue() {}
    virtual int getType() const = 0;
    virtual void* getData() = 0;
    virtual Variant* getVariant(const std::string& name) = 0;
    virtual std::shared_ptr<IVariantValue> clone() = 0;
    virtual bool add(const std::string& name, const Variant& variant) = 0;
    virtual bool add(const std::string& name, Variant&& variant) = 0;
    virtual bool add(const Variant& variant) = 0;
    virtual bool add(Variant&& variant) = 0;
    virtual int size() const = 0;
    virtual void visit(IVariantVisitor& visitor, Variant& variant, int index, int level, int size, const std::string& name) = 0;
};



template<class T>
class VariantValueTypeInfo
{
};
