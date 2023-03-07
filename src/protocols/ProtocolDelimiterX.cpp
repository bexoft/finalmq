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


#include "finalmq/protocols/ProtocolDelimiterX.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"
#include "finalmq/variant/VariantValues.h"


namespace finalmq {

//---------------------------------------
// ProtocolStream
//---------------------------------------

const std::uint32_t ProtocolDelimiterX::PROTOCOL_ID = 6;
const std::string ProtocolDelimiterX::PROTOCOL_NAME = "delimiter_x";

const std::string ProtocolDelimiterX::KEY_DELIMITER = "delimiter";



static std::string getDelimiter(const Variant& data)
{
    std::string delimiter = data.getDataValue<std::string>(ProtocolDelimiterX::KEY_DELIMITER);
    if (delimiter.empty())
    {
        return "\n";
    }
    return delimiter;
}



ProtocolDelimiterX::ProtocolDelimiterX(const Variant& data)
    : ProtocolDelimiter(getDelimiter(data))
{
}


// IProtocol
std::uint32_t ProtocolDelimiterX::getProtocolId() const
{
    return PROTOCOL_ID;
}



//---------------------------------------
// ProtocolDelimiterXFactory
//---------------------------------------


struct RegisterProtocolDelimiterXFactory
{
    RegisterProtocolDelimiterXFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolDelimiterX::PROTOCOL_NAME, ProtocolDelimiterX::PROTOCOL_ID, std::make_shared<ProtocolDelimiterXFactory>());
    }
} g_registerProtocolDelimiterXFactory;


// IProtocolFactory
IProtocolPtr ProtocolDelimiterXFactory::createProtocol(const Variant& data)
{
    return std::make_shared<ProtocolDelimiterX>(data);
}


}   // namespace finalmq
