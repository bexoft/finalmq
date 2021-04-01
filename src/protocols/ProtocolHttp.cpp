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


#include "finalmq/protocols/ProtocolHttp.h"
#include "finalmq/protocolconnection/ProtocolMessage.h"
#include "finalmq/protocolconnection/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"


namespace finalmq {

//---------------------------------------
// ProtocolStream
//---------------------------------------


ProtocolHttp::ProtocolHttp()
{

}



// IProtocol
void ProtocolHttp::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

std::uint32_t ProtocolHttp::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolHttp::areMessagesResendable() const
{
    return false;
}

IMessagePtr ProtocolHttp::createMessage() const
{
    return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
}

void ProtocolHttp::receive(const SocketPtr& socket, int bytesToRead)
{

}

void ProtocolHttp::prepareMessageToSend(IMessagePtr message)
{
    message->prepareMessageToSend();
}

void ProtocolHttp::socketConnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
}

void ProtocolHttp::socketDisconnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}



//---------------------------------------
// ProtocolHttpFactory
//---------------------------------------


struct RegisterProtocolHttpFactory
{
    RegisterProtocolHttpFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolHttp::PROTOCOL_ID, std::make_shared<ProtocolHttpFactory>());
    }
} g_registerProtocolHttpFactory;


// IProtocolFactory
IProtocolPtr ProtocolHttpFactory::createProtocol()
{
    return std::make_shared<ProtocolHttp>();
}

}   // namespace finalmq
