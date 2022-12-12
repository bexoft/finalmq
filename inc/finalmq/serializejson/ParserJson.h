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

#include "finalmq/metadata/MetaStruct.h"
#include "finalmq/serialize/IParserVisitor.h"
#include "finalmq/json/JsonParser.h"


#include <string>
#include <deque>

namespace finalmq {

class SYMBOLEXP ParserJson : public IJsonParserVisitor
{
public:
    ParserJson(IParserVisitor& visitor, const char* ptr, ssize_t size = CHECK_ON_ZEROTERM);

    const char* parseStruct(const std::string& typeName);

private:
    // IJsonParserVisitor
    virtual void syntaxError(const char* str, const char* message) override;
    virtual void enterNull() override;
    virtual void enterBool(bool value) override;
    virtual void enterInt32(std::int32_t value) override;
    virtual void enterUInt32(std::uint32_t value) override;
    virtual void enterInt64(std::int64_t value) override;
    virtual void enterUInt64(std::uint64_t value) override;
    virtual void enterDouble(double value) override;
    virtual void enterString(const char* value, ssize_t size) override;
    virtual void enterString(std::string&& value) override;
    virtual void enterArray() override;
    virtual void exitArray() override;
    virtual void enterObject() override;
    virtual void exitObject() override;
    virtual void enterKey(const char* key, ssize_t size) override;
    virtual void enterKey(std::string&& key) override;
    virtual void finished() override;


    template<class T>
    void enterNumber(T value);

    template< class T>
    T convert(const char* value, ssize_t size);

    const char*         m_ptr = nullptr;
    ssize_t             m_size = 0;
    JsonParser          m_parser;
    IParserVisitor&     m_visitor;

    std::deque<const MetaField*>   m_stack;
    const MetaStruct*   m_structCurrent = nullptr;
    const MetaField*    m_fieldCurrent = nullptr;

    std::vector<bool>           m_arrayBool;
    std::vector<std::int32_t>   m_arrayInt32;
    std::vector<std::uint32_t>  m_arrayUInt32;
    std::vector<std::int64_t>   m_arrayInt64;
    std::vector<std::uint64_t>  m_arrayUInt64;
    std::vector<float>          m_arrayFloat;
    std::vector<double>         m_arrayDouble;
    std::vector<std::string>    m_arrayString;
    std::vector<Bytes>          m_arrayBytes;
};

}   // namespace finalmq
