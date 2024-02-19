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

#include <deque>
#include <functional>

#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/protocols/mqtt5/Mqtt5Client.h"
#include "finalmq/protocolsession/IProtocol.h"

namespace finalmq
{
class SYMBOLEXP ProtocolMqtt5Client : public IProtocol, public IMqtt5ClientCallback
{
public:
    static const std::string KEY_USERNAME;              ///< the username for the broker
    static const std::string KEY_PASSWORD;              ///< the password for the broker
    static const std::string KEY_SESSIONEXPIRYINTERVAL; ///< the mqtt session expiry interval in seconds
    static const std::string KEY_KEEPALIVE;             ///< the mqtt keep alive interval in seconds
    static const std::string KEY_TOPIC_PREFIX;          ///< a topic prefix

    static const std::uint32_t PROTOCOL_ID; // 5
    static const std::string PROTOCOL_NAME; // mqtt5client

    ProtocolMqtt5Client(const Variant& data);
    ~ProtocolMqtt5Client();

private:
    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual void setConnection(const IStreamConnectionPtr& connection) override;
    virtual IStreamConnectionPtr getConnection() const override;
    virtual void disconnect() override;
    virtual std::uint32_t getProtocolId() const override;
    virtual bool areMessagesResendable() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool doesSupportSession() const override;
    virtual bool needsReply() const override;
    virtual bool isMultiConnectionSession() const override;
    virtual bool isSendRequestByPoll() const override;
    virtual bool doesSupportFileTransfer() const override;
    virtual bool isSynchronousRequestReply() const override;
    virtual FuncCreateMessage getMessageFactory() const override;
    virtual void sendMessage(IMessagePtr message) override;
    virtual void moveOldProtocolState(IProtocol& protocolOld) override;
    virtual bool received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;
    virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual IMessagePtr pollReply(std::deque<IMessagePtr>&& messages) override;
    virtual void subscribe(const std::vector<std::string>& subscribtions) override;
    virtual void cycleTime() override;
    virtual IProtocolSessionDataPtr createProtocolSessionData() override;
    virtual void setProtocolSessionData(const IProtocolSessionDataPtr& protocolSessionData) override;

    // IMqtt5ClientCallback
    virtual void receivedConnAck(const ConnAckData& data) override;
    virtual void receivedPublish(const PublishData& data, const IMessagePtr& message) override;
    virtual void receivedSubAck(const std::vector<std::uint8_t>& reasoncodes) override;
    virtual void receivedUnsubAck(const std::vector<std::uint8_t>& reasoncodes) override;
    virtual void receivedPingResp() override;
    virtual void receivedDisconnect(const DisconnectData& data) override;
    virtual void receivedAuth(const AuthData& data) override;
    virtual void closeConnection() override;

    enum class State
    {
        WAITFORHEADER,
        WAITFORLENGTH,
        WAITFORPAYLOAD,
        MESSAGECOMPLETE,
    };

    bool receiveHeader(const SocketPtr& socket, int& bytesToRead);
    bool receiveRemainingSize(const SocketPtr& socket, int& bytesToRead);
    void setPayloadSize();
    bool receivePayload(const SocketPtr& socket, int& bytesToRead);
    bool processPayload();
    void clearState();

    std::string m_username{};
    std::string m_password{};
    std::uint32_t m_sessionExpiryInterval{ 5 * 60 }; // default 5 minutes
    std::uint32_t m_keepAlive{ 20 };                 // default 20 seconds
    std::string m_topicPrefix{};
    std::string m_sessionPrefix{};
    std::string m_clientId{};
    std::string m_virtualSessionId{};

    bool m_firstConnection{ true };
    PollingTimer m_timerReconnect{};

    std::weak_ptr<IProtocolCallback> m_callback{};
    IStreamConnectionPtr m_connection{};
    std::unique_ptr<IMqtt5Client> m_client{};
    mutable std::mutex m_mutex{};
};

class SYMBOLEXP ProtocolMqtt5ClientFactory : public IProtocolFactory
{
public:
private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol(const Variant& data) override;
};

} // namespace finalmq
