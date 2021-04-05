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


static void split(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest)
{
    const char delimiterstring[] = { delimiter, 0 };
    while (indexBegin < indexEnd)
    {
        size_t pos = src.find_first_of(delimiterstring, indexBegin);
        if (pos == std::string::npos || pos > indexEnd)
        {
            pos = indexEnd;
        }
        ssize_t len = pos - indexBegin;
        assert(len >= 0);
        dest.emplace_back(&src[indexBegin], len);
        indexBegin += len + 1;
    }
}


bool ProtocolHttp::receiveHeaders(ssize_t bytesReceived)
{
    bool ok = true;
    bytesReceived += m_sizeRemaining;
    assert(bytesReceived <= static_cast<ssize_t>(m_receiveBuffer.size()));
    while (m_offsetRemaining < bytesReceived && ok)
    {
        size_t index = m_receiveBuffer.find_first_of('\n', m_offsetRemaining);
        if (index != std::string::npos)
        {
            ssize_t indexEndLine = index;
            --indexEndLine; // goto '\r'
            ssize_t len = indexEndLine - m_offsetRemaining;
            if (len < 0 || m_receiveBuffer[indexEndLine] != '\r')
            {
                ok = false;
            }
            if (ok)
            {
                if (m_state == STATE_FIND_FIRST_LINE)
                {
                    m_contentLength = 0;
                    if (len < 4)
                    {
                        ok = false;
                    }
                    else
                    {
                        // is response
                        if (m_receiveBuffer[m_offsetRemaining] == 'H' && m_receiveBuffer[m_offsetRemaining + 1] == 'T')
                        {
                            std::vector<std::string> lineSplit;
                            split(m_receiveBuffer, m_offsetRemaining, indexEndLine, ' ', lineSplit);
                            if (lineSplit.size() == 3)
                            {
                                m_message = std::make_shared<ProtocolMessage>(0);
                                m_message->addMetadata("fmq_http", "response");
                                m_message->addMetadata("fmq_protocol", std::move(lineSplit[0]));
                                m_message->addMetadata("fmq_status", std::move(lineSplit[1]));
                                m_message->addMetadata("fmq_statustext", std::move(lineSplit[2]));
                                m_state = STATE_FIND_HEADERS;
                            }
                            else
                            {
                                ok = false;
                            }
                        }
                        else
                        {
                            std::vector<std::string> lineSplit;
                            split(m_receiveBuffer, m_offsetRemaining, indexEndLine, ' ', lineSplit);
                            if (lineSplit.size() == 3)
                            {
                                std::vector<std::string> pathquerySplit;
                                split(lineSplit[1], 0, lineSplit[1].size(), '?', pathquerySplit);
                                m_message = std::make_shared<ProtocolMessage>(0);
                                m_message->addMetadata("fmq_http", "request");
                                m_message->addMetadata("fmq_method", std::move(lineSplit[0]));
                                if (pathquerySplit.size() >= 1)
                                {
                                    m_message->addMetadata("fmq_path", std::move(pathquerySplit[0]));
                                }
                                if (pathquerySplit.size() >= 2)
                                {
                                    m_message->addMetadata("fmq_query", std::move(pathquerySplit[1]));
                                }
                                m_message->addMetadata("fmq_protocol", std::move(lineSplit[2]));
                                m_state = STATE_FIND_HEADERS;
                            }
                            else
                            {
                                ok = false;
                            }
                        }
                    }
                }
                else if (m_state == STATE_FIND_HEADERS)
                {
                    if (len == 0)
                    {
                        if (m_contentLength == 0)
                        {
                            m_state = STATE_CONTENT_DONE;
                        }
                        else
                        {
                            m_state = STATE_CONTENT;
                            m_message->resizeReceivePayload(m_contentLength);
                        }
                        m_indexFilled = 0;
                        m_offsetRemaining += 2;
                        break;
                    }
                    else
                    {
                        std::vector<std::string> lineSplit;
                        split(m_receiveBuffer, m_offsetRemaining, indexEndLine, ':', lineSplit);
                        if (lineSplit.size() == 2)
                        {
                            std::string& value = lineSplit[1];
                            if (!value.empty() && value[0] == ' ')
                            {
                                value.erase(value.begin());
                            }
                            if (lineSplit[0] == "Content-Length")
                            {
                                m_contentLength = std::atoll(value.c_str());
                            }
                            m_message->addMetadata(std::move(lineSplit[0]), std::move(lineSplit[1]));
                        }
                        else if (lineSplit.size() == 1)
                        {
                            m_message->addMetadata("", std::move(lineSplit[0]));
                        }
                        else
                        {
                            ok = false;
                        }
                    }
                }
                m_offsetRemaining += len + 2;
            }
        }
        else
        {
            break;
        }
    }
    m_sizeRemaining = bytesReceived - m_offsetRemaining;
    assert(m_sizeRemaining >= 0);
    return ok;
}


