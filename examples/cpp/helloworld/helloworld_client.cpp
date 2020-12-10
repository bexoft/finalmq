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

#include "remoteentity/RemoteEntityContainer.h"
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocols/ProtocolDelimiter.h"
#include "logger/Logger.h"

// the definition of the messages are in the file helloworld.fmq
#include "helloworld.fmq.h"

#include <iostream>
#include <thread>
#include <algorithm>


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::PeerEvent;
using finalmq::ProtocolHeaderBinarySize;
using finalmq::RemoteEntityContentType;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::remoteentity::Status;
using finalmq::Logger;
using finalmq::LogContext;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Sex;
using helloworld::Person;
using helloworld::Address;



int main()
{
    // display log traces
    Logger::instance().registerConsumer([] (const LogContext& context, const char* text) {
        std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
    });

    // Create and initialize entity container. Entities can be added with registerEntity().
    // Entities are like remote objects, but they can be at the same time client and server.
    // This means, an entity can send (client) and receive (server) a request command.
    RemoteEntityContainer entityContainer;
    entityContainer.init();

    // run entity container in separate thread
    std::thread thread([&entityContainer] () {
        entityContainer.run();
    });

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session->getConnectionData();
        std::cout << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString() << std::endl;
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    RemoteEntity entityClient;
    entityContainer.registerEntity(&entityClient);

    // register peer events to see when a remote entity connects or disconnects.
    entityClient.registerPeerEvent([] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
        std::cout << "peer event " << peerEvent.toString() << std::endl;
    });

    // connect to port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    // note: Also multiple connects are possible.
    // And by the way, also bind()s are possible. An EntityContainer can be client and server at the same time.
    // A client can be started before the server is started. The connect is been retried in the background till the server
    // becomes available. Use the ConnectProperties to change the reconnect properties
    // (default is: try to connect every 5s forever till the server becomes available).
    IProtocolSessionPtr sessionClient = entityContainer.connect("tcp://localhost:7777", std::make_shared<ProtocolHeaderBinarySize>(), RemoteEntityContentType::CONTENTTYPE_PROTO);

    // connect entityClient to remote server entity "MyService" with the created TCP session.
    // The returned peerId identifies the peer entity.
    // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
    PeerId peerId = entityClient.connect(sessionClient, "MyService", [] (PeerId peerId, Status status) {
        std::cout << "connect reply: " << status.toString() << std::endl;
    });

    // asynchronous request/reply
    // A peer entity is been identified by its peerId.
    // each request has its own lambda. The lambda is been called when the corresponding reply is received.
    entityClient.requestReply<HelloReply>(peerId,
                HelloRequest{{ {"Bonnie","Parker",Sex::FEMALE,1910,{"somestreet",   12,76875,"Rowena","USA"}},
                               {"Clyde", "Barrow",Sex::MALE,  1909,{"anotherstreet",32,37385,"Telico","USA"}} }},
                [] (PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
        if (reply)
        {
            std::cout << "REPLY: ";
            std::for_each(reply->greetings.begin(), reply->greetings.end(), [] (const auto& entry) {
                std::cout << entry << ". ";
            });
            std::cout << std::endl;
        }
        else
        {
            std::cout << "REPLY error: " << status.toString() << std::endl;
        }
    });

    // another request/reply
    entityClient.requestReply<HelloReply>(peerId,
                HelloRequest{{ {"Albert","Einstein",Sex::FEMALE,1879,{"somestreet",   12,89073, "Ulm",    "Germany"}},
                               {"Marie", "Curie",   Sex::FEMALE,1867,{"anotherstreet",32,00001,"Warschau","Poland"}},
                               {"World", "",        Sex::DIVERSE,0,{}} }},
                [] (PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
        if (reply)
        {
            std::cout << "REPLY: - ";
            std::for_each(reply->greetings.begin(), reply->greetings.end(), [] (const auto& entry) {
                std::cout << entry << " - ";
            });
            std::cout << std::endl;
        }
        else
        {
            std::cout << "REPLY error: " << status.toString() << std::endl;
        }
    });

    // wait 20s
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));

    // release the thread
    entityContainer.terminatePollerLoop(1000);
    thread.join();

    return 0;
}
