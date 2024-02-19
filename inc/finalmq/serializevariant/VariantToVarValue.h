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

#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/variant/Variant.h"
#include "finalmq/serialize/IParserVisitor.h"
#include "finalmq/serializevariant/VarValueToVariant.h"


namespace finalmq {


class SYMBOLEXP VariantToVarValue : private IVariantVisitor
{
public:
    VariantToVarValue(const Variant& variant, IParserVisitor& visitor);

    void convert();

private:
    // IVariantVisitor
    virtual void enterLeaf(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) override;
    virtual void enterStruct(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) override;
    virtual void exitStruct(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) override;
    virtual void enterList(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) override;
    virtual void exitList(Variant& variant, int type, ssize_t index, int level, ssize_t size, const std::string& name) override;


    Variant&                        m_variant;
    IParserVisitor&                 m_visitor;
    std::deque<const MetaField*>    m_fieldStack{};
    static const MetaStruct*        m_structVarValue;
};

}   // namespace finalmq
