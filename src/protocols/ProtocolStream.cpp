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


#include "finalmq/protocols/ProtocolStream.h"
#include "finalmq/protocolconnection/ProtocolMessage.h"
#include "finalmq/protocolconnection/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"


namespace finalmq {

//---------------------------------------
// ProtocolStream
//---------------------------------------


ProtocolStream::ProtocolStream()
{

}


// IProtocol
void ProtocolStream::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

std::uint32_t ProtocolStream::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolStream::areMessagesResendable() const
{
    return true;
}

IMessagePtr ProtocolStream::createMessage() const
{
    return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
}

void ProtocolStream::receive(const SocketPtr& socket, int bytesToRead)
{
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* payload = message->resizeReceivePayload(bytesToRead);
    int res = socket->receive(payload, bytesToRead);
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= bytesToRead);
        message->resizeReceivePayload(bytesReceived);
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->received(message);
        }
    }
}

void ProtocolStream::prepareMessageToSend(IMessagePtr message)
{
    message->prepareMessageToSend();
}

void ProtocolStream::socketConnected(const IProtocolSessionPtr& /*session*/)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
}

void ProtocolStream::socketDisconnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}



//---------------------------------------
// ProtocolStreamFactory
//---------------------------------------

struct RegisterProtocolStreamFactory
{
    RegisterProtocolStreamFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolStream::PROTOCOL_ID, std::make_shared<ProtocolStreamFactory>());
    }
} g_registerProtocolStreamFactory;




// IProtocolFactory
IProtocolPtr ProtocolStreamFactory::createProtocol()
{
    return std::make_shared<ProtocolStream>();
}

}   // namespace finalmq
