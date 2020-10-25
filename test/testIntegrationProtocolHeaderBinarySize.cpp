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


#include "protocolconnection/ProtocolSessionContainer.h"
#include "MockIProtocolSessionCallback.h"
#include "protocols/ProtocolHeaderBinarySize.h"
#include "testHelper.h"

#include <thread>
//#include <chrono>


using ::testing::_;
using ::testing::Return;
//using ::testing::InSequence;


using namespace finalmq;

static const std::string MESSAGE1_BUFFER = "Hello";



class TestIntegrationProtocolHeaderBinarySize : public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_factoryProtocol = std::make_shared<ProtocolHeaderBinarySizeFactory>();
        m_mockClientCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_mockServerCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_sessionContainer = std::make_unique<ProtocolSessionContainer>();
        m_sessionContainer->init(1, 1);
        IProtocolSessionContainer* sessionContainerRaw = m_sessionContainer.get();
        m_thread = std::make_unique<std::thread>([sessionContainerRaw] () {
            sessionContainerRaw->threadEntry();
        });
    }

    virtual void TearDown()
    {
        EXPECT_EQ(m_sessionContainer->terminatePollerLoop(100), true);
        m_sessionContainer = nullptr;
        m_thread->join();
    }

    std::shared_ptr<IProtocolSessionContainer>              m_sessionContainer;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockClientCallback;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockServerCallback;
    std::shared_ptr<IProtocolFactory>                       m_factoryProtocol;

    std::unique_ptr<std::thread>                            m_thread;

};




TEST_F(TestIntegrationProtocolHeaderBinarySize, testStartAndStopThreadIntern)
{
}




TEST_F(TestIntegrationProtocolHeaderBinarySize, testBind)
{
    int res = m_sessionContainer->bind("tcp://localhost:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);
}


TEST_F(TestIntegrationProtocolHeaderBinarySize, testUnbind)
{
    int res = m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);
    m_sessionContainer->unbind("tcp://*:3333");
}

MATCHER_P(ReceivedMessage, msg, "")
{
    BufferRef buffer = arg->getReceivePayload();
    std::string str(buffer.first, buffer.second);
    return str == msg;
}

TEST_F(TestIntegrationProtocolHeaderBinarySize, testBindConnect)
{
    int res = m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    IProtocolSessionPtr connConnect;
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333", m_mockClientCallback, std::make_shared<ProtocolHeaderBinarySize>());
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectReceive, 5000);

    EXPECT_EQ(connConnect, connection);
}



TEST_F(TestIntegrationProtocolHeaderBinarySize, testConnectBind)
{
    auto& expectConnected = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333", m_mockClientCallback, std::make_shared<ProtocolHeaderBinarySize>(), 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    int res = m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);

    waitTillDone(expectConnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, CONNECTIONSTATE_CONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), connection);
}


TEST_F(TestIntegrationProtocolHeaderBinarySize, testSendConnectBind)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333", m_mockClientCallback, std::make_shared<ProtocolHeaderBinarySize>(), 1);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    int res = m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);

    waitTillDone(expectReceive, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, CONNECTIONSTATE_CONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), connection);
}



TEST_F(TestIntegrationProtocolHeaderBinarySize, testReconnectExpires)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(0);
    auto& expectDisconnected = EXPECT_CALL(*m_mockClientCallback, disconnected(_)).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333", m_mockClientCallback, std::make_shared<ProtocolHeaderBinarySize>(), 1, 1);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectDisconnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, CONNECTIONSTATE_DISCONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), nullptr);
}




TEST_F(TestIntegrationProtocolHeaderBinarySize, testBindConnectDisconnect)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnected = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);
    EXPECT_CALL(*m_mockClientCallback, disconnected(_)).Times(1);
    auto& expectDisconnected = EXPECT_CALL(*m_mockServerCallback, disconnected(_)).Times(1);

    int res = m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333", m_mockClientCallback, std::make_shared<ProtocolHeaderBinarySize>());
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectConnected, 5000);

    connection->disconnect();

    waitTillDone(expectDisconnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, CONNECTIONSTATE_DISCONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), nullptr);
}


TEST_F(TestIntegrationProtocolHeaderBinarySize, testGetAllConnections)
{
    int res = m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);

    IProtocolSessionPtr connConnect;
    IProtocolSessionPtr connBind;
    auto& expectConnectedClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    auto& expectConnectedServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connBind));

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333", m_mockClientCallback, std::make_shared<ProtocolHeaderBinarySize>());

    waitTillDone(expectConnectedClient, 5000);
    waitTillDone(expectConnectedServer, 5000);

    EXPECT_EQ(connConnect, connection);

    std::vector< IProtocolSessionPtr > sessions = m_sessionContainer->getAllSessions();
    EXPECT_EQ(sessions.size(), 2);
    if (sessions[0] == connBind)
    {
        std::swap(sessions[0], sessions[1]);
    }
    EXPECT_EQ(sessions[0], connConnect);
    EXPECT_EQ(sessions[1], connBind);
}


TEST_F(TestIntegrationProtocolHeaderBinarySize, testSendMultipleMessages)
{
    int res = m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    EXPECT_EQ(res, 0);

    IProtocolSessionPtr connConnect;
    auto& expectConnectedClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(10000);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333", m_mockClientCallback, std::make_shared<ProtocolHeaderBinarySize>());
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);

    for (int i = 0; i < 10000; ++i)
    {
        connection->sendMessage(message);
    }
    waitTillDone(expectConnectedClient, 5000);
    EXPECT_EQ(connConnect, connection);

    waitTillDone(expectReceive, 10000);
}
