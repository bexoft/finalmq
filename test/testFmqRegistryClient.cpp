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

#include "remoteentity/RemoteEntityContainer.h"
#include "protocols/ProtocolHeaderBinarySize.h"

#include "remoteentity/FmqRegistryClient.h"
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
        registerCommand<test::TestRequest>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<test::TestRequest>& request) {
            assert(request);
            replyContext->reply(test::TestReply("test reply"));
        });
    }
};


class MockEventsRegistry
{
public:
    MOCK_METHOD(void, testReply, (PeerId peerId, remoteentity::Status status, const std::shared_ptr<test::TestReply>& reply));
    MOCK_METHOD(void, connEvent, (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent));
    MOCK_METHOD(void, connectReply, (PeerId peerId, remoteentity::Status status));
};



class TestIntegrationFmqRegistryClient: public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
        m_entityContainerRegistry.init();
        m_entityContainerRegistry.registerEntity(&m_registry, "fmqreg");
        m_entityContainerRegistry.bind("tcp://*:" PORTNUMBER_PROTO, std::make_shared<ProtocolHeaderBinarySizeFactory>(), RemoteEntityContentType::CONTENTTYPE_PROTO);
        m_threadRegistry = std::thread([this] () {
            m_entityContainerRegistry.run();
        });

        m_entityContainerServer = std::make_shared<RemoteEntityContainer>();
        m_entityContainerServer->init();
        EntityId entityId = m_entityContainerServer->registerEntity(&m_server);
        m_entityContainerServer->bind("tcp://*:7799", std::make_shared<ProtocolHeaderBinarySizeFactory>(), RemoteEntityContentType::CONTENTTYPE_PROTO);
        m_threadServer = std::thread([this] () {
            m_entityContainerServer->run();
        });

        FmqRegistryClient fmqRegistryClient(m_entityContainerServer);
        fmqRegistryClient.registerService({"MyService", "", entityId, {{fmqreg::SocketProtocol::SOCKET_TCP, ProtocolHeaderBinarySize::PROTOCOL_ID, RemoteEntityContentType::CONTENTTYPE_PROTO, false, "tcp://*:7799"}}});

        m_entityContainerClient = std::make_shared<RemoteEntityContainer>();
        m_entityContainerClient->init();
        m_entityIdClient = m_entityContainerClient->registerEntity(&m_entityClient);
        m_threadClient = std::thread([this] () {
            m_entityContainerClient->run();
        });
    }

    virtual void TearDown()
    {
        FmqRegistryClient fmqRegistryClient(m_entityContainerServer);
        fmqRegistryClient.registerService({"MyService", "", ENTITYID_INVALID, {}});

        m_entityContainerRegistry.terminatePollerLoop(5000);
        m_threadRegistry.join();

        m_entityContainerServer->terminatePollerLoop(5000);
        m_threadServer.join();

        m_entityContainerClient->terminatePollerLoop(5000);
        m_threadClient.join();
    }


    RemoteEntityContainer       m_entityContainerRegistry;
    Registry                    m_registry;
    std::thread                 m_threadRegistry;

    IRemoteEntityContainerPtr   m_entityContainerServer;
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
    EXPECT_CALL(m_mockEvents, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_PEER_DISCONNECTED))).Times(1);

    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    fmqRegistryClient.connectService("MyService", m_entityIdClient, {}, [this] (PeerId peerId, remoteentity::Status status) {
        m_mockEvents.connectReply(peerId, status);
    });
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectSendDisconnect)
{
    EXPECT_CALL(m_mockEvents, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_PEER_DISCONNECTED))).Times(1);
    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, {}, [this] (PeerId peerId, remoteentity::Status status) {
        m_mockEvents.connectReply(peerId, status);
    });

    EXPECT_CALL(m_mockEvents, testReply(peerId, remoteentity::Status(remoteentity::Status::STATUS_PEER_DISCONNECTED), _)).Times(1);
    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<test::TestReply>& reply) {
        m_mockEvents.testReply(peerId, status, reply);
    });
}


TEST_F(TestIntegrationFmqRegistryClient, testConnectSend)
{
//    EXPECT_CALL(m_mockEvents, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_OK))).Times(1);
//    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
//    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, {}, [this] (PeerId peerId, remoteentity::Status status) {
//        m_mockEvents.connectReply(peerId, status);
//    });

//    auto& expectReply = EXPECT_CALL(m_mockEvents, testReply(peerId, remoteentity::Status(remoteentity::Status::STATUS_OK), _)).Times(1);
//    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<test::TestReply>& reply) {
//        m_mockEvents.testReply(peerId, status, reply);
//    });
//    waitTillDone(expectReply, 15000);
}

TEST_F(TestIntegrationFmqRegistryClient, testConnectNoServiceSend)
{
//    FmqRegistryClient fmqRegistryClient(m_entityContainerClient);
//    fmqRegistryClient.registerService({"MyService", "", ENTITYID_INVALID, {}});

//    EXPECT_CALL(m_mockEvents, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_PEER_DISCONNECTED))).Times(1);
//    PeerId peerId = fmqRegistryClient.connectService("MyService", m_entityIdClient, {}, [this] (PeerId peerId, remoteentity::Status status) {
//        m_mockEvents.connectReply(peerId, status);
//    });

//    EXPECT_CALL(m_mockEvents, testReply(peerId, remoteentity::Status(remoteentity::Status::STATUS_PEER_DISCONNECTED), _)).Times(1);
//    m_entityClient.requestReply<test::TestReply>(peerId, test::TestRequest{"request"}, [this] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<test::TestReply>& reply) {
//        m_mockEvents.testReply(peerId, status, reply);
//    });
}

