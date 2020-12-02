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


#include "remoteentity/RemoteEntityContainer.h"
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocols/ProtocolDelimiter.h"
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
    MOCK_METHOD(void, testRequest, (ReplyContextUPtr& replyContext, const std::shared_ptr<TestRequest>& request));
    MOCK_METHOD(void, testReply, (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply));
    MOCK_METHOD(void, peerEvent, (PeerId peerId, PeerEvent peerEvent, bool incoming));
    MOCK_METHOD(void, connEvent, (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent));
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
    }

    virtual void TearDown()
    {
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
        registerCommand<TestRequest>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<TestRequest>& request) {
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
    RemoteEntityContainer entityContainer1;
    RemoteEntityContainer entityContainer2;
    EntityServer entityServer(mockEventsServer);
    RemoteEntity entityClient;

    IRemoteEntityContainer& ientityContainerServer = entityContainer1;
    IRemoteEntityContainer& ientityContainerClient = entityContainer2;
    IRemoteEntity& ientityClient = entityClient;

    std::thread thread1 = std::thread([&ientityContainerServer] () {
        ientityContainerServer.threadEntry();
    });
    std::thread thread2 = std::thread([&ientityContainerClient] () {
        ientityContainerClient.threadEntry();
    });

    ientityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    ientityContainerServer.init();
    ientityContainerClient.init();

    ientityContainerServer.registerEntity(&entityServer, "MyServer");
    ientityContainerClient.registerEntity(&entityClient);

    ientityContainerServer.bind("tcp://*:7777", std::make_shared<ProtocolHeaderBinarySizeFactory>(), CONTENTTYPE_PROTO);
    IProtocolSessionPtr sessionClient = ientityContainerClient.connect("tcp://localhost:7777", std::make_shared<ProtocolHeaderBinarySize>(), CONTENTTYPE_PROTO);

    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent::PEER_CONNECTED, true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent::PEER_CONNECTED, false)).Times(1);
    PeerId peerId = ientityClient.connect(sessionClient, "MyServer");

    static const int LOOP = 1;
    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        ientityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    waitTillDone(expectReply, 150000);
    bool ok1 = ientityContainerServer.terminatePollerLoop(1000);
    bool ok2 = ientityContainerClient.terminatePollerLoop(1000);
    ASSERT_EQ(ok1, true);
    ASSERT_EQ(ok2, true);
    thread1.join();
    thread2.join();
}




TEST_F(TestIntegrationRemoteEntity, testJson)
{
    MockEvents mockEventsServer;
    MockEvents mockEventsClient;
    RemoteEntityContainer entityContainer1;
    RemoteEntityContainer entityContainer2;
    EntityServer entityServer(mockEventsServer);
    RemoteEntity entityClient;

    IRemoteEntityContainer& ientityContainerServer = entityContainer1;
    IRemoteEntityContainer& ientityContainerClient = entityContainer2;
    IRemoteEntity& ientityClient = entityClient;

    std::thread thread1 = std::thread([&ientityContainerServer] () {
        ientityContainerServer.threadEntry();
    });
    std::thread thread2 = std::thread([&ientityContainerClient] () {
        ientityContainerClient.threadEntry();
    });

    ientityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    ientityContainerServer.init();
    ientityContainerClient.init();

    ientityContainerServer.registerEntity(&entityServer, "MyServer");
    ientityContainerClient.registerEntity(&entityClient);

    ientityContainerServer.bind("tcp://*:7777", std::make_shared<ProtocolDelimiterFactory>("\n"), CONTENTTYPE_JSON);
    IProtocolSessionPtr sessionClient = ientityContainerClient.connect("tcp://localhost:7777", std::make_shared<ProtocolDelimiter>("\n"), CONTENTTYPE_JSON);

    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent::PEER_CONNECTED, true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent::PEER_CONNECTED, false)).Times(1);
    PeerId peerId = ientityClient.connect(sessionClient, "MyServer");

    static const int LOOP = 1;
    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        ientityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    waitTillDone(expectReply, 150000);
    bool ok1 = ientityContainerServer.terminatePollerLoop(1000);
    bool ok2 = ientityContainerClient.terminatePollerLoop(1000);
    ASSERT_EQ(ok1, true);
    ASSERT_EQ(ok2, true);
    thread1.join();
    thread2.join();
}


