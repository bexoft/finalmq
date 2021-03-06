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


#include "finalmq/protocols/ProtocolHeaderBinarySize.h"
#include "finalmq/protocolconnection/ProtocolMessage.h"
#include "finalmq/protocolconnection/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"

#include <atomic>

namespace finalmq {


static const int HEADERSIZE = 4;


//---------------------------------------
// ProtocolHeaderBinarySize
//---------------------------------------


ProtocolHeaderBinarySize::ProtocolHeaderBinarySize()
    : m_headerHelper(HEADERSIZE, [] (const std::string& header) {
            assert(header.size() == 4);
            int sizePayload = 0;
            for (int i = 0; i < 4; ++i)
            {
                sizePayload |= ((unsigned int)header[i] & 0xff) << (8 * i);
            }
            return sizePayload;
      })
{

}


// IProtocol
void ProtocolHeaderBinarySize::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

std::uint32_t ProtocolHeaderBinarySize::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolHeaderBinarySize::areMessagesResendable() const
{
    return true;
}

IMessagePtr ProtocolHeaderBinarySize::createMessage() const
{
    return std::make_shared<ProtocolMessage>(PROTOCOL_ID, HEADERSIZE);
}

void ProtocolHeaderBinarySize::receive(const SocketPtr& socket, int bytesToRead)
{
    std::deque<IMessagePtr> messages;
    m_headerHelper.receive(socket, bytesToRead, messages);
    auto callback = m_callback.lock();
    if (callback)
    {
        for (auto it = messages.begin(); it != messages.end(); ++it)
        {
            callback->received(*it);
        }
    }
}

void ProtocolHeaderBinarySize::prepareMessageToSend(IMessagePtr message)
{
    if (!message->wasSent())
    {
        ssize_t sizePayload = message->getTotalSendPayloadSize();
        const std::list<BufferRef>& buffers = message->getAllSendBuffers();
        assert(!buffers.empty());
        assert(buffers.begin()->second >= 4);
        char* header = buffers.begin()->first;
        for (int i = 0; i < 4; ++i)
        {
            header[i] = (sizePayload >> (i * 8)) & 0xff;
        }
        message->prepareMessageToSend();
    }
}

void ProtocolHeaderBinarySize::socketConnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
}

void ProtocolHeaderBinarySize::socketDisconnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}



//---------------------------------------
// ProtocolHeaderBinarySizeFactory
//---------------------------------------

struct RegisterProtocolHeaderBinarySizeFactory
{
    RegisterProtocolHeaderBinarySizeFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolHeaderBinarySize::PROTOCOL_ID, std::make_shared<ProtocolHeaderBinarySizeFactory>());
    }
} g_registerProtocolHeaderBinarySizeFactory;



// IProtocolFactory
IProtocolPtr ProtocolHeaderBinarySizeFactory::createProtocol()
{
    return std::make_shared<ProtocolHeaderBinarySize>();
}

}   // namespace finalmq
