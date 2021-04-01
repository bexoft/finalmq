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
#include "finalmq/variant/Variant.h"
#include "finalmq/serialize/IParserVisitor.h"


#include <deque>
#include <functional>


namespace finalmq {


namespace variant {
    class VarValue;
}
class SerializerStruct;

class SYMBOLEXP VarValueToVariant
{
public:
    VarValueToVariant(Variant& variant);

    IParserVisitor& getVisitor();
    void setExitNotification(std::function<void()> funcExit);
    void convert();

private:
    VarValueToVariant(const VarValueToVariant&) = delete;
    const VarValueToVariant& operator =(const VarValueToVariant&) = delete;
    VarValueToVariant(const VarValueToVariant&&) = delete;
    const VarValueToVariant& operator =(const VarValueToVariant&&) = delete;

    void processVarValue(const variant::VarValue& varValue, Variant& variant);

    Variant&                            m_variant;
    std::shared_ptr<SerializerStruct>   m_serializerStruct;
    std::shared_ptr<variant::VarValue>  m_varValue;
};

}   // namespace finalmq