TEST_F(TestIntegrationRemoteEntity, testSslProto)
{
    MockEvents mockEventsServer;
    MockEvents mockEventsClient;
    RemoteEntityContainer entityContainer1;
    RemoteEntityContainer entityContainer2;
    EntityServer entityServer(mockEventsServer);
    RemoteEntity entityClient;

    IRemoteEntityContainer& ientityContainerServer = entityContainer1;
    IRemoteEntityContainer& ientityContainerClient = entityContainer2;
    IRemoteEntity& ientityClient = entityClient;

    std::thread thread1 = std::thread([&ientityContainerServer] () {
        ientityContainerServer.threadEntry();
    });
    std::thread thread2 = std::thread([&ientityContainerClient] () {
        ientityContainerClient.threadEntry();
    });

    ientityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    ientityContainerServer.init();
    ientityContainerClient.init();

    ientityContainerServer.registerEntity(&entityServer, "MyServer");
    ientityContainerClient.registerEntity(&entityClient);

    ientityContainerServer.bind("tcp://*:7777", std::make_shared<ProtocolHeaderBinarySizeFactory>(), CONTENTTYPE_PROTO, {{true, "ssltest.cert.pem", "ssltest.key.pem"}});
    IProtocolSessionPtr sessionClient = ientityContainerClient.connect("tcp://localhost:7777", std::make_shared<ProtocolHeaderBinarySize>(), CONTENTTYPE_PROTO, {{true}});

    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent::PEER_CONNECTED, true)).Times(1);
    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent::PEER_CONNECTED, false)).Times(1);
    PeerId peerId = ientityClient.connect(sessionClient, "MyServer");

    static const int LOOP = 1;
    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        ientityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    waitTillDone(expectReply, 150000);
    bool ok1 = ientityContainerServer.terminatePollerLoop(1000);
    bool ok2 = ientityContainerClient.terminatePollerLoop(1000);
    ASSERT_EQ(ok1, true);
    ASSERT_EQ(ok2, true);
    thread1.join();
    thread2.join();
}








/*
TEST_F(TestIntegrationRemoteEntity, testServer)
{
    MockEvents mockEventsServer;
    RemoteEntityContainer entityContainer1;
    EntityServer entityServer(mockEventsServer);

    IRemoteEntityContainer& ientityContainerServer = entityContainer1;

    std::thread thread1 = std::thread([&ientityContainerServer] () {
        ientityContainerServer.threadEntry();
    });

    ientityContainerServer.init();

    ientityContainerServer.registerEntity(&entityServer, "MyServer");
//{"mode":"MSG_REQUEST","corrid":"1","destname":"MyServer","type":"test.TestRequest"}{}
    ientityContainerServer.bind("tcp://localhost:8888", std::make_shared<ProtocolDelimiterFactory>("\n"), CONTENTTYPE_JSON);
    ientityContainerServer.bind("tcp://localhost:7777", std::make_shared<ProtocolHeaderBinarySizeFactory>(), CONTENTTYPE_PROTO);
    ientityContainerServer.bind("ipc://abc1234", std::make_shared<ProtocolHeaderBinarySizeFactory>(), CONTENTTYPE_PROTO);

//    EXPECT_CALL(mockEventsServer, peerEvent(_, PeerEvent::PEER_CONNECTED, true)).WillRepeatedly();

    std::this_thread::sleep_for(std::chrono::milliseconds(10000000));
    bool ok1 = ientityContainerServer.terminatePollerLoop(1000);
    ASSERT_EQ(ok1, true);
    thread1.join();
}


TEST_F(TestIntegrationRemoteEntity, testClient)
{
    MockEvents mockEventsClient;
    RemoteEntityContainer entityContainer2;
    RemoteEntity entityClient;

    IRemoteEntityContainer& ientityContainerClient = entityContainer2;
    IRemoteEntity& ientityClient = entityClient;

    std::thread thread2 = std::thread([&ientityContainerClient] () {
        ientityContainerClient.threadEntry();
    });

    ientityClient.registerPeerEvent([&mockEventsClient] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        mockEventsClient.peerEvent(peerId, peerEvent, incoming);
    });

    ientityContainerClient.init();

    ientityContainerClient.registerEntity(&entityClient);

//    IProtocolSessionPtr sessionClient = ientityContainerClient.connect("tcp://localhost:7777", std::make_shared<ProtocolHeaderBinarySize>(), CONTENTTYPE_PROTO);
    IProtocolSessionPtr sessionClient = ientityContainerClient.connect("ipc://abc1234", std::make_shared<ProtocolHeaderBinarySize>(), CONTENTTYPE_PROTO);

    EXPECT_CALL(mockEventsClient, peerEvent(_, PeerEvent::PEER_CONNECTED, false)).Times(1);
    PeerId peerId = ientityClient.connect(sessionClient, "MyServer");

    auto timestart = std::chrono::high_resolution_clock::now();
    static const int LOOP = 100000;
//    EXPECT_CALL(mockEventsServer, testRequest(_, _)).Times(LOOP);
    auto& expectReply = EXPECT_CALL(mockEventsClient, testReply(peerId, _, _)).Times(LOOP);
    for (int i = 0; i < LOOP; ++i)
    {
        ientityClient.requestReply<TestReply>(peerId, TestRequest{DATA_REQUEST}, [&mockEventsClient] (PeerId peerId, remoteentity::Status status, const std::shared_ptr<TestReply>& reply) {
            ASSERT_EQ(reply->datareply, DATA_REPLY);
            mockEventsClient.testReply(peerId, status, reply);
        });
    }

    waitTillDone(expectReply, 150000);
    auto timeend = std::chrono::high_resolution_clock::now();
    auto diff = (timeend - timestart) / std::chrono::milliseconds(1);
    std::cout << "Time: " << diff << std::endl;


    bool ok2 = ientityContainerClient.terminatePollerLoop(1000);
    ASSERT_EQ(ok2, true);
    thread2.join();
}
*/



#endif
