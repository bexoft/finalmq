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
#include <vector>


namespace finalmq {


class SYMBOLEXP ProtocolDelimiter : public IProtocol
{
public:
    ProtocolDelimiter(const std::string& delimiter, const Variant& data);
    ~ProtocolDelimiter();

private:
    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual void setConnection(const IStreamConnectionPtr& connection) override;
    virtual IStreamConnectionPtr getConnection() const override;
    virtual void disconnect() override;
    virtual std::uint32_t getProtocolId() const = 0;
    virtual bool areMessagesResendable() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool doesSupportSession() const override;
    virtual bool needsReply() const override;
    virtual bool isMultiConnectionSession() const override;
    virtual bool isSendRequestByPoll() const override;
    virtual bool doesSupportFileTransfer() const override;
    virtual FuncCreateMessage getMessageFactory() const override;
    virtual void sendMessage(IMessagePtr message) override;
    virtual void moveOldProtocolState(IProtocol& protocolOld) override;
    virtual bool received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;
    virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual IMessagePtr pollReply(std::deque<IMessagePtr>&& messages) override;
    virtual void subscribe(const std::vector<std::string>& subscribtions) override;
    virtual void cycleTime() override;

    std::vector<ssize_t> findEndOfMessage(const char* buffer, ssize_t size);

    std::weak_ptr<IProtocolCallback>    m_callback;
    IStreamConnectionPtr                m_connection;

    const std::string                   m_delimiter;
    const char                          m_delimiterStart;
    ssize_t                             m_indexStartMessage = 0;
    std::shared_ptr<std::string>        m_receiveBuffer;
    ssize_t                             m_bufferSize = 0;

    struct ReceiveBufferStore
    {
        std::shared_ptr<std::string> receiveBuffer;
        ssize_t                      indexStartMessage = -1;
        ssize_t                      indexEndMessage = -1;
    };
    std::vector<ReceiveBufferStore>     m_receiveBuffers;
    ssize_t                             m_receiveBuffersTotal = 0;

    Variant                             m_data;

    mutable std::mutex                  m_mutex;
};



}   // namespace finalmq
