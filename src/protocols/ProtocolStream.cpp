
#include "protocols/ProtocolStream.h"
#include "protocolconnection/ProtocolMessage.h"
#include "streamconnection/Socket.h"




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

int ProtocolStream::getProtocolId() const
{
    return m_protocolId;
}

bool ProtocolStream::areMessagesResendable() const
{
    return true;
}

IMessagePtr ProtocolStream::createMessage() const
{
    return std::make_shared<ProtocolMessage>(m_protocolId);
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

void ProtocolStream::socketConnected()
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



// IProtocolFactory
IProtocolPtr ProtocolStreamFactory::createProtocol()
{
    return std::make_shared<ProtocolStream>();
}

