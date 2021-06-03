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

#ifdef USE_OPENSSL

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatProto.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/protocols/ProtocolHeaderBinarySize.h"
#include "finalmq/protocols/ProtocolDelimiterLinefeed.h"
#include "finalmq/logger/Logger.h"
#include "test.fmq.h"

#include "testHelper.h"

#include <thread>

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;


using namespace finalmq;
using namespace test;


class MockEvents
{
public:
    MOCK_METHOD(void, testRequest, (const ReplyContextPtr& replyContext, const std::shared_ptr<TestRequest>& request));
    MOCK_METHOD(void, testReply, (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply));
    MOCK_METHOD(void, peerEvent, (PeerId peerId, PeerEvent peerEvent, bool incoming));
    MOCK_METHOD(void, connEvent, (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent));
    MOCK_METHOD(void, connectReply, (PeerId peerId, remoteentity::Status status));
};



class TestIntegrationRemoteEntity: public testing::Test
{
public:
    void receivedServer(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead)
    {
    }

protected:
    virtual void SetUp()
    {
        Logger::instance().registerConsumer([] (const LogContext& context, const char* text) {
            std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
            ASSERT_EQ(true, false);
        });
    }

    virtual void TearDown()
    {
        // reset logger
        std::unique_ptr<ILogger> noLogger;
        Logger::setInstance(noLogger);
    }

};


static const std::string DATA_REQUEST = "Hello";
static const std::string DATA_REPLY = "World";



class EntityServer : public RemoteEntity
{
public:
    EntityServer(MockEvents& mockEvents)
        : m_mockEvents(mockEvents)
    {
        registerCommand<TestRequest>([this] (const ReplyContextPtr& replyContext, const std::shared_ptr<TestRequest>& request) {
            assert(request);
            m_mockEvents.testRequest(replyContext, request);
            ASSERT_EQ(request->datarequest, DATA_REQUEST);
            replyContext->reply(TestReply(DATA_REPLY));
        });

        registerPeerEvent([this] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
            m_mockEvents.peerEvent(peerId, peerEvent, incoming);
        });
    }

    MockEvents& m_mockEvents;
};




TEST_F(TestIntegrationRemoteEntity, testProto)
{
    MockEvents mockEventsServer;
    MockEvents mockEventsClient;
    RemoteEntityContainer entityContainerServer;
    RemoteEntityContainer entityContainerClient;
    EntityServer entityServer(mockEventsServer);
    RemoteEntity entityClient;

    entityContainerServer.init();
    entityContainerClient.init();

    std::thread thread1 = std::thread([&entityContainerServer] () {
        entityContainerServer.run();
    });
    std::thread thread2 = std::thread([&entityContainerClient] () {
        entityContainerClient.run();
    });

    entityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    entityContainerServer.registerEntity(&entityServer, "MyServer");
    entityContainerClient.registerEntity(&entityClient);

    entityContainerServer.bind("tcp://*:7788", std::make_shared<ProtocolHeaderBinarySizeFactory>(), RemoteEntityFormatProto::CONTENT_TYPE);
    IProtocolSessionPtr sessionClient = entityContainerClient.connect("tcp://localhost:7788", std::make_shared<ProtocolHeaderBinarySize>(), RemoteEntityFormatProto::CONTENT_TYPE);

    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTING), false)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), false)).Times(1);
    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), false)).Times(1);
    EXPECT_CALL(mockEventsClient, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_OK))).Times(1);
    PeerId peerId = entityClient.connect(sessionClient, "MyServer", [&mockEventsClient] (PeerId peerId, remoteentity::Status status) {
        mockEventsClient.connectReply(peerId, status);
    });

    static const int LOOP = 1;
    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        entityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(status, remoteentity::Status::STATUS_OK);
            ASSERT_NE(reply, nullptr);
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    waitTillDone(expectReply, 15000);
    entityContainerServer.terminatePollerLoop();
    entityContainerClient.terminatePollerLoop();
    thread1.join();
    thread2.join();
}




