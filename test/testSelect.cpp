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


#include "poller/PollerImplSelect.h"
#include "helpers/OperatingSystem.h"


#include "MockIOperatingSystem.h"


using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::DoAll;

using namespace finalmq;

static const std::string BUFFER = "Hello";

static const int CONTROLSOCKET_READ = 4;
static const int CONTROLSOCKET_WRITE = 5;
static const int TESTSOCKET = 7;

static const int NUMBER_OF_BYTES_TO_READ = 20;
static const int MILLITOMICRO = 1000;
static const int TIMEOUT = 10;

MATCHER_P(FdSet, fds, "")
{
    return (memcmp(arg, fds, sizeof(*fds)) == 0);
}

MATCHER_P(Time, tim, "")
{
    return (memcmp(arg, tim, sizeof(*tim)) == 0);
}


class TestSelect: public testing::Test
{
protected:
    virtual void SetUp()
    {
        m_mockMockOperatingSystem = new MockIOperatingSystem;
        std::unique_ptr<IOperatingSystem> iOperatingSystem(m_mockMockOperatingSystem);
        OperatingSystem::setInstance(iOperatingSystem);
        m_select = std::make_unique<PollerImplSelect>();

        SocketDescriptorPtr sd1 = std::make_shared<SocketDescriptor>(CONTROLSOCKET_READ);
        SocketDescriptorPtr sd2 = std::make_shared<SocketDescriptor>(CONTROLSOCKET_WRITE);
        EXPECT_CALL(*m_mockMockOperatingSystem, makeSocketPair(_, _)).Times(1)
                    .WillRepeatedly(DoAll(testing::SetArgReferee<0>(sd1), testing::SetArgReferee<1>(sd2), Return(0)));
        EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                            .WillRepeatedly(Return(1));
        m_select->init();
        testing::Mock::VerifyAndClearExpectations(m_mockMockOperatingSystem);
    }

    virtual void TearDown()
    {
        testing::Mock::VerifyAndClearExpectations(m_mockMockOperatingSystem);
        EXPECT_CALL(*m_mockMockOperatingSystem, closeSocket(_)).WillRepeatedly(Return(0));
        m_select = nullptr;
        std::unique_ptr<IOperatingSystem> resetOperatingSystem;
        OperatingSystem::setInstance(resetOperatingSystem);
    }

    MockIOperatingSystem* m_mockMockOperatingSystem = nullptr;
    std::unique_ptr<IPoller> m_select;
};



TEST_F(TestSelect, timeout)
{
    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    EXPECT_CALL(*m_mockMockOperatingSystem, select(CONTROLSOCKET_READ + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                .WillRepeatedly(Return(0));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, true);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}


TEST_F(TestSelect, testAddSocketReadableWait)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));

    m_select->addSocketEnableRead(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    FD_SET(socket->getDescriptor(), &fdsRead);

    EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                                                                  .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(fdsRead), Return(1)));
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


TEST_F(TestSelect, testAddSocketReadableEINTR)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));
    m_select->addSocketEnableRead(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    FD_SET(socket->getDescriptor(), &fdsRead);

    {
        InSequence seq;

        EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                            .WillOnce(Return(-1));
        EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                            .WillOnce(testing::DoAll(testing::SetArgPointee<1>(fdsRead), Return(1)));
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



TEST_F(TestSelect, testAddSocketReleaseByControlSocket)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));
    m_select->addSocketEnableRead(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    FD_SET(CONTROLSOCKET_READ, &fdsRead);

    {
        InSequence seq;

        EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                            .WillOnce(testing::DoAll(testing::SetArgPointee<1>(fdsRead), Return(1)));
        EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                            .WillOnce(Return(0));
    }
    EXPECT_CALL(*m_mockMockOperatingSystem, ioctlInt(CONTROLSOCKET_READ, FIONREAD, _)).Times(1)
                                                        .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(1), Return(0)));
    EXPECT_CALL(*m_mockMockOperatingSystem, read(CONTROLSOCKET_READ, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, true);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}



