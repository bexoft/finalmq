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



#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatProto.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/logger/Logger.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/protocols/ProtocolMqtt5.h"

// the definition of the messages are in the file helloworld.fmq
#include "helloworld.fmq.h"

#include <iostream>
#include <thread>
#include <algorithm>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "helloworld_server"


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatProto;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::EntityId;
using finalmq::PeerEvent;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::remoteentity::Status;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::VariantStruct;
using finalmq::ProtocolMqtt5;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Sex;
using helloworld::Person;
using helloworld::Address;



#define LOOP 1000

void triggerRequest(RemoteEntity& entityClient, PeerId peerId, const std::chrono::time_point<std::chrono::system_clock>& starttime, int index)
{
    entityClient.requestReply<HelloReply>(peerId,
        HelloRequest{ { {"Bonnie","Parker",Sex::FEMALE,1910,{"somestreet", 12,76875,"Rowena","USA"}} } },
        [&entityClient, peerId, &starttime, index](PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
            if (reply)
            {
                if (index == LOOP - 1)
                {
                    auto now = std::chrono::system_clock::now();
                    std::chrono::duration<double> dur = now - starttime;
                    long long delta = static_cast<long long>(dur.count() * 1000);
                    std::cout << "time for " << LOOP << " sequential requests: " << delta << "ms" << std::endl;
                }
                else
                {
                    triggerRequest(entityClient, peerId, starttime, index + 1);
                }
            }
            else
            {
                std::cout << "REPLY error: " << status.toString() << std::endl;
            }
        });
}



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
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create client entity and register it at the entityContainer
    // note: multiple entities can be registered.
    RemoteEntity entityClient;
    entityContainer.registerEntity(&entityClient);

    // register peer events to see when a remote entity connects or disconnects.
    entityClient.registerPeerEvent([] (PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
        streamInfo << "peer event " << peerEvent.toString();
    });

    // connect to port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    // note: Also multiple connects are possible.
    // And by the way, also bind()s are possible. An EntityContainer can be client and server at the same time.
    // A client can be started before the server is started. The connect is been retried in the background till the server
    // becomes available. Use the ConnectProperties to change the reconnect properties
    // (default is: try to connect every 5s forever till the server becomes available).
    IProtocolSessionPtr sessionClient = entityContainer.connect("tcp://localhost:7777:headersize:protobuf");
//    IProtocolSessionPtr sessionClient = entityContainer.connect("ipc://my_uds:headersize:protobuf");

    // use mqtt5 -> connect to broker
    //IProtocolSessionPtr sessionClient = entityContainer.connect("tcp://localhost:1883:mqtt5client:json", { {},{},
    //    VariantStruct{  //{ProtocolMqtt5::KEY_USERNAME, std::string("")},
    //                    //{ProtocolMqtt5::KEY_PASSWORD, std::string("")},
    //                    {ProtocolMqtt5::KEY_SESSIONEXPIRYINTERVAL, 300},
    //                    {ProtocolMqtt5::KEY_KEEPALIVE, 20},
    //    } });

    // connect entityClient to remote server entity "MyService" with the created TCP session.
    // The returned peerId identifies the peer entity.
    // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
    PeerId peerId = entityClient.connect(sessionClient, "MyService", [] (PeerId peerId, Status status) {
        streamInfo << "connect reply: " << status.toString();
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

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    for (int i = 0; i < 10; ++i)
    {
        // performance measurement of throughput
        auto starttime = std::chrono::system_clock::now();
        for (int i = 0; i < LOOP; ++i)
        {
            // asynchronous request/reply
            // A peer entity is been identified by its peerId.
            // each request has its own lambda. The lambda is been called when the corresponding reply is received.
            entityClient.requestReply<HelloReply>(peerId,
                        HelloRequest{{ {"Bonnie","Parker",Sex::FEMALE,1910,{"somestreet", 12,76875,"Rowena","USA"}} }},
                        [i, starttime] (PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
                if (reply)
                {
                    if (i == LOOP-1)
                    {
                        auto now = std::chrono::system_clock::now();
                        std::chrono::duration<double> dur = now - starttime;
                        long long delta = static_cast<long long>(dur.count() * 1000);
                        std::cout << "time for " << LOOP << " parallel requests: " << delta << "ms" << std::endl;
                    }
                }
                else
                {
                    std::cout << "REPLY error: " << status.toString() << std::endl;
                }
            });
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    }

    // performance measurement of latency
    auto starttime = std::chrono::system_clock::now();
    triggerRequest(entityClient, peerId, starttime, 0);

    // wait 20s
    std::this_thread::sleep_for(std::chrono::milliseconds(200000));

    // release the thread
    entityContainer.terminatePollerLoop();
    thread.join();

    return 0;
}
