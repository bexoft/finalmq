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

#include <cstdint>
#include <string>
#include <vector>

namespace finalmq {

static const ssize_t CHECK_ON_ZEROTERM = -1;
    
struct IHl7Parser
{
    virtual ~IHl7Parser() {}
    virtual bool startParse(const char* str, ssize_t size = CHECK_ON_ZEROTERM) = 0;
    virtual int parseToken(int level, std::string& token, bool& isarray) = 0;
    virtual int parseTokenArray(int level, std::vector<std::string>& array) = 0;
    virtual int parseTillEndOfStruct(int level) = 0;
    virtual void getSegmentId(std::string& token) const = 0;
    virtual int isNextFieldFilled(int level, bool& filled) = 0;
    virtual const char* getCurrentPosition() const = 0;
};


class SYMBOLEXP Hl7Parser : public IHl7Parser
{
public:
    Hl7Parser();

    virtual bool startParse(const char* str, ssize_t size = CHECK_ON_ZEROTERM) override;
    virtual int parseToken(int level, std::string& token, bool& isarray) override;
    virtual int parseTokenArray(int level, std::vector<std::string>& array) override;
    virtual int parseTillEndOfStruct(int level) override;
    virtual void getSegmentId(std::string& token) const override;
    virtual int isNextFieldFilled(int level, bool& filled) override;
    virtual const char* getCurrentPosition() const override;

private:
    inline char getChar(const char* str) const;
    void skipControlCharacters();
    int isDelimiter(char c) const;
    std::string deEscape(const char* start, const char* end) const;

    const char* m_end = nullptr;
    const char* m_str = nullptr;

    static const int LAYER_MAX = 4;

    char                    m_delimiterField[LAYER_MAX] = {0, 0, 0, 0};
    char                    m_delimiterRepeat = 0;
    char                    m_escape = 0;

    int                     m_waitForDeleimiterField = 0;
};

}   // namespace finalmq
