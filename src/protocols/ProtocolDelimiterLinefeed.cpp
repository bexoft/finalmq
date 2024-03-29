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


#include "finalmq/protocols/ProtocolDelimiterLinefeed.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"


namespace finalmq {

//---------------------------------------
// ProtocolStream
//---------------------------------------

const std::uint32_t ProtocolDelimiterLinefeed::PROTOCOL_ID = 3;
const std::string ProtocolDelimiterLinefeed::PROTOCOL_NAME = "delimiter_lf";


ProtocolDelimiterLinefeed::ProtocolDelimiterLinefeed()
    : ProtocolDelimiter("\n")
{

}


// IProtocol
std::uint32_t ProtocolDelimiterLinefeed::getProtocolId() const
{
    return PROTOCOL_ID;
}




//---------------------------------------
// ProtocolDelimiterLinefeedFactory
//---------------------------------------


struct RegisterProtocolDelimiterLinefeedFactory
{
    RegisterProtocolDelimiterLinefeedFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolDelimiterLinefeed::PROTOCOL_NAME, ProtocolDelimiterLinefeed::PROTOCOL_ID, std::make_shared<ProtocolDelimiterLinefeedFactory>());
    }
} g_registerProtocolDelimiterLinefeedFactory;


// IProtocolFactory
IProtocolPtr ProtocolDelimiterLinefeedFactory::createProtocol(const Variant& /*data*/)
{
    return std::make_shared<ProtocolDelimiterLinefeed>();
}


}   // namespace finalmq
