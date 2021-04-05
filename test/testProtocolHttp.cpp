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

#include "finalmq/protocols/ProtocolHttp.h"
#include "finalmq//protocolconnection/ProtocolMessage.h"
#include "MockIProtocolCallback.h"
#include "finalmq/helpers/OperatingSystem.h"
#include "finalmq/streamconnection/Socket.h"

#include "MockIOperatingSystem.h"


using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::DoAll;
using ::testing::SetArrayArgument;

using namespace finalmq;




class TestProtocolHttp: public testing::Test
{
protected:
    virtual void SetUp()
    {
        m_mockMockOperatingSystem = new MockIOperatingSystem;
        std::unique_ptr<IOperatingSystem> iOperatingSystem(m_mockMockOperatingSystem);
        OperatingSystem::setInstance(iOperatingSystem);

        EXPECT_CALL(*m_mockMockOperatingSystem, socket(0, 0, 0)).WillOnce(Return(3));
        EXPECT_CALL(*m_mockMockOperatingSystem, setNonBlocking(3, true)).WillOnce(Return(3));
        EXPECT_CALL(*m_mockMockOperatingSystem, setLinger(3, true, 0)).WillOnce(Return(3));
        m_socket->create(0, 0, 0);

        m_protocol = &m_http;
        m_protocol->setCallback(m_mockCallback);
    }

    virtual void TearDown()
    {
        std::unique_ptr<IOperatingSystem> resetOperatingSystem;
        OperatingSystem::setInstance(resetOperatingSystem);
    }

    MockIOperatingSystem*                   m_mockMockOperatingSystem = nullptr;
    ProtocolHttp                            m_http;
    IProtocol*                              m_protocol = nullptr;
    std::shared_ptr<MockIProtocolCallback>  m_mockCallback = std::make_shared<MockIProtocolCallback>();
    std::shared_ptr<Socket>                 m_socket = std::make_shared<Socket>();
};



TEST_F(TestProtocolHttp, testFirstLineIncomplete)
{
    EXPECT_CALL(*m_mockCallback, received(_)).Times(0);
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);
    std::string receiveBuffer = "GET /hello HTTP/1.1\r";
    int size = receiveBuffer.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data()+size), Return(size)));
    m_protocol->receive(m_socket, size);
}


TEST_F(TestProtocolHttp, testFirstLinecomplete)
{
    EXPECT_CALL(*m_mockCallback, received(_)).Times(0);
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);

    std::string receiveBuffer1 = "GET /hello HTTP/1.1\r";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->receive(m_socket, size1);

    std::string receiveBuffer2 = "\n";
    int size2 = receiveBuffer2.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size2, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer2.data(), receiveBuffer2.data() + size2), Return(size2)));
    m_protocol->receive(m_socket, size2);
}


MATCHER_P(MatcherReceiveMessage, message, "")
{
    return (arg->getAllMetadata() == message->getAllMetadata() &&
        arg->getReceivePayload().second == message->getReceivePayload().second &&
        memcmp(arg->getReceivePayload().first, message->getReceivePayload().first, arg->getReceivePayload().second) == 0);
}


TEST_F(TestProtocolHttp, testHeaders)
{
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);

    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nhello: ";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->receive(m_socket, size1);

    std::string receiveBuffer2 = "123\r\n";
    int size2 = receiveBuffer2.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size2, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer2.data(), receiveBuffer2.data() + size2), Return(size2)));
    m_protocol->receive(m_socket, size2);

    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    message->addMetadata("fmq_http", "request");
    message->addMetadata("fmq_method", "GET");
    message->addMetadata("fmq_path", "/hello");
    message->addMetadata("fmq_query", "filter=world&lang=en");
    message->addMetadata("fmq_protocol", "HTTP/1.1");
    message->addMetadata("hello", "123");
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message))).Times(1);
    std::string receiveBuffer3 = "\r\n";
    int size3 = receiveBuffer3.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size3, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer3.data(), receiveBuffer3.data() + size3), Return(size3)));
    m_protocol->receive(m_socket, size3);
}

TEST_F(TestProtocolHttp, testPayload)
{
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);

    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    message->addMetadata("fmq_http", "request");
    message->addMetadata("fmq_method", "GET");
    message->addMetadata("fmq_path", "/hello");
    message->addMetadata("fmq_query", "filter=world&lang=en");
    message->addMetadata("fmq_protocol", "HTTP/1.1");
    message->addMetadata("Content-Length", "10");
    message->resizeReceivePayload(10);
    memcpy(message->getReceivePayload().first, "0123456789", 10);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message))).Times(1);

    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456789";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->receive(m_socket, size1);
}

TEST_F(TestProtocolHttp, testSplitPayload)
{
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);


    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->receive(m_socket, size1);

    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    message->addMetadata("fmq_http", "request");
    message->addMetadata("fmq_method", "GET");
    message->addMetadata("fmq_path", "/hello");
    message->addMetadata("fmq_query", "filter=world&lang=en");
    message->addMetadata("fmq_protocol", "HTTP/1.1");
    message->addMetadata("Content-Length", "10");
    message->resizeReceivePayload(10);
    memcpy(message->getReceivePayload().first, "0123456789", 10);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message))).Times(1);

    std::string receiveBuffer2 = "789";
    int size2 = receiveBuffer2.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size2, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer2.data(), receiveBuffer2.data() + size2), Return(size2)));
    m_protocol->receive(m_socket, size2);
}


TEST_F(TestProtocolHttp, testPayloadTooBig)
{
    EXPECT_CALL(*m_mockCallback, received(_)).Times(0);
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(1);

    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n01234567890";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->receive(m_socket, size1);
}

TEST_F(TestProtocolHttp, testSplitPayloadTooBig)
{
    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockMockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->receive(m_socket, size1);

    EXPECT_CALL(*m_mockCallback, received(_)).Times(0);
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(1);

    std::string receiveBuffer2 = "7890";
    int size2 = receiveBuffer2.size();
    m_protocol->receive(m_socket, size2);
}