TEST_F(TestSelect, testAddSocketReadableError)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));
    m_select->addSocketEnableRead(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    FD_SET(socket->getDescriptor(), &fdsRead);
    EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                        .WillOnce(Return(-1));
    EXPECT_CALL(*m_mockMockOperatingSystem, getLastError()).Times(1)
                        .WillOnce(Return(SOCKETERROR(EACCES)));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, true);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}


TEST_F(TestSelect, testAddSocketReadableWaitSocketDescriptorsChanged)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));
    m_select->addSocketEnableRead(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    FD_SET(socket->getDescriptor(), &fdsRead);

    EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
            .WillRepeatedly(
                testing::DoAll(
                    testing::Invoke([this, &socket](int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout){
                        m_select->removeSocket(socket);
                    }),
                    testing::SetArgPointee<1>(fdsRead),
                    Return(1)
                )
            );
    EXPECT_CALL(*m_mockMockOperatingSystem, ioctlInt(socket->getDescriptor(), FIONREAD, _)).Times(1)
                                                        .WillRepeatedly(testing::DoAll(testing::SetArgPointee<2>(NUMBER_OF_BYTES_TO_READ), Return(0)));
    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));

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

TEST_F(TestSelect, testAddSocketDisconnect)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));
    m_select->addSocketEnableRead(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    FD_SET(socket->getDescriptor(), &fdsRead);

    EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                                                                  .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(fdsRead), Return(1)));
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

TEST_F(TestSelect, testAddSocketIoCtlError)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(1)
                                                        .WillRepeatedly(Return(1));
    m_select->addSocketEnableRead(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    FD_ZERO(&fdsRead);
    FD_SET(socket->getDescriptor(), &fdsRead);

    EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                                                                  .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(fdsRead), Return(1)));
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


TEST_F(TestSelect, testAddSocketWritableWait)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(2)
                                                        .WillRepeatedly(Return(1));

    m_select->addSocketEnableRead(socket);
    m_select->enableWrite(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsWriteIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    fd_set fdsRead;
    fd_set fdsWrite;
    FD_ZERO(&fdsRead);
    FD_ZERO(&fdsWrite);
    FD_SET(socket->getDescriptor(), &fdsWrite);

    EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                                                                  .WillRepeatedly(testing::DoAll(testing::SetArgPointee<1>(fdsRead),
                                                                                                 testing::SetArgPointee<2>(fdsWrite),
                                                                                                 Return(1)));

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





TEST_F(TestSelect, testAddSocketDisableWritableWait)
{
    SocketDescriptorPtr socket = std::make_shared<SocketDescriptor>(TESTSOCKET);

    EXPECT_CALL(*m_mockMockOperatingSystem, write(CONTROLSOCKET_WRITE, _, 1)).Times(3)
                                                        .WillRepeatedly(Return(1));
    m_select->addSocketEnableRead(socket);
    m_select->enableWrite(socket);
    m_select->disableWrite(socket);

    fd_set fdsReadIn;
    fd_set fdsWriteIn;
    FD_ZERO(&fdsReadIn);
    FD_ZERO(&fdsWriteIn);
    FD_SET(CONTROLSOCKET_READ, &fdsReadIn);
    FD_SET(socket->getDescriptor(), &fdsReadIn);

    timeval tim;
    tim.tv_sec = 0;
    tim.tv_usec = TIMEOUT * MILLITOMICRO;

    EXPECT_CALL(*m_mockMockOperatingSystem, select(socket->getDescriptor() + 1, FdSet(&fdsReadIn), FdSet(&fdsWriteIn), nullptr, Time(&tim))).Times(1)
                                                                  .WillRepeatedly(Return(0));

    const PollerResult& result = m_select->wait(TIMEOUT);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, true);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}

