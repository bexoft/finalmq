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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "finalmq/protocols/protocolhelpers/ProtocolDelimiter.h"
#include "finalmq//protocolsession/ProtocolMessage.h"
#include "MockIProtocolCallback.h"
#include "finalmq/helpers/OperatingSystem.h"
#include "finalmq/streamconnection/Socket.h"

#include "MockIOperatingSystem.h"
#include "MockIStreamConnection.h"


using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::DoAll;
using ::testing::SetArrayArgument;

using namespace finalmq;


MATCHER_P(MatcherReceiveMessage, message, "")
{
    return (arg->getAllMetainfo() == message->getAllMetainfo() &&
        arg->getReceivePayload().second == message->getReceivePayload().second &&
        memcmp(arg->getReceivePayload().first, message->getReceivePayload().first, arg->getReceivePayload().second) == 0);
}



class ProtocolDelimiterTest : public ProtocolDelimiter
{
public:

    ProtocolDelimiterTest() 
        : ProtocolDelimiter("lol")
    {
    }

private:
    // IProtocol
    virtual std::uint32_t getProtocolId() const override
    {
        return 12345;
    }
};



class TestProtocolDelimiter: public testing::Test
{
protected:
    virtual void SetUp()
    {
        m_mockOperatingSystem = new MockIOperatingSystem;
        OperatingSystem::setInstance(std::unique_ptr<IOperatingSystem>(m_mockOperatingSystem));

        EXPECT_CALL(*m_mockOperatingSystem, socket(0, 0, 0)).WillOnce(Return(3));
        EXPECT_CALL(*m_mockOperatingSystem, setNonBlocking(3, true)).WillOnce(Return(3));
        EXPECT_CALL(*m_mockOperatingSystem, setLinger(3, true, 0)).WillOnce(Return(3));
        m_socket->create(0, 0, 0);

        m_protocol = std::make_shared<ProtocolDelimiterTest>();
        m_protocol->setCallback(m_mockCallback);
    }

    virtual void TearDown()
    {
        EXPECT_CALL(*m_mockOperatingSystem, closeSocket(m_socket->getSocketDescriptor()->getDescriptor())).WillOnce(Return(0));
        m_socket = nullptr;
        OperatingSystem::setInstance({});   // destroy the mock
        if (m_protocol->getConnection())
        {
            EXPECT_CALL(*m_mockStreamConnection, disconnect()).WillOnce(Return());
        }
        m_protocol = nullptr;
    }

    MockIOperatingSystem*                   m_mockOperatingSystem = nullptr;
    IProtocolPtr                            m_protocol = nullptr;
    std::shared_ptr<MockIProtocolCallback>  m_mockCallback = std::make_shared<MockIProtocolCallback>();
    std::shared_ptr<MockIStreamConnection>  m_mockStreamConnection = std::make_shared<MockIStreamConnection>();
    std::shared_ptr<Socket>                 m_socket = std::make_shared<Socket>();
};



TEST_F(TestProtocolDelimiter, testReceiveByteByByte)
{
    EXPECT_CALL(*m_mockCallback, received(_, _)).Times(0);
    std::string receiveBuffer = "H";
    int size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "l";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "o";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    message->resizeReceiveBuffer(1);
    memcpy(message->getReceivePayload().first, "H", 1);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message), _)).Times(1);
    receiveBuffer = "l";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "Ha";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "l";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "o";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    message->resizeReceiveBuffer(2);
    memcpy(message->getReceivePayload().first, "Ha", 2);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message), _)).Times(1);
    receiveBuffer = "l";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);
}

TEST_F(TestProtocolDelimiter, testReceive)
{
    std::shared_ptr<IMessage> message1 = std::make_shared<ProtocolMessage>(0);
    message1->resizeReceiveBuffer(1);
    memcpy(message1->getReceivePayload().first, "A", 1);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message1), _)).Times(1);
    std::string receiveBuffer = "AlolB";
    int size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "CD";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "EF";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    receiveBuffer = "Gl";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);

    std::shared_ptr<IMessage> message2 = std::make_shared<ProtocolMessage>(0);
    message2->resizeReceiveBuffer(6);
    memcpy(message2->getReceivePayload().first, "BCDEFG", 6);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message2), _)).Times(1);
    receiveBuffer = "ol";
    size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data() + size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);
}

