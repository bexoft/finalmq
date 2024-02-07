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

#if !defined(WIN32)

#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "finalmq/poller/PollerImplEpoll.h"
#include "finalmq/helpers/OperatingSystem.h"


#include "MockIOperatingSystem.h"
#include "matchers.h"


using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::DoAll;

using namespace finalmq;

static const std::string BUFFER = "Hello";

static const int EPOLL_FD = 3;
static const int CONTROLSOCKET_READ = 4;
static const int CONTROLSOCKET_WRITE = 5;
static const int TESTSOCKET = 7;

static const int NUMBER_OF_BYTES_TO_READ = 20;
static const int TIMEOUT = 10;




class TestEpoll: public testing::Test
{
protected:
    virtual void SetUp()
    {
        m_mockMockOperatingSystem = new MockIOperatingSystem;
        OperatingSystem::setInstance(std::unique_ptr<IOperatingSystem>(m_mockMockOperatingSystem));
        m_select = std::make_unique<PollerImplEpoll>();

        EXPECT_CALL(*m_mockMockOperatingSystem, epoll_create1(EPOLL_CLOEXEC)).Times(1)
                    .WillRepeatedly(Return(EPOLL_FD));

        SocketDescriptorPtr sd1 = std::make_shared<SocketDescriptor>(CONTROLSOCKET_READ);
        SocketDescriptorPtr sd2 = std::make_shared<SocketDescriptor>(CONTROLSOCKET_WRITE);
        EXPECT_CALL(*m_mockMockOperatingSystem, makeSocketPair(_, _)).Times(1)
                    .WillRepeatedly(DoAll(testing::SetArgReferee<0>(sd1), testing::SetArgReferee<1>(sd2), Return(0)));

        epoll_event evCtl;
        evCtl.events = EPOLLIN;
        evCtl.data.fd = CONTROLSOCKET_READ;
        EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, CONTROLSOCKET_READ, Event(&evCtl))).Times(1);

        m_select->init();
        testing::Mock::VerifyAndClearExpectations(m_mockMockOperatingSystem);
    }

    virtual void TearDown()
    {
        EXPECT_CALL(*m_mockMockOperatingSystem, close(EPOLL_FD)).Times(1).WillRepeatedly(Return(0));
        EXPECT_CALL(*m_mockMockOperatingSystem, closeSocket(_)).WillRepeatedly(Return(0));
        m_select = nullptr;
        OperatingSystem::setInstance({});
    }

    MockIOperatingSystem* m_mockMockOperatingSystem = nullptr;
    std::unique_ptr<IPoller> m_select;
};


TEST_F(TestEpoll, timeout)
{
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                .WillRepeatedly(Return(0));

    const PollerResult& result = m_select->wait(10);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, true);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}



TEST_F(TestEpoll, testAddSocketReadableWait)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(_, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    struct epoll_event events;
    events.data.fd = socket->getDescriptor();
    events.events = EPOLLIN;

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(events), Return(1)));
    EXPECT_CALL(*m_mockMockOperatingSystem, ioctlInt(socket->getDescriptor(), FIONREAD, _)).Times(1)
                                                        .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(NUMBER_OF_BYTES_TO_READ), Return(0)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    if (result.descriptorInfos.size() == 1)
    {
        EXPECT_EQ(result.descriptorInfos[0].sd, socket->getDescriptor());
        EXPECT_EQ(result.descriptorInfos[0].disconnected, false);
        EXPECT_EQ(result.descriptorInfos[0].readable, true);
        EXPECT_EQ(result.descriptorInfos[0].writable, false);
        EXPECT_EQ(result.descriptorInfos[0].bytesToRead, NUMBER_OF_BYTES_TO_READ);
    }
}


TEST_F(TestEpoll, testAddSocketReadableEINTR)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    struct epoll_event events;
    events.data.fd = socket->getDescriptor();
    events.events = EPOLLIN;

    {
        InSequence seq;

        EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(_, _, _, TIMEOUT, nullptr)).Times(1)
                    .WillRepeatedly(Return(-1));
        EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(_, _, _, TIMEOUT, nullptr)).Times(1)
                    .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(events), Return(1)));
    }
    EXPECT_CALL(*m_mockMockOperatingSystem, getLastError()).Times(1)
                        .WillOnce(Return(SOCKETERROR(EINTR)));
    EXPECT_CALL(*m_mockMockOperatingSystem, ioctlInt(socket->getDescriptor(), FIONREAD, _)).Times(1)
                                                        .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(NUMBER_OF_BYTES_TO_READ), Return(0)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    if (result.descriptorInfos.size() == 1)
    {
        EXPECT_EQ(result.descriptorInfos[0].sd, socket->getDescriptor());
        EXPECT_EQ(result.descriptorInfos[0].disconnected, false);
        EXPECT_EQ(result.descriptorInfos[0].readable, true);
        EXPECT_EQ(result.descriptorInfos[0].writable, false);
        EXPECT_EQ(result.descriptorInfos[0].bytesToRead, NUMBER_OF_BYTES_TO_READ);
    }
}




TEST_F(TestEpoll, testAddSocketReadableError)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                .WillRepeatedly(Return(-1));
    EXPECT_CALL(*m_mockMockOperatingSystem, getLastError()).Times(1)
                        .WillOnce(Return(SOCKETERROR(EACCES)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, true);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}


