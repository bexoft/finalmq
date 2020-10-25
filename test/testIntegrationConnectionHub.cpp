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


#include "protocolconnection/ProtocolSessionContainer.h"
#include "MockIProtocolSessionCallback.h"
#include "protocols/ProtocolDelimiter.h"
#include "protocols/ProtocolStream.h"
#include "testHelper.h"
#include "connectionhub/ConnectionHub.h"

#include <thread>
#include <chrono>

using ::testing::_;
using ::testing::Return;

using namespace std::chrono_literals;
using namespace finalmq;

static const std::string MESSAGE_BUFFER = "Hello";
static const std::string DELIMITER = "\n";

MATCHER_P(ReceivedMessage, msg, "")
{
    BufferRef buffer = arg->getReceivePayload();
    std::string str(buffer.first, buffer.second);
    return str == msg;
}




class TestIntegrationConnectionHub: public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_factoryProtocol = std::make_shared<ProtocolDelimiterFactory>(DELIMITER);
        m_mockClientCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_mockServerCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_sessionContainer = std::make_unique<ProtocolSessionContainer>();
        m_sessionContainer->init(1, 1);
        IProtocolSessionContainer* sessionContainerRaw = m_sessionContainer.get();
        m_threadSessionContainer = std::make_unique<std::thread>([sessionContainerRaw] () {
            sessionContainerRaw->threadEntry();
        });
        m_connectionHub = std::make_unique<ConnectionHub>();
        m_connectionHub->init(1, 1);
        IConnectionHub* connectionHubRaw = m_connectionHub.get();
        m_threadConnectionHub = std::make_unique<std::thread>([connectionHubRaw] () {
            connectionHubRaw->threadEntry();
        });
    }

    virtual void TearDown()
    {
        EXPECT_EQ(m_sessionContainer->terminatePollerLoop(100), true);
        m_sessionContainer = nullptr;
        m_threadSessionContainer->join();

        EXPECT_EQ(m_connectionHub->waitForTerminationOfPollerLoop(100), true);
        m_connectionHub = nullptr;
        m_threadConnectionHub->join();
    }

    std::shared_ptr<IProtocolSessionContainer>              m_sessionContainer;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockClientCallback;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockServerCallback;
    std::shared_ptr<IProtocolFactory>                       m_factoryProtocol;
    std::unique_ptr<IConnectionHub>                         m_connectionHub;


    std::unique_ptr<std::thread>                            m_threadSessionContainer;
    std::unique_ptr<std::thread>                            m_threadConnectionHub;
};



TEST_F(TestIntegrationConnectionHub, testStartAndStopThreadIntern)
{
}


TEST_F(TestIntegrationConnectionHub, testConnect)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);
}

TEST_F(TestIntegrationConnectionHub, testUnbind)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);

    m_connectionHub->unbind("tcp://*:3334");
}



TEST_F(TestIntegrationConnectionHub, testGetAllSessions)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);

    std::vector< IProtocolSessionPtr > sessions = m_connectionHub->getAllSessions();
    EXPECT_EQ(sessions.size(), 2);
}


TEST_F(TestIntegrationConnectionHub, testGetSession)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);

    std::vector< IProtocolSessionPtr > sessions = m_connectionHub->getAllSessions();
    EXPECT_EQ(sessions.size(), 2);
    for (size_t i = 0; i < sessions.size(); ++i)
    {
        EXPECT_EQ(m_connectionHub->getSession(sessions[i]->getSessionId()), sessions[i]);
    }
}


TEST_F(TestIntegrationConnectionHub, testForwardLateMessage)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    IProtocolSessionPtr bindSession;
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                                .WillOnce(testing::SaveArg<0>(&bindSession));

    m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    auto& expectReceiveClient = EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(MESSAGE_BUFFER))).Times(1);
    auto& expectReceiveServer = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE_BUFFER))).Times(1);
    m_connectionHub->startMessageForwarding();

    IMessagePtr message = session->createMessage();
    message->addSendPayload(MESSAGE_BUFFER);
    session->sendMessage(message);
    waitTillDone(expectConnectServer, 5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    bindSession->sendMessage(message);

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectReceiveClient, 5000);
    waitTillDone(expectReceiveServer, 5000);
}

TEST_F(TestIntegrationConnectionHub, testForwardEarlyMessage)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    IMessagePtr message = session->createMessage();
    message->addSendPayload(MESSAGE_BUFFER);
    session->sendMessage(message);

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE_BUFFER))).Times(1);
    m_connectionHub->startMessageForwarding();

    waitTillDone(expectReceive, 5000);
}



TEST_F(TestIntegrationConnectionHub, testStopForwardingToSession)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    IProtocolSessionPtr bindSession;
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                                .WillOnce(testing::SaveArg<0>(&bindSession));

    IProtocolSessionPtr sessionForward = m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    IMessagePtr message = session->createMessage();
    message->addSendPayload(MESSAGE_BUFFER);
    session->sendMessage(message);

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE_BUFFER))).Times(1);
    m_connectionHub->startMessageForwarding();

    waitTillDone(expectReceive, 5000);

    testing::Mock::VerifyAndClearExpectations(m_mockServerCallback.get());

    EXPECT_CALL(*m_mockServerCallback, received(_, _)).Times(0);
    auto& expectReceiveClient = EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(MESSAGE_BUFFER))).Times(1);
    m_connectionHub->stopForwardingToSession(sessionForward->getSessionId());
    session->sendMessage(message);
    bindSession->sendMessage(message);

    waitTillDone(expectReceiveClient, 5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}


TEST_F(TestIntegrationConnectionHub, testStopForwardingFromSession)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    IProtocolSessionPtr bindSession;
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                                .WillOnce(testing::SaveArg<0>(&bindSession));

    IProtocolSessionPtr sessionForward = m_connectionHub->connect("tcp://localhost:3333", std::make_shared<ProtocolStream>(), 1);
    m_sessionContainer->bind("tcp://*:3333", m_mockServerCallback, m_factoryProtocol);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334", m_mockClientCallback, std::make_shared<ProtocolDelimiter>(DELIMITER), 1);
    m_connectionHub->bind("tcp://*:3334", std::make_shared<ProtocolStreamFactory>());

    IMessagePtr message = session->createMessage();
    message->addSendPayload(MESSAGE_BUFFER);
    session->sendMessage(message);

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE_BUFFER))).Times(1);
    m_connectionHub->startMessageForwarding();

    waitTillDone(expectReceive, 5000);

    testing::Mock::VerifyAndClearExpectations(m_mockServerCallback.get());

    EXPECT_CALL(*m_mockClientCallback, received(_, _)).Times(0);
    auto& expectReceiveServer = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE_BUFFER))).Times(1);
    m_connectionHub->stopForwardingFromSession(sessionForward->getSessionId());
    session->sendMessage(message);
    bindSession->sendMessage(message);

    waitTillDone(expectReceiveServer, 5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

