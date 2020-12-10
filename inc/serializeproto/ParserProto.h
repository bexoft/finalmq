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


#include <string>

namespace finalmq {





class SYMBOLEXP ParserProto
{
public:
    ParserProto(IParserVisitor& visitor, const char* ptr, ssize_t size);

    bool parseStruct(const std::string& typeName);

private:
    enum WireType
    {
        WIRETYPE_VARINT = 0,
        WIRETYPE_FIXED64 = 1,
        WIRETYPE_LENGTH_DELIMITED = 2,
        WIRETYPE_START_GROUP = 3,
        WIRETYPE_END_GROUP = 4,
        WIRETYPE_FIXED32 = 5,
    };

    bool parseString(const char*& buffer, ssize_t& size);
    void parseStructWire(const MetaField& field);
    bool parseStructIntern(const MetaStruct& stru);
    void parseArrayStruct(const MetaField& field);

    template<class T>
    bool parseArrayString(std::vector<T>& array);

    template<class T, int WIRETYPE>
    bool parseFixedValue(T& value);

    template<class T>
    bool parseVarint(T& value);

    template<class T, class D>
    bool parseZigZag(T& value);

    template<class T, int WIRETYPE>
    bool parseArrayFixed(std::vector<T>& array);

    template<class T, bool ZIGZAG = false>
    bool parseArrayVarint(std::vector<T>& array);

    std::uint64_t parseVarint();

    template<class T>
    T parseFixed();

    inline std::int32_t zigzag(std::uint32_t value);
    inline std::int64_t zigzag(std::uint64_t value);

    void skip(WireType wireType);

    const char*         m_ptr = nullptr;
    ssize_t             m_size = 0;
    IParserVisitor&     m_visitor;

    std::uint32_t       m_tag = 0;
};

}   // namespace finalmq
