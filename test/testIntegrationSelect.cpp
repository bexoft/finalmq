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


#include "finalmq/poller/PollerImplSelect.h"
#include "finalmq/poller/PollerImplEpoll.h"
#include "finalmq/helpers/OperatingSystem.h"

#include <thread>
#include <chrono>

using ::testing::_;
using ::testing::Return;

using namespace std::chrono_literals;
using namespace finalmq;

static const std::string BUFFER = "Hello";


typedef PollerImplSelect Poller;


TEST(TestIntegrationSelect, testTimeout)
{
    std::unique_ptr<IPoller> poller = std::make_unique<Poller>();
    poller->init();
    const PollerResult& result = poller->wait(0);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, true);
    EXPECT_EQ(result.descriptorInfos.size(), 0);
}




TEST(TestIntegrationSelect, testAddSocketReadableBeforeWait)
{
    std::unique_ptr<IPoller> poller = std::make_unique<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    poller->addSocket(controlSocketInside);
    poller->enableRead(controlSocketInside);
    OperatingSystem::instance().send(controlSocketOutside->getDescriptor(), BUFFER.c_str(), static_cast<int>(BUFFER.size()), 0);

    const PollerResult& result = poller->wait(10);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    EXPECT_EQ(result.descriptorInfos[0].sd, controlSocketInside->getDescriptor());
    EXPECT_EQ(result.descriptorInfos[0].readable, true);
    EXPECT_EQ(result.descriptorInfos[0].writable, false);
    EXPECT_EQ(result.descriptorInfos[0].bytesToRead, BUFFER.size());
}


TEST(TestIntegrationSelect, testAddSocketReadableInsideWait)
{
    std::shared_ptr<IPoller> poller = std::make_shared<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    std::thread thread([poller, controlSocketInside, controlSocketOutside] () {
        std::this_thread::sleep_for(10ms);
        poller->addSocket(controlSocketInside);
        poller->enableRead(controlSocketInside);
        OperatingSystem::instance().send(controlSocketOutside->getDescriptor(), BUFFER.c_str(), static_cast<int>(BUFFER.size()), 0);
    });

    const PollerResult& result = poller->wait(1000000);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, false);
    EXPECT_EQ(result.descriptorInfos.size(), 1);
    EXPECT_EQ(result.descriptorInfos[0].sd, controlSocketInside->getDescriptor());
    EXPECT_EQ(result.descriptorInfos[0].readable, true);
    EXPECT_EQ(result.descriptorInfos[0].writable, false);
    EXPECT_EQ(result.descriptorInfos[0].bytesToRead, BUFFER.size());

    thread.join();
}


TEST(TestIntegrationSelect, testEnableWriteSocketBeforeWait)
{
    std::unique_ptr<IPoller> poller = std::make_unique<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    poller->addSocket(controlSocketInside);
    poller->enableRead(controlSocketInside);
    const PollerResult& result1 = poller->wait(0);
    EXPECT_EQ(result1.error, false);
    EXPECT_EQ(result1.timeout, true);
    EXPECT_EQ(result1.descriptorInfos.size(), 0);

    poller->enableWrite(controlSocketInside);
    const PollerResult& result2 = poller->wait(0);
    EXPECT_EQ(result2.error, false);
    EXPECT_EQ(result2.timeout, false);
    EXPECT_EQ(result2.descriptorInfos.size(), 1);
    EXPECT_EQ(result2.descriptorInfos[0].sd, controlSocketInside->getDescriptor());
    EXPECT_EQ(result2.descriptorInfos[0].readable, false);
    EXPECT_EQ(result2.descriptorInfos[0].writable, true);
    EXPECT_EQ(result2.descriptorInfos[0].bytesToRead, 0);
}


TEST(TestIntegrationSelect, testEnableWriteSocketInsideWait)
{
    std::shared_ptr<IPoller> poller = std::make_shared<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    poller->addSocket(controlSocketInside);
    poller->enableRead(controlSocketInside);

    const PollerResult& result1 = poller->wait(0);
    EXPECT_EQ(result1.error, false);
    EXPECT_EQ(result1.timeout, true);
    EXPECT_EQ(result1.descriptorInfos.size(), 0);

    std::thread thread([poller, controlSocketInside] () {
        std::this_thread::sleep_for(10ms);
        poller->enableWrite(controlSocketInside);
    });

    const PollerResult& result2 = poller->wait(1000000);
    EXPECT_EQ(result2.error, false);
    EXPECT_EQ(result2.timeout, false);
    EXPECT_EQ(result2.descriptorInfos.size(), 1);
    EXPECT_EQ(result2.descriptorInfos[0].sd, controlSocketInside->getDescriptor());
    EXPECT_EQ(result2.descriptorInfos[0].readable, false);
    EXPECT_EQ(result2.descriptorInfos[0].writable, true);
    EXPECT_EQ(result2.descriptorInfos[0].bytesToRead, 0);

    thread.join();
}

