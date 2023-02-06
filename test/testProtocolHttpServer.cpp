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

#include "finalmq/protocols/ProtocolHttpServer.h"
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



class TestProtocolHttpServer: public testing::Test
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

        m_protocol = std::make_shared<ProtocolHttpServer>();
        EXPECT_CALL(*m_mockCallback, setActivityTimeout(_));
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



TEST_F(TestProtocolHttpServer, testReceiveFirstLineIncomplete)
{
    EXPECT_CALL(*m_mockCallback, received(_, _)).Times(0);
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);
    std::string receiveBuffer = "GET /hello HTTP/1.1\r";
    int size = receiveBuffer.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer.data(), receiveBuffer.data()+size), Return(size)));
    m_protocol->received(nullptr, m_socket, size);
}


TEST_F(TestProtocolHttpServer, testReceiveFirstLineComplete)
{
    EXPECT_CALL(*m_mockCallback, received(_, _)).Times(0);
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);

    std::string receiveBuffer1 = "GET /hello HTTP/1.1\r";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->received(nullptr, m_socket, size1);

    std::string receiveBuffer2 = "\n";
    int size2 = receiveBuffer2.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size2, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer2.data(), receiveBuffer2.data() + size2), Return(size2)));
    m_protocol->received(nullptr, m_socket, size2);
}



TEST_F(TestProtocolHttpServer, testReceiveHeaders)
{
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);

    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nhello: ";
    int size1 = receiveBuffer1.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    m_protocol->received(nullptr, m_socket, size1);

    std::string receiveBuffer2 = "123\r\n";
    int size2 = receiveBuffer2.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size2, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer2.data(), receiveBuffer2.data() + size2), Return(size2)));
    m_protocol->received(nullptr, m_socket, size2);

    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    IMessage::Metainfo& metainfo = message->getAllMetainfo();
    metainfo[ProtocolHttpServer::FMQ_HTTP] = "request";
    metainfo[ProtocolHttpServer::FMQ_METHOD] = "GET";
    metainfo[ProtocolHttpServer::FMQ_PATH] = "/hello";
    metainfo[ProtocolHttpServer::FMQ_QUERY_PREFIX + "filter"] = "world";
    metainfo[ProtocolHttpServer::FMQ_QUERY_PREFIX + "lang"] = "en";
    metainfo[ProtocolHttpServer::FMQ_PROTOCOL] = "HTTP/1.1";

    message->addMetainfo("hello", "123");
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message), _)).Times(1);
    std::string receiveBuffer3 = "\r\n";
    int size3 = receiveBuffer3.size();
    m_protocol->setConnection(m_mockStreamConnection);
    EXPECT_CALL(*m_mockCallback, setSessionName(_, _, _)).Times(1);
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size3, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer3.data(), receiveBuffer3.data() + size3), Return(size3)));
    m_protocol->received(nullptr, m_socket, size3);
}

TEST_F(TestProtocolHttpServer, testReceivePayload)
{
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);

    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    IMessage::Metainfo& metainfo = message->getAllMetainfo();
    metainfo[ProtocolHttpServer::FMQ_HTTP] = "request";
    metainfo[ProtocolHttpServer::FMQ_METHOD] = "GET";
    metainfo[ProtocolHttpServer::FMQ_PATH] = "/hello";
    metainfo[ProtocolHttpServer::FMQ_QUERY_PREFIX + "filter"] = "world";
    metainfo[ProtocolHttpServer::FMQ_QUERY_PREFIX + "lang"] = "en";
    metainfo[ProtocolHttpServer::FMQ_PROTOCOL] = "HTTP/1.1";

    message->addMetainfo("Content-Length", "10");
    message->resizeReceiveBuffer(10);
    memcpy(message->getReceivePayload().first, "0123456789", 10);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message), _)).Times(1);

    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456789";
    int size1 = receiveBuffer1.size();
    m_protocol->setConnection(m_mockStreamConnection);
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    EXPECT_CALL(*m_mockCallback, setSessionName(_, _, _)).Times(1);
    m_protocol->received(nullptr, m_socket, size1);
}

