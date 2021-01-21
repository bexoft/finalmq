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
#include "finalmq/protocolconnection/ProtocolFixHeaderHelper.h"



namespace finalmq {


class SYMBOLEXP ProtocolHeaderBinarySize : public IProtocol
{
public:
    static const std::uint32_t PROTOCOL_ID;

    ProtocolHeaderBinarySize();

private:
    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual std::uint32_t getProtocolId() const override;
    virtual bool areMessagesResendable() const override;
    virtual IMessagePtr createMessage() const override;
    virtual void receive(const SocketPtr& socket, int bytesToRead) override;
    virtual void prepareMessageToSend(IMessagePtr message) override;
    virtual void socketConnected() override;
    virtual void socketDisconnected() override;

    std::weak_ptr<IProtocolCallback>    m_callback;
    ProtocolFixHeaderHelper             m_headerHelper;
};


class SYMBOLEXP ProtocolHeaderBinarySizeFactory : public IProtocolFactory
{
public:

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol() override;
};

}   // namespace finalmq