void ProtocolHttp::reset()
{
    m_offsetRemaining = 0;
    m_sizeRemaining = 0;
    m_contentLength = 0;
    m_indexFilled = 0;
    m_message = nullptr;
    m_state = STATE_FIND_FIRST_LINE;
}


void ProtocolHttp::receive(const SocketPtr& socket, int bytesToRead)
{
    bool ok = true;

    if (m_state != STATE_CONTENT)
    {
        if (m_offsetRemaining == 0)
        {
            m_receiveBuffer.resize(m_sizeRemaining + bytesToRead);
        }
        else
        {
            std::string temp;
            if (m_sizeRemaining > 0)
            {
                temp = std::move(m_receiveBuffer);
            }
            m_receiveBuffer.resize(m_sizeRemaining + bytesToRead);
            if (m_sizeRemaining > 0)
            {
                memcpy(&m_receiveBuffer[0], &temp[m_offsetRemaining], m_sizeRemaining);
            }
            m_offsetRemaining = 0;
        }

        ssize_t bytesReceived = 0;
        int res = 0;
        do
        {
            res = socket->receive(const_cast<char*>(m_receiveBuffer.data() + bytesReceived + m_sizeRemaining), bytesToRead - bytesReceived);
            if (res > 0)
            {
                bytesReceived += res;
            }
        } while (res > 0 && bytesReceived < bytesToRead);
        if (res >= 0)
        {
            assert(bytesReceived <= bytesToRead);
            ok = receiveHeaders(bytesReceived);
            if (ok && m_state == STATE_CONTENT)
            {
                BufferRef payload = m_message->getReceivePayload();
                assert(payload.second == m_contentLength);
                if (m_sizeRemaining <= m_contentLength)
                {
                    memcpy(payload.first, m_receiveBuffer.data() + m_offsetRemaining, m_sizeRemaining);
                    m_indexFilled = m_sizeRemaining;
                    assert(m_indexFilled <= m_contentLength);
                    if (m_indexFilled == m_contentLength)
                    {
                        m_state = STATE_CONTENT_DONE;
                    }
                }
                else
                {
                    // too much content
                    ok = false;
                }
            }
        }
    }
    else
    {
        BufferRef payload = m_message->getReceivePayload();
        assert(payload.second == m_contentLength);
        ssize_t remainingContent = m_contentLength - m_indexFilled;
        if (bytesToRead <= remainingContent)
        {
            ssize_t bytesReceived = 0;
            int res = 0;
            do
            {
                res = socket->receive(const_cast<char*>(payload.first + bytesReceived + m_indexFilled), bytesToRead - bytesReceived);
                if (res > 0)
                {
                    bytesReceived += res;
                }
            } while (res > 0 && bytesReceived < bytesToRead);
            if (res >= 0)
            {
                m_indexFilled += bytesReceived;
                assert(m_indexFilled <= m_contentLength);
                if (m_indexFilled == m_contentLength)
                {
                    m_state = STATE_CONTENT_DONE;
                }
            }
        }
        else
        {
            // too much content
            ok = false;
        }
    }

    if (ok)
    {
        if (m_state == STATE_CONTENT_DONE)
        {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->received(m_message);
            }
            reset();
        }
    }
    else
    {
        reset();
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->disconnected();
        }
    }
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
