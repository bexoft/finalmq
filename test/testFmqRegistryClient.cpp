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

#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatProto.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/protocols/ProtocolHeaderBinarySize.h"

#include "finalmq/remoteentity/FmqRegistryClient.h"
#include "../services/fmqreg/registry.h"

#include "test.fmq.h"

#include "testHelper.h"

#include <thread>
#include <chrono>

using ::testing::_;
using ::testing::Return;

using namespace finalmq;

#define PORTNUMBER_PROTO    "18180"


class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        registerCommand<test::TestRequest>([] (const RequestContextPtr& requestContext, const std::shared_ptr<test::TestRequest>& request) {
            assert(request);
            requestContext->reply(test::TestReply("test reply"));
        });
    }
};


class MockEventsRegistry
{
public:
    MOCK_METHOD(void, testReply, (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply));
    MOCK_METHOD(void, connEvent, (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent));
    MOCK_METHOD(void, connectReply, (PeerId peerId, Status status));
};



class TestIntegrationFmqRegistryClient: public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_entityContainerRegistry.init(nullptr, 1, nullptr, false, 1);
        m_entityContainerRegistry.registerEntity(&m_registry, "fmqreg");
        m_entityContainerRegistry.bind("tcp://*:" PORTNUMBER_PROTO ":headersize:protobuf");
        m_threadRegistry = std::thread([this] () {
            m_entityContainerRegistry.run();
        });

        m_entityContainerServer = std::make_shared<RemoteEntityContainer>();
        m_entityContainerServer->init(nullptr, 1, nullptr, false, 1);
        m_entityIdServer = m_entityContainerServer->registerEntity(&m_server);
        m_entityContainerServer->bind("tcp://*:7799:headersize:protobuf");
        m_threadServer = std::thread([this] () {
            m_entityContainerServer->run();
        });

        m_entityContainerClient = std::make_shared<RemoteEntityContainer>();
        m_entityContainerClient->init(nullptr, 1, nullptr, false, 1);
        m_entityIdClient = m_entityContainerClient->registerEntity(&m_entityClient);
        m_threadClient = std::thread([this] () {
            m_entityContainerClient->run();
        });
    }

    virtual void TearDown()
    {
        FmqRegistryClient fmqRegistryClient(m_entityContainerServer);
        fmqRegistryClient.registerService({"MyService", "", ENTITYID_INVALID, {}});

        m_entityContainerRegistry.terminatePollerLoop();
        m_threadRegistry.join();

        m_entityContainerServer->terminatePollerLoop();
        m_threadServer.join();

        m_entityContainerClient->terminatePollerLoop();
        m_threadClient.join();
    }


    RemoteEntityContainer       m_entityContainerRegistry;
    Registry                    m_registry;
    std::thread                 m_threadRegistry;

    IRemoteEntityContainerPtr   m_entityContainerServer;
    EntityId                    m_entityIdServer = ENTITYID_INVALID;
    EntityServer                m_server;
    std::thread                 m_threadServer;

    IRemoteEntityContainerPtr   m_entityContainerClient;
    RemoteEntity                m_entityClient;
    EntityId                    m_entityIdClient = ENTITYID_INVALID;
    std::thread                 m_threadClient;

    MockEventsRegistry          m_mockEvents;
};




TEST_F(TestIntegrationFmqRegistryClient, testConnectDisconnect)
{
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    auto& expectConnectReply = EXPECT_CALL(m_mockEvents, connectReply(_, _)).Times(1);

    fmqRegistryClient.connectService("MyService", m_entityIdClient, {}, [this] (PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    bool ok = waitTillDone(expectConnectReply, 15000);
    ASSERT_EQ(ok, true);
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectSendDisconnect)
{
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    EXPECT_CALL(m_mockEvents, connectReply(_, _)).Times(1);
    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, { {}, {10, 3000} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, _, _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });
    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}


TEST_F(TestIntegrationFmqRegistryClient, testConnectSend)
{
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    EXPECT_CALL(m_mockEvents, connectReply(_, Status(Status::STATUS_OK))).Times(1);
    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, { {}, {10, 3000} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, Status(Status::STATUS_OK), _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });
    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectEmptyEndpointSend)
{
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    fmqRegistryClient.registerService({"MyService", "", ENTITYID_INVALID, {}});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    EXPECT_CALL(m_mockEvents, connectReply(_, Status(Status::STATUS_PEER_DISCONNECTED))).Times(1);
    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, { {}, {10, 3000} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, Status(Status::STATUS_PEER_DISCONNECTED), _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });
    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectNoServiceSend)
{
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    EXPECT_CALL(m_mockEvents, connectReply(_, Status(Status::STATUS_PEER_DISCONNECTED))).Times(1);
    PeerId peerId = fmqRegistryClient.connectService("NoService", m_entityIdClient, { {}, {10, 100} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, Status(Status::STATUS_PEER_DISCONNECTED), _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });
    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}



TEST_F(TestIntegrationFmqRegistryClient, testConnectDisconnectLateRegister)
{
    auto& expectConnectReply = EXPECT_CALL(m_mockEvents, connectReply(_, _)).Times(1);

    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    fmqRegistryClient.connectService("MyService", m_entityIdClient, { {}, {10, 3000} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});

    bool ok = waitTillDone(expectConnectReply, 15000);
    ASSERT_EQ(ok, true);
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectSendDisconnectLateRegister)
{
    EXPECT_CALL(m_mockEvents, connectReply(_, _)).Times(1);
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, { {}, {10, 3000} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, _, _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});

    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}


TEST_F(TestIntegrationFmqRegistryClient, testConnectSendLateRegister)
{
    EXPECT_CALL(m_mockEvents, connectReply(_, Status(Status::STATUS_OK))).Times(1);
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, { {}, {10, 3000} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, Status(Status::STATUS_OK), _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});

    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectEmptyEndpointSendLateRegister)
{
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);

    EXPECT_CALL(m_mockEvents, connectReply(_, Status(Status::STATUS_PEER_DISCONNECTED))).Times(1);
    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, { {}, {10, 3000} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, Status(Status::STATUS_PEER_DISCONNECTED), _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    fmqRegistryClient.registerService({"MyService", "", ENTITYID_INVALID, {}});

    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectNoServiceSendLateRegister)
{
    EXPECT_CALL(m_mockEvents, connectReply(_, Status(Status::STATUS_PEER_DISCONNECTED))).Times(1);
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    PeerId peerId = fmqRegistryClient.connectService("NoService", m_entityIdClient, { {}, {10, 100} }, [this](PeerId peerId, Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, Status(Status::STATUS_PEER_DISCONNECTED), _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    fmqRegistryClient.registerService({"MyService", "", m_entityIdServer, {{fmqreg::SocketProtocol::SOCKET_TCP, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7799:headersize:protobuf"}}});

    bool ok = waitTillDone(expectReply, 15000);
    ASSERT_EQ(ok, true);
}

