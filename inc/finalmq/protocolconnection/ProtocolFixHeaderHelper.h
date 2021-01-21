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

#include "finalmq/streamconnection/IMessage.h"
#include "finalmq/protocolconnection/IProtocol.h"
#include "finalmq/helpers/BexDefines.h"

#include <deque>
#include <functional>

namespace finalmq {

class SYMBOLEXP ProtocolFixHeaderHelper
{
public:
    ProtocolFixHeaderHelper(int sizeHeader, std::function<int(const std::string& header)> funcGetPayloadSize);

    void receive(const SocketPtr& socket, int bytesToRead, std::deque<IMessagePtr>& messages);

private:

    enum class State
    {
        WAITFORHEADER,
        HEADERRECEIVED,
        WAITFORPAYLOAD,
        PAYLOADRECEIVED
    };

    bool receiveHeader(const SocketPtr& socket, int& bytesToRead);
    void setPayloadSize(int sizePayload);
    bool receivePayload(const SocketPtr& socket, int& bytesToRead);
    void clearState();

    std::string m_header;
    State       m_state = State::WAITFORHEADER;
    ssize_t     m_sizeCurrent = 0;

    ssize_t     m_sizePayload = 0;
    IMessagePtr m_message;
    char*       m_payload = nullptr;

    std::function<int(const std::string& header)>   m_funcGetPayloadSize;
};

}   // namespace finalmq
