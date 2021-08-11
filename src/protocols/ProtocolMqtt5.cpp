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


#include "finalmq/protocols/ProtocolMqtt5.h"
#include "finalmq/streamconnection/Socket.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"

namespace finalmq {

static const ssize_t HEADERSIZE = 1;

const int ProtocolMqtt5::PROTOCOL_ID = 5;
const std::string ProtocolMqtt5::PROTOCOL_NAME = "mqtt5";


ProtocolMqtt5::ProtocolMqtt5()
{
}


bool ProtocolMqtt5::receive(const SocketPtr& socket, int bytesToRead, std::deque<IMessagePtr>& messages)
{
    m_messages = &messages;
    bool ok = true;
    while ((bytesToRead > 0) && ok)
    {
        switch (m_state)
        {
        case State::WAITFORHEADER:
            ok = receiveHeader(socket, bytesToRead);
            break;
        case State::WAITFORLENGTH:
            ok = receiveRemainingSize(socket, bytesToRead);
            break;
        case State::WAITFORPAYLOAD:
            ok = receivePayload(socket, bytesToRead);
            break;
        default:
            assert(false);
            break;
        }
    }
    m_messages = nullptr;
    return ok;
}


bool ProtocolMqtt5::receiveHeader(const SocketPtr& socket, int& bytesToRead)
{
    assert(bytesToRead >= 1);
    assert(m_state == State::WAITFORHEADER);
    bool ret = false;

    int res = socket->receive(&m_header, HEADERSIZE);
    if (res == HEADERSIZE)
    {
        bytesToRead -= HEADERSIZE;
        ret = true;
        m_remainigSize = 0;
        m_remainigSizeShift = 0;
        m_state = State::WAITFORLENGTH;
    }
    return ret;
}

bool ProtocolMqtt5::receiveRemainingSize(const SocketPtr& socket, int& bytesToRead)
{
    assert(bytesToRead >= 1);
    assert(m_state == State::WAITFORHEADER);
    bool ok = true;
    bool done = false;
    while (bytesToRead > 1 && !done && ok)
    {
        ok = false;
        if (m_remainigSizeShift <= 21)
        {
            char data;
            int res = socket->receive(&data, 1);
            if (res == 1)
            {
                ok = true;
                bytesToRead -= 1;
                m_remainigSize |= (data & 0x7f) << m_remainigSizeShift;
                if ((data & 0x80) == 0)
                {
                    done = true;
                    setPayloadSize();
                }
                m_remainigSizeShift += 7;
            }
        }
    }

    return ok;
}


void ProtocolMqtt5::setPayloadSize()
{
    assert(m_state == State::WAITFORLENGTH);
    assert(m_remainigSize >= 0);
    m_message = std::make_shared<ProtocolMessage>(0, HEADERSIZE);
    m_buffer = m_message->resizeReceiveBuffer(HEADERSIZE + m_remainigSize);
    m_buffer[0] = m_header;
    if (m_remainigSize != 0)
    {
        m_state = State::WAITFORPAYLOAD;
    }
    else
    {
        handlePayloadReceived();
    }
    m_sizeCurrent = 0;
}


bool ProtocolMqtt5::receivePayload(const SocketPtr& socket, int& bytesToRead)
{
    assert(m_state == State::WAITFORPAYLOAD);
    assert(m_sizeCurrent < m_remainigSize);
    bool ok = false;

    ssize_t sizeRead = m_remainigSize - m_sizeCurrent;
    if (bytesToRead < sizeRead)
    {
        sizeRead = bytesToRead;
    }
    int res = socket->receive(m_buffer + HEADERSIZE + m_sizeCurrent, static_cast<int>(sizeRead));
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= sizeRead);
        bytesToRead -= bytesReceived;
        assert(bytesToRead >= 0);
        m_sizeCurrent += bytesReceived;
        assert(m_sizeCurrent <= m_remainigSize);
        if (bytesReceived == sizeRead)
        {
            ok = true;
        }
        if (m_sizeCurrent == m_remainigSize)
        {
            m_sizeCurrent = 0;
            if (ok)
            {
                ok = processPayload();
            }
            if (ok)
            {
                handlePayloadReceived();
            }
        }
    }
    return ok;
}

void ProtocolMqtt5::handlePayloadReceived()
{
    m_messages->push_back(m_message);
    clearState();
}


bool ProtocolMqtt5::processPayload()
{
    return false;
}


void ProtocolMqtt5::clearState()
{
    m_sizeCurrent = 0;
    m_sizePayload = 0;
    m_message = nullptr;
    m_buffer = nullptr;
    m_state = State::WAITFORHEADER;
}


// IProtocol
void ProtocolMqtt5::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

void ProtocolMqtt5::setConnection(const IStreamConnectionPtr& connection)
{
    m_connection = connection;
}

std::uint32_t ProtocolMqtt5::getProtocolId() const 
{
    return PROTOCOL_ID;
}

bool ProtocolMqtt5::areMessagesResendable() const 
{
    return false;
}

bool ProtocolMqtt5::doesSupportMetainfo() const 
{
    return true;
}

bool ProtocolMqtt5::doesSupportSession() const 
{
    return true;
}

bool ProtocolMqtt5::needsReply() const 
{
    return false;
}

bool ProtocolMqtt5::isMultiConnectionSession() const 
{
    return false;
}

bool ProtocolMqtt5::isSendRequestByPoll() const 
{
    return false;
}

bool ProtocolMqtt5::doesSupportFileTransfer() const 
{
    return false;
}

IProtocol::FuncCreateMessage ProtocolMqtt5::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}

bool ProtocolMqtt5::sendMessage(IMessagePtr message) 
{
    return false;
}

void ProtocolMqtt5::moveOldProtocolState(IProtocol& protocolOld) 
{
}

bool ProtocolMqtt5::received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) 
{
    std::deque<IMessagePtr> messages;
    bool ok = receive(socket, bytesToRead, messages);
    return ok;
}

hybrid_ptr<IStreamConnectionCallback> ProtocolMqtt5::connected(const IStreamConnectionPtr& connection) 
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
    return nullptr;
}

void ProtocolMqtt5::disconnected(const IStreamConnectionPtr& connection) 
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}

IMessagePtr ProtocolMqtt5::pollReply(std::deque<IMessagePtr>&& messages) 
{
    return {};
}


//---------------------------------------
// ProtocolMqtt5Factory
//---------------------------------------

struct RegisterProtocolMqtt5Factory
{
    RegisterProtocolMqtt5Factory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolMqtt5::PROTOCOL_NAME, ProtocolMqtt5::PROTOCOL_ID, std::make_shared<ProtocolMqtt5Factory>());
    }
} g_registerProtocolMqtt5Factory;




// IProtocolFactory
IProtocolPtr ProtocolMqtt5Factory::createProtocol()
{
    return std::make_shared<ProtocolMqtt5>();
}

}   // namespace finalmq
