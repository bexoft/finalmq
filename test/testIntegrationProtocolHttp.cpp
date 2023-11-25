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


#include "finalmq/protocolsession/ProtocolSessionContainer.h"
#include "MockIProtocolSessionCallback.h"
#include "testHelper.h"

#include <thread>
//#include <chrono>


using ::testing::_;
using ::testing::Return;
using ::testing::Between;
using ::testing::Invoke;
//using ::testing::InSequence;


using namespace finalmq;

static const std::string MESSAGE1_BUFFER = "Hello";



class TestIntegrationProtocolHttp : public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_mockClientCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_mockServerCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_sessionContainer = std::make_unique<ProtocolSessionContainer>();
        m_sessionContainer->init(nullptr, 1, nullptr, 1);
        IProtocolSessionContainer* sessionContainerRaw = m_sessionContainer.get();
        m_thread = std::make_unique<std::thread>([sessionContainerRaw] () {
            sessionContainerRaw->run();
        });
    }

    virtual void TearDown()
    {
        EXPECT_CALL(*m_mockClientCallback, disconnected(_)).WillRepeatedly(Return());
        EXPECT_CALL(*m_mockServerCallback, disconnected(_)).WillRepeatedly(Return());

        m_sessionContainer->terminatePollerLoop();
        m_thread->join();
        m_sessionContainer = nullptr;
    }

    std::shared_ptr<IProtocolSessionContainer>              m_sessionContainer;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockClientCallback;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockServerCallback;

    std::unique_ptr<std::thread>                            m_thread;

};




TEST_F(TestIntegrationProtocolHttp, testStartAndStopThreadIntern)
{
}


TEST_F(TestIntegrationProtocolHttp, testBind)
{
    int res = m_sessionContainer->bind("tcp://localhost:3335:httpserver", m_mockServerCallback);
    EXPECT_EQ(res, 0);
}

TEST_F(TestIntegrationProtocolHttp, testUnbind)
{
    int res = m_sessionContainer->bind("tcp://*:3335:httpserver", m_mockServerCallback);
    EXPECT_EQ(res, 0);
    m_sessionContainer->unbind("tcp://*:3335:httpclient");
}


MATCHER_P(ReceivedMessage, msg, "")
{
    BufferRef buffer = arg->getReceivePayload();
    std::string str(buffer.first, buffer.second);
    return str == msg;
}

TEST_F(TestIntegrationProtocolHttp, testBindConnect)
{
    int res = m_sessionContainer->bind("tcp://*:3335:httpserver", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    IProtocolSessionPtr connConnect;
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);

    // the disconnect response could come
    EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(""))).Times(Between(0, 1));

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3335:httpclient", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectReceive, 5000);

    EXPECT_EQ(connConnect, connection);
}

TEST_F(TestIntegrationProtocolHttp, testConnectBindSend)
{
    auto& expectConnected = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, received(_, _)).Times(1);

    // the disconnect response could come
    EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(""))).Times(Between(0, 1));

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3335:httpclient", m_mockClientCallback, { {}, {1} });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    int res = m_sessionContainer->bind("tcp://*:3335:httpserver", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectConnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, ConnectionState::CONNECTIONSTATE_CONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), connection);
}



TEST_F(TestIntegrationProtocolHttp, testReconnectExpires)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(0);
    EXPECT_CALL(*m_mockClientCallback, disconnected(_)).Times(0);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3335:httpclient", m_mockClientCallback, { {}, {1, 1} });
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    for (int i = 0; i < 5000 / 10; ++i)
    {
        if (connection->getConnectionData().connectionState == ConnectionState::CONNECTIONSTATE_DISCONNECTED)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    EXPECT_EQ(connection->getConnectionData().connectionState, ConnectionState::CONNECTIONSTATE_DISCONNECTED);
    EXPECT_NE(m_sessionContainer->getSession(connection->getSessionId()), nullptr);
}



TEST_F(TestIntegrationProtocolHttp, testBindConnectDisconnect)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnected = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);
    auto& expectDisconnected = EXPECT_CALL(*m_mockClientCallback, disconnected(_)).Times(1);

    // the disconnect response could come
    EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(""))).Times(Between(0, 1));

    int res = m_sessionContainer->bind("tcp://*:3335:httpserver", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3335:httpclient", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectConnected, 5000);

    connection->disconnect();

    waitTillDone(expectDisconnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, ConnectionState::CONNECTIONSTATE_DISCONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), nullptr);
}


TEST_F(TestIntegrationProtocolHttp, testGetAllConnections)
{
    int res = m_sessionContainer->bind("tcp://*:3335:httpserver", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    IProtocolSessionPtr connConnect;
    IProtocolSessionPtr connBind;
    auto& expectConnectedClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    auto& expectConnectedServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connBind));
    EXPECT_CALL(*m_mockServerCallback, received(_, _)).Times(1);

    // the disconnect response could come
    EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(""))).Times(Between(0, 1));

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3335:httpclient", m_mockClientCallback);

    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

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


TEST_F(TestIntegrationProtocolHttp, testSendMultipleMessages)
{
    int res = m_sessionContainer->bind("tcp://*:3335:httpserver", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    auto& expectConnectedClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(testing::Between(6, 10000));
    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER)))
        .Times(10000)
        .WillRepeatedly(Invoke([](const IProtocolSessionPtr& session, const IMessagePtr& message) {
            session->sendMessage(message, true);
        })
    );

    auto& expectReceivedClient = EXPECT_CALL(*m_mockClientCallback, received(_, _)).Times(10000);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3335:httpclient", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);

    for (int i = 0; i < 10000; ++i)
    {
        connection->sendMessage(message);
    }
    waitTillDone(expectConnectedClient, 5000);
    waitTillDone(expectReceive, 10000);
    waitTillDone(expectReceivedClient, 5000);
}

TEST_F(TestIntegrationProtocolHttp, testCookie)
{
    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://www.ibm.com:80:httpclient", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();

    connection->sendMessage(message);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
