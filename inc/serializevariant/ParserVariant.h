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

#include "metadata/MetaStruct.h"
#include "serialize/IParserVisitor.h"
#include "variant/Variant.h"


#include <string>
#include <deque>


namespace finalmq {


class SYMBOLEXP ParserVariant
{
public:
    ParserVariant(IParserVisitor& visitor, const Variant& variant);

    bool parseStruct(const std::string& typeName);

private:
    void parseStruct(const MetaStruct& stru, const Variant& variant);
    void parseStruct(const MetaStruct& stru);
    void processField(const Variant* sub, const MetaField* field);
    void processEmptyField(const MetaField* field);

    IParserVisitor&     m_visitor;
    const Variant&      m_root;
};

}   // namespace finalmq