TEST_F(TestIntegrationRemoteEntity, testJson)
{
    MockEvents mockEventsServer;
    MockEvents mockEventsClient;
    RemoteEntityContainer entityContainerServer;
    RemoteEntityContainer entityContainerClient;
    EntityServer entityServer(mockEventsServer);
    RemoteEntity entityClient;

    entityContainerServer.init();
    entityContainerClient.init();

    std::thread thread1 = std::thread([&entityContainerServer] () {
        entityContainerServer.run();
    });
    std::thread thread2 = std::thread([&entityContainerClient] () {
        entityContainerClient.run();
    });

    entityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    entityContainerServer.registerEntity(&entityServer, "MyServer");
    entityContainerClient.registerEntity(&entityClient);

    entityContainerServer.bind("tcp://*:7788", std::make_shared<ProtocolDelimiterLinefeedFactory>(), RemoteEntityFormatJson::CONTENT_TYPE);
    IProtocolSessionPtr sessionClient = entityContainerClient.connect("tcp://localhost:7788", std::make_shared<ProtocolDelimiterLinefeed>(), RemoteEntityFormatJson::CONTENT_TYPE);

    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTING), false)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), false)).Times(1);
    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), false)).Times(1);
    EXPECT_CALL(mockEventsClient, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_OK))).Times(1);
    PeerId peerId = entityClient.connect(sessionClient, "MyServer", [&mockEventsClient] (PeerId peerId, remoteentity::Status status) {
        mockEventsClient.connectReply(peerId, status);
    });

    static const int LOOP = 1;
    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        entityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    waitTillDone(expectReply, 15000);
    entityContainerServer.terminatePollerLoop();
    entityContainerClient.terminatePollerLoop();
    thread1.join();
    thread2.join();
}


TEST_F(TestIntegrationRemoteEntity, testSslProto)
{
    MockEvents mockEventsServer;
    MockEvents mockEventsClient;
    RemoteEntityContainer entityContainerServer;
    RemoteEntityContainer entityContainerClient;
    EntityServer entityServer(mockEventsServer);
    RemoteEntity entityClient;

    entityContainerServer.init();
    entityContainerClient.init();

    std::thread thread1 = std::thread([&entityContainerServer] () {
        entityContainerServer.run();
    });
    std::thread thread2 = std::thread([&entityContainerClient] () {
        entityContainerClient.run();
    });

    entityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    entityContainerServer.registerEntity(&entityServer, "MyServer");
    entityContainerClient.registerEntity(&entityClient);

    entityContainerServer.bind("tcp://*:7788", std::make_shared<ProtocolHeaderBinarySizeFactory>(), RemoteEntityFormatProto::CONTENT_TYPE, {{true, "ssltest.cert.pem", "ssltest.key.pem"}});
    IProtocolSessionPtr sessionClient = entityContainerClient.connect("tcp://localhost:7788", std::make_shared<ProtocolHeaderBinarySize>(), RemoteEntityFormatProto::CONTENT_TYPE, {{true}});

    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTING), false)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), false)).Times(1);
    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), false)).Times(1);
    EXPECT_CALL(mockEventsClient, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_OK))).Times(1);
    PeerId peerId = entityClient.connect(sessionClient, "MyServer", [&mockEventsClient] (PeerId peerId, remoteentity::Status status) {
        mockEventsClient.connectReply(peerId, status);
    });

    static const int LOOP = 1;
    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        entityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    waitTillDone(expectReply, 15000);
    entityContainerServer.terminatePollerLoop();
    entityContainerClient.terminatePollerLoop();
    thread1.join();
    thread2.join();
}



TEST_F(TestIntegrationRemoteEntity, testProtoLateConnect)
{
    MockEvents mockEventsServer;
    MockEvents mockEventsClient;
    RemoteEntityContainer entityContainerServer;
    RemoteEntityContainer entityContainerClient;
    EntityServer entityServer(mockEventsServer);
    RemoteEntity entityClient;

    entityContainerServer.init();
    entityContainerClient.init();

    std::thread thread1 = std::thread([&entityContainerServer] () {
        entityContainerServer.run();
    });
    std::thread thread2 = std::thread([&entityContainerClient] () {
        entityContainerClient.run();
    });

    entityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    entityContainerServer.registerEntity(&entityServer, "MyServer");
    entityContainerClient.registerEntity(&entityClient);

    entityContainerServer.bind("tcp://*:7788", std::make_shared<ProtocolHeaderBinarySizeFactory>(), RemoteEntityFormatProto::CONTENT_TYPE);

    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTING), false)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_CONNECTED), false)).Times(1);
    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent(PeerEvent::PEER_DISCONNECTED), false)).Times(1);

    PeerId peerId = entityClient.createPeer([&mockEventsClient] (PeerId peerId, remoteentity::Status status) {
            mockEventsClient.connectReply(peerId, status);
    });

    static const int LOOP = 1;
    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        entityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(status, remoteentity::Status::STATUS_OK);
            ASSERT_NE(reply, nullptr);
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    IProtocolSessionPtr sessionClient = entityContainerClient.connect("tcp://localhost:7788", std::make_shared<ProtocolHeaderBinarySize>(), RemoteEntityFormatProto::CONTENT_TYPE);

    EXPECT_CALL(mockEventsClient, connectReply(_, remoteentity::Status(remoteentity::Status::STATUS_OK))).Times(1);
    entityClient.connect(peerId, sessionClient, "MyServer");

    waitTillDone(expectReply, 15000);
    entityContainerServer.terminatePollerLoop();
    entityContainerClient.terminatePollerLoop();
    thread1.join();
    thread2.join();
}




#endif
