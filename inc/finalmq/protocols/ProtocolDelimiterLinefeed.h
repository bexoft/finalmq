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

#include "finalmq/protocols/protocolhelpers/ProtocolDelimiter.h"


namespace finalmq {


class SYMBOLEXP ProtocolDelimiterLinefeed : public ProtocolDelimiter
{
public:
    static const std::uint32_t PROTOCOL_ID;           // 3
    static const std::string PROTOCOL_NAME; // delimiter_lf

    ProtocolDelimiterLinefeed();

private:
    // IProtocol
    virtual std::uint32_t getProtocolId() const override;
};


class SYMBOLEXP ProtocolDelimiterLinefeedFactory : public IProtocolFactory
{
public:

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol(const Variant& data) override;
};

}   // namespace finalmq
