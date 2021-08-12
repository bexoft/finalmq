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
#include "finalmq/protocolsession/IProtocol.h"
#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/metadata/MetaType.h"

#include <deque>
#include <functional>

namespace finalmq {

class SYMBOLEXP ProtocolMqtt5 : public IProtocol
{
public:
    static const int PROTOCOL_ID;           // 5
    static const std::string PROTOCOL_NAME; // mqtt5

    ProtocolMqtt5();

    bool receive(const SocketPtr& socket, int bytesToRead, std::deque<IMessagePtr>& messages);

private:
    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual void setConnection(const IStreamConnectionPtr& connection) override;
    virtual std::uint32_t getProtocolId() const override;
    virtual bool areMessagesResendable() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool doesSupportSession() const override;
    virtual bool needsReply() const override;
    virtual bool isMultiConnectionSession() const override;
    virtual bool isSendRequestByPoll() const override;
    virtual bool doesSupportFileTransfer() const override;
    virtual FuncCreateMessage getMessageFactory() const override;
    virtual bool sendMessage(IMessagePtr message) override;
    virtual void moveOldProtocolState(IProtocol& protocolOld) override;
    virtual bool received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;
    virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual IMessagePtr pollReply(std::deque<IMessagePtr>&& messages) override;

    enum class State
    {
        WAITFORHEADER,
        WAITFORLENGTH,
        WAITFORPAYLOAD
    };

    bool receiveHeader(const SocketPtr& socket, int& bytesToRead);
    bool receiveRemainingSize(const SocketPtr& socket, int& bytesToRead);
    void setPayloadSize();
    bool receivePayload(const SocketPtr& socket, int& bytesToRead);
    void handlePayloadReceived();
    bool processPayload();
    void clearState();

    bool handleConnect();
    bool handleConnAck();
    bool handlePublish();
    bool handlePubAck();
    bool handlePubRec();
    bool handlePubRel();
    bool handlePubComp();
    bool handleSubscribe();
    bool handleSubAck();
    bool handleUnsubscribe();
    bool handleUnsubAck();
    bool handlePing();
    bool handlePingResp();
    bool handleDisconnect();
    bool handleAuth();

    bool doesFit(int size) const;
    bool read1ByteNumber(unsigned int& number);
    bool read2ByteNumber(unsigned int& number);
    bool read4ByteNumber(unsigned int& number);
    bool readVarByteNumber(unsigned int& number);
    bool readString(std::string& str);
    bool readStringPair(std::string& key, std::string& value);
    bool readBinary(Bytes& value);
    bool readProperties(Variant& props, Variant* userprops, IMessage::Metainfo* metainfo);

    std::weak_ptr<IProtocolCallback>    m_callback;
    IStreamConnectionPtr                m_connection;

    unsigned char m_header;
    int         m_remainingSize = 0;
    int         m_remainingSizeShift = 0;
    State       m_state = State::WAITFORHEADER;
    ssize_t     m_sizeCurrent = 0;

    ssize_t     m_sizePayload = 0;
    IMessagePtr m_message;
    char*       m_buffer = nullptr;
    int         m_indexRead = 0;

    std::deque<IMessagePtr>* m_messages = nullptr;
};


class SYMBOLEXP ProtocolMqtt5Factory : public IProtocolFactory
{
public:

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol() override;
};


}   // namespace finalmq