TEST_F(TestEpoll, testAddSocketReadableWaitSocketDescriptorsChanged)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    struct epoll_event events;
    events.data.fd = socket->getDescriptor();
    events.events = EPOLLIN;

    epoll_event evCtlRemove;
    evCtlRemove.events = 0;
    evCtlRemove.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_DEL, socket->getDescriptor(), Event(&evCtlRemove))).Times(1);

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
            .WillRepeatedly(
                testing::DoAll(
                    testing::Invoke([this, &socket](int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t* sigmask){
                        m_select->removeSocket(socket);
                    }),
                    testing::SetArgPointee<1>(events),
                    Return(1)
                )
            );
    EXPECT_CALL(*m_mockMockOperatingSystem, ioctlInt(socket->getDescriptor(), FIONREAD, _)).Times(1)
                                                        .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(NUMBER_OF_BYTES_TO_READ), Return(0)));

    EXPECT_CALL(*m_mockMockOperatingSystem, closeSocket(socket->getDescriptor())).WillRepeatedly(Return(0));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    if (result.descriptorInfos.size() == 1)
    {
        EXPECT_EQ(result.descriptorInfos[0].sd, socket->getDescriptor());
        EXPECT_EQ(result.descriptorInfos[0].disconnected, false);
        EXPECT_EQ(result.descriptorInfos[0].readable, true);
        EXPECT_EQ(result.descriptorInfos[0].writable, false);
        EXPECT_EQ(result.descriptorInfos[0].bytesToRead, NUMBER_OF_BYTES_TO_READ);
    }
}

TEST_F(TestEpoll, testAddSocketDisconnectRead)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    struct epoll_event events;
    events.data.fd = socket->getDescriptor();
    events.events = EPOLLIN;

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                                                .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(events), Return(1)));
    EXPECT_CALL(*m_mockMockOperatingSystem, ioctlInt(socket->getDescriptor(), FIONREAD, _)).Times(1)
                                                .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(0), Return(0)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    if (result.descriptorInfos.size() == 1)
    {
        EXPECT_EQ(result.descriptorInfos[0].sd, socket->getDescriptor());
        EXPECT_EQ(result.descriptorInfos[0].disconnected, false);
        EXPECT_EQ(result.descriptorInfos[0].readable, true);
        EXPECT_EQ(result.descriptorInfos[0].writable, false);
        EXPECT_EQ(result.descriptorInfos[0].bytesToRead, 0);
    }
}



TEST_F(TestEpoll, testAddSocketDisconnectEpollError)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    struct epoll_event events;
    events.data.fd = socket->getDescriptor();
    events.events = EPOLLERR;

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                                                .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(events), Return(1)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    if (result.descriptorInfos.size() == 1)
    {
        EXPECT_EQ(result.descriptorInfos[0].sd, socket->getDescriptor());
        EXPECT_EQ(result.descriptorInfos[0].disconnected, true);
        EXPECT_EQ(result.descriptorInfos[0].readable, false);
        EXPECT_EQ(result.descriptorInfos[0].writable, false);
        EXPECT_EQ(result.descriptorInfos[0].bytesToRead, 0);
    }
}

TEST_F(TestEpoll, testAddSocketIoCtlError)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    struct epoll_event events;
    events.data.fd = socket->getDescriptor();
    events.events = EPOLLIN;

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                                                .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(events), Return(1)));
    EXPECT_CALL(*m_mockMockOperatingSystem, ioctlInt(socket->getDescriptor(), FIONREAD, _)).Times(1)
                                                .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(0), Return(-1)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    if (result.descriptorInfos.size() == 1)
    {
        EXPECT_EQ(result.descriptorInfos[0].sd, socket->getDescriptor());
        EXPECT_EQ(result.descriptorInfos[0].disconnected, false);
        EXPECT_EQ(result.descriptorInfos[0].readable, true);
        EXPECT_EQ(result.descriptorInfos[0].writable, false);
        EXPECT_EQ(result.descriptorInfos[0].bytesToRead, 0);
    }
}


TEST_F(TestEpoll, testAddSocketWritableWait)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    epoll_event evCtlWrite;
    evCtlWrite.events = EPOLLIN | EPOLLOUT;
    evCtlWrite.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_MOD, socket->getDescriptor(), Event(&evCtlWrite))).Times(1);

    m_select->enableWrite(socket);

    struct epoll_event events;
    events.data.fd = socket->getDescriptor();
    events.events = EPOLLOUT;

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                                                .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(events), Return(1)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    if (result.descriptorInfos.size() == 1)
    {
        EXPECT_EQ(result.descriptorInfos[0].sd, socket->getDescriptor());
        EXPECT_EQ(result.descriptorInfos[0].disconnected, false);
        EXPECT_EQ(result.descriptorInfos[0].readable, false);
        EXPECT_EQ(result.descriptorInfos[0].writable, true);
        EXPECT_EQ(result.descriptorInfos[0].bytesToRead, 0);
    }
}



TEST_F(TestEpoll, testAddSocketDisableWritableWait)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    epoll_event evCtl;
    evCtl.events = EPOLLIN;
    evCtl.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, socket->getDescriptor(), Event(&evCtl))).Times(1);

    m_select->addSocketEnableRead(socket);

    epoll_event evCtlWrite;
    evCtlWrite.events = EPOLLIN | EPOLLOUT;
    evCtlWrite.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_MOD, socket->getDescriptor(), Event(&evCtlWrite))).Times(1);

    m_select->enableWrite(socket);

    epoll_event evCtlDisableWrite;
    evCtlDisableWrite.events = EPOLLIN;
    evCtlDisableWrite.data.fd = socket->getDescriptor();
    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_ctl(EPOLL_FD, EPOLL_CTL_MOD, socket->getDescriptor(), Event(&evCtlDisableWrite))).Times(1);

    m_select->disableWrite(socket);

    EXPECT_CALL(*m_mockMockOperatingSystem, epoll_pwait(EPOLL_FD, _, _, TIMEOUT, nullptr)).Times(1)
                                                .WillRepeatedly(Return(0));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, true);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}


#endif
