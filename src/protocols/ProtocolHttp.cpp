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
#include "finalmq/protocolconnection/ProtocolSession.h"
#include "finalmq/streamconnection/Socket.h"


namespace finalmq {


const std::string ProtocolHttp::FMQ_HTTP = "_fmq_http";
const std::string ProtocolHttp::FMQ_METHOD = "_fmq_method";
const std::string ProtocolHttp::FMQ_PROTOCOL = "_fmq_protocol";
const std::string ProtocolHttp::FMQ_PATH = "_fmq_path";
const std::string ProtocolHttp::FMQ_QUERY = "_fmq_query";
const std::string ProtocolHttp::FMQ_STATUS = "_fmq_status";
const std::string ProtocolHttp::FMQ_STATUSTEXT = "_fmq_status";
const std::string ProtocolHttp::HTTP_REQUEST = "request";
const std::string ProtocolHttp::HTTP_RESPONSE = "response";
    
static const std::string CONTENT_LENGTH = "Content-Length";



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
        if (pos == std::string::npos || static_cast<ssize_t>(pos) > indexEnd)
        {
            pos = indexEnd;
        }
        ssize_t len = pos - indexBegin;
        assert(len >= 0);
        dest.emplace_back(&src[indexBegin], len);
        indexBegin += len + 1;
    }
}

static void splitOnce(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest)
{
    const char delimiterstring[] = { delimiter, 0 };

    size_t pos = src.find_first_of(delimiterstring, indexBegin);
    if (pos == std::string::npos || static_cast<ssize_t>(pos) > indexEnd)
    {
        pos = indexEnd;
    }
    ssize_t len = pos - indexBegin;
    assert(len >= 0);
    dest.emplace_back(&src[indexBegin], len);
    indexBegin += len + 1;

    if (indexBegin < indexEnd)
    {
        ssize_t len = indexEnd - indexBegin;
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
                                m_message->addMetadata(FMQ_HTTP, std::string(HTTP_RESPONSE));
                                m_message->addMetadata(FMQ_PROTOCOL, std::move(lineSplit[0]));
                                m_message->addMetadata(FMQ_STATUS, std::move(lineSplit[1]));
                                m_message->addMetadata(FMQ_STATUSTEXT, std::move(lineSplit[2]));
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
                                m_message->addMetadata(FMQ_HTTP, std::string(HTTP_REQUEST));
                                m_message->addMetadata(FMQ_METHOD, std::move(lineSplit[0]));
                                if (pathquerySplit.size() >= 1)
                                {
                                    m_message->addMetadata(FMQ_PATH, std::move(pathquerySplit[0]));
                                }
                                if (pathquerySplit.size() >= 2)
                                {
                                    m_message->addMetadata(FMQ_QUERY, std::move(pathquerySplit[1]));
                                }
                                m_message->addMetadata(FMQ_PROTOCOL, std::move(lineSplit[2]));
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
                        splitOnce(m_receiveBuffer, m_offsetRemaining, indexEndLine, ':', lineSplit);
                        if (lineSplit.size() == 2)
                        {
                            std::string& value = lineSplit[1];
                            if (!value.empty() && value[0] == ' ')
                            {
                                value.erase(value.begin());
                            }
                            if (lineSplit[0] == CONTENT_LENGTH)
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
                            assert(false);
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


static std::string HEADER_KEEP_ALIVE = "Connection: keep-alive\r\n";

void ProtocolHttp::prepareMessageToSend(IMessagePtr message)
{
    std::string firstLine;
    const std::string& http = message->getMetadata(FMQ_HTTP);
    if (http == HTTP_REQUEST)
    {
        const std::string& method = message->getMetadata(FMQ_METHOD);
        const std::string& path = message->getMetadata(FMQ_PATH);
        const std::string& query = message->getMetadata(FMQ_QUERY);
        firstLine = method;
        firstLine += ' ';
        firstLine += path;
        if (!query.empty())
        {
            firstLine += '?';
            firstLine += query;
        }
        firstLine += " HTTP/1.1";
    }
    else
    {
        const std::string& status = message->getMetadata(FMQ_STATUS);
        const std::string& statustext = message->getMetadata(FMQ_STATUSTEXT);
        firstLine = "HTTP/1.1 ";
        firstLine += status;
        firstLine += ' ';
        firstLine += statustext;
    }

    int sumHeaderSize = firstLine.size() + 4;   // 2 = '\r\n' and last empty line
    if (http == HTTP_REQUEST)
    {
        sumHeaderSize += m_headerHost.size();   // Host: hostname\r\n
    }
    sumHeaderSize += HEADER_KEEP_ALIVE.size();  // Connection: keep-alive\r\n

    ProtocolMessage::Metadata& metadata = message->getAllMetadata();
    static const std::string PREFIX_PRIVATE_HEADER = "_fmq_";
    for (size_t i = 0; i < metadata.size(); ++i)
    {
        const std::string& key = metadata[i].first;
        const std::string& value = metadata[i].second;
        if (!key.empty() && key.find_first_of(PREFIX_PRIVATE_HEADER) != 0)
        {
            sumHeaderSize += key.size() + value.size() + 4;    // 4 = ': ' and '\r\n'
        }
    }

    char* headerBuffer = message->addSendHeader(sumHeaderSize);
    int index = 0;
    assert(index + firstLine.size() + 2 <= sumHeaderSize);
    memcpy(headerBuffer + index, firstLine.data(), firstLine.size());
    index += firstLine.size();
    memcpy(headerBuffer + index, "\r\n", 2);
    index += 2;

    if (http == HTTP_REQUEST)
    {
        // Host: hostname\r\n
        assert(index + m_headerHost.size() <= sumHeaderSize);
        memcpy(headerBuffer + index, m_headerHost.data(), m_headerHost.size());
        index += m_headerHost.size();
    }

    // Connection: keep-alive\r\n
    assert(index + HEADER_KEEP_ALIVE.size() <= sumHeaderSize);
    memcpy(headerBuffer + index, HEADER_KEEP_ALIVE.data(), HEADER_KEEP_ALIVE.size());
    index += HEADER_KEEP_ALIVE.size();

    for (size_t i = 0; i < metadata.size(); ++i)
    {
        const std::string& key = metadata[i].first;
        const std::string& value = metadata[i].second;
        if (!key.empty() && key.find_first_of(PREFIX_PRIVATE_HEADER) != 0)
        {
            assert(index + key.size() + metadata[i].second.size() + 4 <= sumHeaderSize);
            memcpy(headerBuffer + index, key.data(), key.size());
            index += key.size();
            memcpy(headerBuffer + index, ": ", 2);
            index += 2;
            memcpy(headerBuffer + index, value.data(), value.size());
            index += value.size();
            memcpy(headerBuffer + index, "\r\n", 2);
            index += 2;
        }
    }
    assert(index + 2 == sumHeaderSize);
    memcpy(headerBuffer + index, "\r\n", 2);
    index += 2;
    assert(index == sumHeaderSize);

    message->prepareMessageToSend();
}

void ProtocolHttp::socketConnected(IProtocolSession& session)
{
    ConnectionData connectionData = session.getConnectionData();
    m_headerHost = "Host: " + connectionData.hostname + ":" + std::to_string(connectionData.port) + "\r\n";
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
