#pragma once

#include "streamconnection/IMessage.h"
#include "protocolconnection/IProtocol.h"
#include <vector>
#include <functional>



class ProtocolFixHeaderHelper
{
public:
    ProtocolFixHeaderHelper(int sizeHeader, std::function<int(const std::string& header)> funcGetPayloadSize);

    std::vector<IMessagePtr> receive(const SocketPtr& socket, int bytesToRead);

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
    int         m_sizeCurrent = 0;

    int         m_sizePayload = 0;
    IMessagePtr m_message;
    char*       m_payload = nullptr;

    std::function<int(const std::string& header)>   m_funcGetPayloadSize;
};