TEST_F(TestProtocolHttpServer, testReceiveSplitPayload)
{
    EXPECT_CALL(*m_mockCallback, disconnected()).Times(0);


    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456";
    int size1 = receiveBuffer1.size();
    m_protocol->setConnection(m_mockStreamConnection);
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    EXPECT_CALL(*m_mockCallback, setSessionName(_, _, _)).Times(1);
    m_protocol->received(nullptr, m_socket, size1);

    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    IMessage::Metainfo& metainfo = message->getAllMetainfo();
    metainfo[ProtocolHttpServer::FMQ_HTTP] = "request";
    metainfo[ProtocolHttpServer::FMQ_METHOD] = "GET";
    metainfo[ProtocolHttpServer::FMQ_PATH] = "/hello";
    metainfo[ProtocolHttpServer::FMQ_QUERY_PREFIX + "filter"] = "world";
    metainfo[ProtocolHttpServer::FMQ_QUERY_PREFIX + "lang"] = "en";
    metainfo[ProtocolHttpServer::FMQ_PROTOCOL] = "HTTP/1.1";

    message->addMetainfo("Content-Length", "10");
    message->resizeReceiveBuffer(10);
    memcpy(message->getReceivePayload().first, "0123456789", 10);
    EXPECT_CALL(*m_mockCallback, received(MatcherReceiveMessage(message), _)).Times(1);

    std::string receiveBuffer2 = "789";
    int size2 = receiveBuffer2.size();
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size2, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer2.data(), receiveBuffer2.data() + size2), Return(size2)));
    m_protocol->received(nullptr, m_socket, size2);
}


TEST_F(TestProtocolHttpServer, testReceivePayloadTooBig)
{
    EXPECT_CALL(*m_mockCallback, received(_, _)).Times(0);

    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n01234567890";
    int size1 = receiveBuffer1.size();
    m_protocol->setConnection(m_mockStreamConnection);
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    bool ok = m_protocol->received(nullptr, m_socket, size1);
    ASSERT_EQ(ok, false);
}

TEST_F(TestProtocolHttpServer, testReceiveSplitPayloadTooBig)
{
    std::string receiveBuffer1 = "GET /hello?filter=world&lang=en HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456";
    int size1 = receiveBuffer1.size();
    m_protocol->setConnection(m_mockStreamConnection);
    EXPECT_CALL(*m_mockOperatingSystem, recv(_, _, size1, 0)).Times(1).WillOnce(DoAll(SetArrayArgument<1>(receiveBuffer1.data(), receiveBuffer1.data() + size1), Return(size1)));
    bool ok = m_protocol->received(nullptr, m_socket, size1);
    ASSERT_EQ(ok, true);

    EXPECT_CALL(*m_mockCallback, received(_, _)).Times(0);

    std::string receiveBuffer2 = "7890";
    int size2 = receiveBuffer2.size();
    ok = m_protocol->received(nullptr, m_socket, size2);
    ASSERT_EQ(ok, false);
}


TEST_F(TestProtocolHttpServer, testSendMessage)
{
    std::shared_ptr<IMessage> message = std::make_shared<ProtocolMessage>(0);
    Variant& controlData = message->getControlData();
    controlData.add(ProtocolHttpServer::FMQ_HTTP, std::string("request"));
    controlData.add(ProtocolHttpServer::FMQ_METHOD, std::string("GET"));
    controlData.add(ProtocolHttpServer::FMQ_PATH, std::string("/hello"));
    controlData.add("queries", VariantStruct{ {"filter", std::string("world")},{"lang", std::string("en")} });
        
    message->addSendPayload("0123456789");

    EXPECT_CALL(*m_mockStreamConnection, sendMessage(message));
    m_protocol->setConnection(m_mockStreamConnection);
    m_protocol->sendMessage(message);

    const std::list<BufferRef>& buffers = message->getAllSendBuffers();
    ASSERT_EQ(buffers.size(), 2);
    auto it1 = buffers.begin();
    auto it2 = it1;
    ++it2;

    static const std::string HEADERS = "GET /hello?filter=world&lang=en HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 10\r\n\r\n";
    static const std::string PAYLOAD = "0123456789";

    ASSERT_EQ(it1->second, HEADERS.size());
    ASSERT_EQ(it2->second, PAYLOAD.size());
    ASSERT_EQ(memcmp(it1->first, HEADERS.data(), HEADERS.size()), 0);
    ASSERT_EQ(memcmp(it2->first, PAYLOAD.data(), PAYLOAD.size()), 0);
}

