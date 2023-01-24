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

#include "finalmq/helpers/IZeroCopyBuffer.h"
#include <string>

namespace finalmq {

struct IHl7BuilderVisitor
{
    virtual ~IHl7BuilderVisitor() {}
    virtual void syntaxError(const char* str, const char* message) = 0;
    virtual void enterStruct() = 0;
    virtual void exitStruct() = 0;
    virtual void enterArray() = 0;
    virtual void exitArray() = 0;
    virtual void enterNull() = 0;
    virtual void enterEmpty() = 0;
    virtual void enterInt64(std::int64_t value) = 0;
    virtual void enterUInt64(std::uint64_t value) = 0;
    virtual void enterDouble(double value) = 0;
    virtual void enterString(const char* value, ssize_t size) = 0;
    virtual void enterString(std::string&& value) = 0;
    virtual void finished() = 0;
};



class SYMBOLEXP Hl7Builder : public IHl7BuilderVisitor
{
public:
    Hl7Builder(IZeroCopyBuffer& buffer, int maxBlockSize = 512, const std::string& delimiters = "|^~\\&");
    ~Hl7Builder();

private:
    // IHl7BuilderVisitor
    virtual void syntaxError(const char* str, const char* message) override;
    virtual void enterStruct() override;
    virtual void exitStruct() override;
    virtual void enterArray() override;
    virtual void exitArray() override;
    virtual void enterNull() override;
    virtual void enterEmpty() override;
    virtual void enterInt64(std::int64_t value) override;
    virtual void enterUInt64(std::uint64_t value) override;
    virtual void enterDouble(double value) override;
    virtual void enterString(const char* value, ssize_t size) override;
    virtual void enterString(std::string&& value) override;
    virtual void finished() override;

    void reserveSpace(ssize_t space);
    void resizeBuffer();
    void correctDelimiters();
    void correctOneDelimiter();
    void escapeString(const char* str, ssize_t size);

    IZeroCopyBuffer&        m_zeroCopybuffer;
    ssize_t                 m_maxBlockSize = 512;
    char*                   m_bufferStart = nullptr;
    char*                   m_buffer = nullptr;
    char*                   m_bufferEnd = nullptr;


    static const int LAYER_MAX = 4;

    int                     m_waitForDeleimiterField = 0;
    int                     m_level = 0;
    bool                    m_array = false;

    std::string             m_delimitersForField;
    char                    m_delimiterField[LAYER_MAX];
    char                    m_delimiterRepeat;
    char                    m_escape;
    char                    m_delimiterCurrent = '\0';
};

}   // namespace finalmq
