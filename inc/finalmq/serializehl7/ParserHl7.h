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

#include <deque>
#include <string>

#include "finalmq/hl7/Hl7Parser.h"
#include "finalmq/metadata/MetaStruct.h"
#include "finalmq/serialize/IParserVisitor.h"

namespace finalmq
{
class SYMBOLEXP ParserHl7
{
public:
    ParserHl7(IParserVisitor& visitor, const char* ptr, ssize_t size = CHECK_ON_ZEROTERM);

    const char* parseStruct(const std::string& typeName);

private:
    ParserHl7(const ParserHl7&) = delete;
    ParserHl7(ParserHl7&&) = delete;
    const ParserHl7& operator=(const ParserHl7&) = delete;
    const ParserHl7& operator=(ParserHl7&&) = delete;

    int parseStruct(int levelSegment, const MetaStruct& stru, bool& isarray);
    bool matches(const std::string& segId, const MetaStruct& stru, ssize_t ixStart);
    bool matchesUp(const std::string& segId);

    const char* m_ptr = nullptr;
    ssize_t m_size = 0;
    IParserVisitor& m_visitor;
    Hl7Parser m_parser{};
    std::vector<std::pair<MetaStruct, ssize_t>> m_stackStruct{};
};

} // namespace finalmq
