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


#include "finalmq/protocolsession/ProtocolSessionContainer.h"
#include "MockIProtocolSessionCallback.h"
#include "finalmq/protocols/ProtocolDelimiterLinefeed.h"
#include "finalmq/protocols/ProtocolStream.h"
#include "testHelper.h"
#include "finalmq/connectionhub/ConnectionHub.h"

#include <thread>
#include <chrono>

using ::testing::_;
using ::testing::Return;

using namespace std::chrono_literals;
using namespace finalmq;

static const std::string MESSAGE_BUFFER = "Hello";

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
        m_mockClientCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_mockServerCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_sessionContainer = std::make_unique<ProtocolSessionContainer>();
        m_sessionContainer->init(nullptr, 1, nullptr, 1);
        IProtocolSessionContainer* sessionContainerRaw = m_sessionContainer.get();
        m_threadSessionContainer = std::make_unique<std::thread>([sessionContainerRaw] () {
            sessionContainerRaw->run();
        });
        m_connectionHub = std::make_unique<ConnectionHub>();
        m_connectionHub->init(1, 1);
        IConnectionHub* connectionHubRaw = m_connectionHub.get();
        m_threadConnectionHub = std::make_unique<std::thread>([connectionHubRaw] () {
            connectionHubRaw->run();
        });
    }

    virtual void TearDown()
    {
        EXPECT_CALL(*m_mockClientCallback, disconnected(_)).WillRepeatedly(Return());
        EXPECT_CALL(*m_mockServerCallback, disconnected(_)).WillRepeatedly(Return());

        m_sessionContainer->terminatePollerLoop();
        m_threadSessionContainer->join();
        m_sessionContainer = nullptr;

        m_connectionHub->waitForTerminationOfPollerLoop();
        m_threadConnectionHub->join();
        m_connectionHub = nullptr;
    }

    std::shared_ptr<IProtocolSessionContainer>              m_sessionContainer;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockClientCallback;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockServerCallback;
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

    m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);
}

TEST_F(TestIntegrationConnectionHub, testUnbind)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);

    m_connectionHub->unbind("tcp://*:3334:stream");
}



TEST_F(TestIntegrationConnectionHub, testGetAllSessions)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

    waitTillDone(expectConnectClient, 5000);
    waitTillDone(expectConnectServer, 5000);

    std::vector< IProtocolSessionPtr > sessions = m_connectionHub->getAllSessions();
    EXPECT_EQ(sessions.size(), 2);
}


TEST_F(TestIntegrationConnectionHub, testGetSession)
{
    auto& expectConnectClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnectServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

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

    m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

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

    m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

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

    IProtocolSessionPtr sessionForward = m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

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

    IProtocolSessionPtr sessionForward = m_connectionHub->connect("tcp://localhost:3333:stream", { {},{1} });
    m_sessionContainer->bind("tcp://*:3333:delimiter_lf", m_mockServerCallback);
    IProtocolSessionPtr session = m_sessionContainer->connect("tcp://localhost:3334:delimiter_lf", m_mockClientCallback, { {},{1} });
    m_connectionHub->bind("tcp://*:3334:stream");

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

