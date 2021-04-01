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

#include "finalmq/helpers/BexDefines.h"

#include <string>
#include <memory>


namespace finalmq {


class Variant;


struct IVariantVisitor
{
    virtual ~IVariantVisitor() {}
    virtual void enterLeaf(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) = 0;
    virtual void enterStruct(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) = 0;
    virtual void exitStruct(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) = 0;
    virtual void enterList(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) = 0;
    virtual void exitList(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) = 0;
};



struct IVariantValue
{
    virtual ~IVariantValue() {}
    virtual int getType() const = 0;
    virtual void* getData() = 0;
    virtual const void* getData() const = 0;
    virtual Variant* getVariant(const std::string& name) = 0;
    virtual const Variant* getVariant(const std::string& name) const = 0;
    virtual std::shared_ptr<IVariantValue> clone() const = 0;
    virtual bool operator ==(const IVariantValue& rhs) const = 0;
    virtual bool add(const std::string& name, const Variant& variant) = 0;
    virtual bool add(const std::string& name, Variant&& variant) = 0;
    virtual bool add(const Variant& variant) = 0;
    virtual bool add(Variant&& variant) = 0;
    virtual ssize_t size() const = 0;
    virtual void accept(IVariantVisitor& visitor, Variant& variant, ssize_t index, int level, ssize_t size, const std::string& name) = 0;
};



template<class T>
class VariantValueTypeInfo
{
};

}   // namespace finalmq
