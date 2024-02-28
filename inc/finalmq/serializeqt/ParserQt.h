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


#include <string>

namespace finalmq {



    class SYMBOLEXP ParserQt
    {
    public:
        enum class Mode
        {
            NONE,
            WRAPPED_BY_QVARIANTLIST,
            WRAPPED_BY_QVARIANT,
        };

        ParserQt(IParserVisitor& visitor, const char* ptr, ssize_t size, Mode mode = Mode::NONE);

        bool parseStruct(const std::string& typeName);

    private:
        bool parseStructIntern(const MetaStruct& stru, bool wrappedByQVariant);

        bool parse(std::int8_t& value);
        bool parse(std::uint8_t& value);
        bool parse(std::int16_t& value);
        bool parse(std::uint16_t& value);
        bool parse(std::int32_t& value);
        bool parse(std::uint32_t& value);
        bool parse(std::int64_t& value);
        bool parse(std::uint64_t& value);
        bool parse(float& value);
        bool parse(double& value);
        bool parse(std::string& str);
        bool parse(Bytes& value);
        bool parsePng(Bytes& value);

        template<class T>
        bool parse(std::vector<T>& value);

        bool parseArrayByte(const char*& buffer, ssize_t& size);
        bool parsePng(const char*& buffer, ssize_t& size);
        bool parseArrayBool(std::vector<bool>& value);
        bool parseArrayStruct(const MetaField& field);
        bool parseArrayPng(std::vector<Bytes>& value);

        bool parseQVariantHeader(const MetaField& field);
        void checkIndex(const MetaField& field, std::int64_t value, std::int64_t& index);
        bool getPngSize(std::uint32_t& size);

        const std::uint8_t* m_ptr = nullptr;
        ssize_t m_size = 0;
        IParserVisitor& m_visitor;
        const Mode m_mode = Mode::NONE;

        enum IndexStatus
        {
            INDEX_NOT_AVAILABLE = -1,
            INDEX_ABORTSTRUCT = -2
        };

        std::int64_t m_index = INDEX_NOT_AVAILABLE;
    };

}   // namespace finalmq
