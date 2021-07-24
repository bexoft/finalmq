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
#include "finalmq/protocolsession/ProtocolDelimiter.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "testHelper.h"

#include <thread>
//#include <chrono>


using ::testing::_;
using ::testing::Return;
//using ::testing::InSequence;


using namespace finalmq;

static const std::string MESSAGE1_BUFFER = "Hello";
static const std::string MESSAGE2_BUFFER(500000, 'A');
static const std::string DELIMITER = "lolololololololololololololololololololololololololololololololololo\n";



class ProtocolDelimiterTestLong : public ProtocolDelimiter
{
public:
    static const int PROTOCOL_ID;
    static const std::string PROTOCOL_NAME;

    ProtocolDelimiterTestLong()
        : ProtocolDelimiter(DELIMITER)
    {
    }
private:
    // IProtocol
    virtual std::uint32_t getProtocolId() const
    {
        return PROTOCOL_ID;   // not a real protocol, just for the test
    }
};
class ProtocolDelimiterTestLongFactory : public IProtocolFactory
{
private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol() override
    {
        return std::make_shared<ProtocolDelimiterTestLong>();
    }
};
const int ProtocolDelimiterTestLong::PROTOCOL_ID = 12345;
const std::string ProtocolDelimiterTestLong::PROTOCOL_NAME = "delimiter_long";
struct RegisterProtocolDelimiterTestLongFactory
{
    RegisterProtocolDelimiterTestLongFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolDelimiterTestLong::PROTOCOL_NAME, ProtocolDelimiterTestLong::PROTOCOL_ID, std::make_shared<ProtocolDelimiterTestLongFactory>());
    }
} g_registerProtocolDelimiterTestLongFactory;





MATCHER_P(ReceivedMessage, msg, "")
{
    BufferRef buffer = arg->getReceivePayload();
    std::string str(buffer.first, buffer.second);
    return str == msg;
}


class TestIntegrationProtocolDelimiterSessionContainer: public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_mockClientCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_mockServerCallback = std::make_shared<MockIProtocolSessionCallback>();
        m_sessionContainer = std::make_unique<ProtocolSessionContainer>();
        m_sessionContainer->init(nullptr, 1);
        IProtocolSessionContainer* sessionContainerRaw = m_sessionContainer.get();
        m_thread = std::make_unique<std::thread>([sessionContainerRaw] () {
            sessionContainerRaw->run();
        });
    }

    virtual void TearDown()
    {
        m_sessionContainer->terminatePollerLoop();
        m_thread->join();
        m_sessionContainer = nullptr;
    }

    std::shared_ptr<IProtocolSessionContainer>              m_sessionContainer;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockClientCallback;
    std::shared_ptr<MockIProtocolSessionCallback>           m_mockServerCallback;

    std::unique_ptr<std::thread>                            m_thread;
};




TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testStartAndStopThreadIntern)
{
}




TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testBind)
{
    int res = m_sessionContainer->bind("tcp://localhost:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);
}


TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testUnbind)
{
    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);
    m_sessionContainer->unbind("tcp://*:3333:delimiter_long");
}


TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testBindConnect)
{
    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    IProtocolSessionPtr connConnect;
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectReceive, 5000);

    EXPECT_EQ(connConnect, connection);
}



TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testConnectBind)
{
    auto& expectConnected = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback, {{},1});

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    waitTillDone(expectConnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, ConnectionState::CONNECTIONSTATE_CONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), connection);
}


TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testSendConnectBind)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    auto& expectReceive = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback, {{},1});
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    waitTillDone(expectReceive, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, ConnectionState::CONNECTIONSTATE_CONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), connection);
}



TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testReconnectExpires)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(0);
    auto& expectDisconnected = EXPECT_CALL(*m_mockClientCallback, disconnected(_)).Times(1);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback, {{},1, 1});
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectDisconnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, ConnectionState::CONNECTIONSTATE_DISCONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), nullptr);
}




TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testBindConnectDisconnect)
{
    EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1);
    auto& expectConnected = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1);
    EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1);
    EXPECT_CALL(*m_mockClientCallback, disconnected(_)).Times(1);
    auto& expectDisconnected = EXPECT_CALL(*m_mockServerCallback, disconnected(_)).Times(1);

    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);
    connection->sendMessage(message);

    waitTillDone(expectConnected, 5000);

    connection->disconnect();

    waitTillDone(expectDisconnected, 5000);

    EXPECT_EQ(connection->getConnectionData().connectionState, ConnectionState::CONNECTIONSTATE_DISCONNECTED);
    EXPECT_EQ(m_sessionContainer->getSession(connection->getSessionId()), nullptr);
}


TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testGetAllConnections)
{
    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    IProtocolSessionPtr connConnect;
    IProtocolSessionPtr connBind;
    auto& expectConnectedClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    auto& expectConnectedServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connBind));

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback);

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


TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testSendMultipleMessages)
{
    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    IProtocolSessionPtr connConnect;
    IProtocolSessionPtr bindConnect;
    auto& expectConnectedClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    auto& expectConnectedServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&bindConnect));
    auto& expectReceiveClient = EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1000);
    auto& expectReceiveServer = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE1_BUFFER))).Times(1000);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE1_BUFFER);

    for (int i = 0; i < 1000; ++i)
    {
        connection->sendMessage(message);
    }

    waitTillDone(expectConnectedClient, 5000);
    waitTillDone(expectConnectedServer, 5000);

    for (int i = 0; i < 1000; ++i)
    {
        bindConnect->sendMessage(message);
    }

    EXPECT_EQ(connConnect, connection);

    waitTillDone(expectReceiveClient, 10000);
    waitTillDone(expectReceiveServer, 10000);
}


TEST_F(TestIntegrationProtocolDelimiterSessionContainer, testSendBigMultipleMessages)
{
    int res = m_sessionContainer->bind("tcp://*:3333:delimiter_long", m_mockServerCallback);
    EXPECT_EQ(res, 0);

    IProtocolSessionPtr connConnect;
    IProtocolSessionPtr bindConnect;
    auto& expectConnectedClient = EXPECT_CALL(*m_mockClientCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&connConnect));
    auto& expectConnectedServer = EXPECT_CALL(*m_mockServerCallback, connected(_)).Times(1)
                                            .WillOnce(testing::SaveArg<0>(&bindConnect));
    auto& expectReceiveClient = EXPECT_CALL(*m_mockClientCallback, received(_, ReceivedMessage(MESSAGE2_BUFFER))).Times(100);
    auto& expectReceiveServer = EXPECT_CALL(*m_mockServerCallback, received(_, ReceivedMessage(MESSAGE2_BUFFER))).Times(100);

    IProtocolSessionPtr connection = m_sessionContainer->connect("tcp://localhost:3333:delimiter_long", m_mockClientCallback);
    IMessagePtr message = connection->createMessage();
    message->addSendPayload(MESSAGE2_BUFFER);

    for (int i = 0; i < 100; ++i)
    {
        connection->sendMessage(message);
    }

    waitTillDone(expectConnectedClient, 5000);
    waitTillDone(expectConnectedServer, 5000);

    for (int i = 0; i < 100; ++i)
    {
        bindConnect->sendMessage(message);
    }

    EXPECT_EQ(connConnect, connection);

    waitTillDone(expectReceiveClient, 10000);
    waitTillDone(expectReceiveServer, 10000);
}