TEST(TestIntegrationSelect, testEnableWriteSocketNotWritable)
{
    std::unique_ptr<IPoller> poller = std::make_unique<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    poller->addSocket(controlSocketInside);
    poller->enableRead(controlSocketInside);
    const PollerResult& result1 = poller->wait(0);
    EXPECT_EQ(result1.error, false);
    EXPECT_EQ(result1.timeout, true);
    EXPECT_EQ(result1.descriptorInfos.size(), 0);

    static const std::string LARGE_BUFFER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    res = 0;
    while (res >= 0)
    {
        res = OperatingSystem::instance().send(controlSocketInside->getDescriptor(), LARGE_BUFFER.c_str(), static_cast<int>(LARGE_BUFFER.size()), 0);
    }
    poller->enableWrite(controlSocketInside);
    const PollerResult& result2 = poller->wait(0);
    EXPECT_EQ(result2.error, false);
    EXPECT_EQ(result2.timeout, true);
    EXPECT_EQ(result2.descriptorInfos.size(), 0);
}


TEST(TestIntegrationSelect, testEnableWriteSocketNotWritableToWritable)
{
    std::shared_ptr<IPoller> poller = std::make_unique<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    poller->addSocket(controlSocketInside);
    poller->enableRead(controlSocketInside);
    const PollerResult& result1 = poller->wait(0);
    EXPECT_EQ(result1.error, false);
    EXPECT_EQ(result1.timeout, true);
    EXPECT_EQ(result1.descriptorInfos.size(), 0);

    static const std::string LARGE_BUFFER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    res = 0;
    while (res >= 0)
    {
        res = OperatingSystem::instance().send(controlSocketInside->getDescriptor(), LARGE_BUFFER.c_str(), static_cast<int>(LARGE_BUFFER.size()), 0);
    }
    poller->enableWrite(controlSocketInside);

    std::thread thread([poller, controlSocketOutside] () {
        std::this_thread::sleep_for(10ms);
        char buffer[1024];
        int res = 0;
        while (res >= 0)
        {
            res = OperatingSystem::instance().recv(controlSocketOutside->getDescriptor(), buffer, sizeof(buffer), 0);
        }
    });

    const PollerResult& result2 = poller->wait(1000000);
    EXPECT_EQ(result2.error, false);
    EXPECT_EQ(result2.timeout, false);
    EXPECT_EQ(result2.descriptorInfos.size(), 1);
    EXPECT_EQ(result2.descriptorInfos[0].sd, controlSocketInside->getDescriptor());
    EXPECT_EQ(result2.descriptorInfos[0].readable, false);
    EXPECT_EQ(result2.descriptorInfos[0].writable, true);
    EXPECT_EQ(result2.descriptorInfos[0].bytesToRead, 0);

    thread.join();
}


TEST(TestIntegrationSelect, testDisableWriteSocket)
{
    std::shared_ptr<IPoller> poller = std::make_unique<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    poller->addSocket(controlSocketInside);
    poller->enableRead(controlSocketInside);
    const PollerResult& result1 = poller->wait(0);
    EXPECT_EQ(result1.error, false);
    EXPECT_EQ(result1.timeout, true);
    EXPECT_EQ(result1.descriptorInfos.size(), 0);

    static const std::string LARGE_BUFFER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    res = 0;
    while (res >= 0)
    {
        res = OperatingSystem::instance().send(controlSocketInside->getDescriptor(), LARGE_BUFFER.c_str(), static_cast<int>(LARGE_BUFFER.size()), 0);
    }
    poller->enableWrite(controlSocketInside);

    std::thread thread([poller, controlSocketInside, controlSocketOutside] () {
        std::this_thread::sleep_for(10ms);
        poller->disableWrite(controlSocketInside);
        std::this_thread::sleep_for(10ms);
        char buffer[1024];
        int res = 0;
        while (res >= 0)
        {
            res = OperatingSystem::instance().recv(controlSocketOutside->getDescriptor(), buffer, sizeof(buffer), 0);
        }
    });

    const PollerResult& result2 = poller->wait(50);
    EXPECT_EQ(result2.error, false);
    EXPECT_EQ(result2.timeout, true);
    EXPECT_EQ(result2.descriptorInfos.size(), 0);

    thread.join();
}


TEST(TestIntegrationSelect, testRemoveSocketInsideWait)
{
    std::shared_ptr<IPoller> poller = std::make_shared<Poller>();
    poller->init();

    SocketDescriptorPtr controlSocketInside;
    SocketDescriptorPtr controlSocketOutside;

    int res = OperatingSystem::instance().makeSocketPair(controlSocketInside, controlSocketOutside);
    EXPECT_EQ(res, 0);
    EXPECT_NE(controlSocketInside, nullptr);
    EXPECT_NE(controlSocketOutside, nullptr);

    poller->addSocket(controlSocketInside);
    poller->enableRead(controlSocketInside);

    std::thread thread([poller, controlSocketInside, controlSocketOutside] () {
        std::this_thread::sleep_for(10ms);
        poller->removeSocket(controlSocketInside);
        std::this_thread::sleep_for(10ms);
        OperatingSystem::instance().send(controlSocketOutside->getDescriptor(), BUFFER.c_str(), static_cast<int>(BUFFER.size()), 0);
    });


    const PollerResult& result = poller->wait(50);
    EXPECT_EQ(result.error, false);
    EXPECT_EQ(result.timeout, true);
    EXPECT_EQ(result.descriptorInfos.size(), 0);

    thread.join();
}

