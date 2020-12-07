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

#include "json/JsonParser.h"
#include "helpers/IZeroCopyBuffer.h"

namespace finalmq {

class JsonBuilder : public IJsonParserVisitor
{
public:
    JsonBuilder(IZeroCopyBuffer& buffer, int maxBlockSize = 1024);
    ~JsonBuilder();

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

    void reserveSpace(int space);
    void resizeBuffer();
    void correctComma();
    void escapeString(const char* str, ssize_t size);

    IZeroCopyBuffer&        m_zeroCopybuffer;
    int                     m_maxBlockSize = 1024;
    char*                   m_bufferStart = nullptr;
    char*                   m_buffer = nullptr;
    char*                   m_bufferEnd = nullptr;
};

}   // namespace finalmq
