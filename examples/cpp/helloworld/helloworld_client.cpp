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
#include "finalmq/protocols/ProtocolMqtt5Client.h"
#include "finalmq/helpers/Executor.h"
#include "finalmq/helpers/ProtothreadRequestReply.h"

// the definition of the messages are in the file helloworld.fmq
#include "helloworld.fmq.h"

#include <iostream>
#include <thread>
#include <algorithm>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "helloworld_server"


using finalmq::RemoteEntity;
using finalmq::IRemoteEntity;
using finalmq::IRemoteEntityPtr;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatProto;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::EntityId;
using finalmq::PeerEvent;
using finalmq::SessionInfo;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::Status;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::VariantStruct;
using finalmq::ProtocolMqtt5Client;
using finalmq::IExecutorPtr;
using finalmq::Executor;
using finalmq::ExecutorWorker;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Gender;
using helloworld::Person;
using helloworld::Address;


#ifdef WIN32
    #define LOOP_PARALLEL   10000
    #define LOOP_SEQUENTIAL 10000
#else
    #define LOOP_PARALLEL   100000
    #define LOOP_SEQUENTIAL 10000
#endif

finalmq::CondVar g_performanceTestFinished{};

void triggerRequest(IRemoteEntity& entityClient, PeerId peerId, const std::chrono::time_point<std::chrono::steady_clock>& starttime, int index)
{
    entityClient.requestReply<HelloReply>(peerId,
        HelloRequest{ { {"Bonnie","Parker",Gender::FEMALE,1910,{"somestreet", 12,76875,"Rowena","USA"}} } },
        [&entityClient, &starttime, index](PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
            if (reply)
            {
                if (index == LOOP_SEQUENTIAL - 1)
                {
                    auto now = std::chrono::steady_clock::now();
                    std::chrono::duration<double> dur = now - starttime;
                    long long delta = static_cast<long long>(dur.count() * 1000);
                    std::cout << "time for " << LOOP_SEQUENTIAL << " sequential requests: " << delta << "ms" << std::endl;
                    g_performanceTestFinished = true;
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



//#define MULTITHREADED
//#define TWOCONNECTIONS

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

#ifndef MULTITHREADED
    entityContainer.init();
    // run entity container in separate thread
    std::thread thread([&entityContainer] () {
        entityContainer.run();
    });
#else
    // If you want that the commands and events shall be executed in extra threads,
    // then call entityContainer.init with an executor.
    ExecutorWorker<Executor> worker(4);
    entityContainer.init(worker.getExecutor());
#endif


    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const SessionInfo& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session.getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create client entity
    IRemoteEntityPtr entityClientPtr = std::make_shared<RemoteEntity>();
    IRemoteEntity& entityClient = *entityClientPtr;

    // register peer events to see when a remote entity connects or disconnects.
    entityClient.registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
        streamInfo << "peer event " << peerEvent.toString();
    });

    // connect to port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    // note: Also multiple connects are possible.
    // And by the way, also bind()s are possible. An EntityContainer can be client and server at the same time.
    // A client can be started before the server is started. The connect is been retried in the background till the server
    // becomes available. Use the ConnectProperties to change the reconnect properties
    // (default is: try to connect every 5s forever till the server becomes available).
    SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7777:headersize:protobuf");
    //SessionInfo sessionClient2 = entityContainer.connect("tcp://localhost:7777:headersize:protobuf");
    //SessionInfo sessionClient = entityContainer.connect("ipc://my_uds:headersize:protobuf");

#ifdef TWOCONNECTIONS
    SessionInfo sessionClient2 = entityContainer.connect("ipc://my_uds:headersize:protobuf");
#endif

    //// if you want to use mqtt5 -> connect to broker
    //SessionInfo sessionClient = entityContainer.connect("tcp://localhost:1883:mqtt5client:json", { {},{},
    //    VariantStruct{  //{ProtocolMqtt5Client::KEY_USERNAME, std::string("")},
    //                    //{ProtocolMqtt5Client::KEY_PASSWORD, std::string("")},
    //                    {ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL, 300},
    //                    {ProtocolMqtt5Client::KEY_KEEPALIVE, 20},
    //    } });

    // connect entityClient to remote server entity "MyService" with the created TCP session.
    // The returned peerId identifies the peer entity.
    // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
    PeerId peerId = entityClient.connect(sessionClient, "MyService", [](PeerId peerId, Status status) {
        streamInfo << "connect reply: " << status.toString();
    });

#ifdef TWOCONNECTIONS
    PeerId peerId2 = entityClient.connect(sessionClient2, "MyService", [](PeerId peerId, Status status) {
        streamInfo << "connect reply: " << status.toString();
        });
#endif

    // asynchronous request/reply
    // A peer entity is been identified by its peerId.
    // each request has its own lambda. The lambda is been called when the corresponding reply is received.
    entityClient.requestReply<HelloReply>(peerId,
                HelloRequest{{ {"Bonnie","Parker",Gender::FEMALE,1910,{"somestreet",   12,76875,"Rowena","USA"}},
                               {"Clyde", "Barrow",Gender::MALE,  1909,{"anotherstreet",32,37385,"Telico","USA"}} }},
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
                HelloRequest{{ {"Albert","Einstein",Gender::FEMALE,1879,{"somestreet",   12,89073, "Ulm",    "Germany"}},
                               {"Marie", "Curie",   Gender::FEMALE,1867,{"anotherstreet",32,00001,"Warschau","Poland"}},
                               {"World", "",        Gender::DIVERSE,0,{}} }},
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

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    for (int i = 0; i < 10; ++i)
    {
        // performance measurement of throughput
        auto starttime = std::chrono::steady_clock::now();
        for (int i = 0; i < LOOP_PARALLEL; ++i)
        {
            // asynchronous request/reply
            // A peer entity is been identified by its peerId.
            // each request has its own lambda. The lambda is been called when the corresponding reply is received.
            entityClient.requestReply<HelloReply>(peerId,
                        HelloRequest{{ {"Bonnie","Parker",Gender::FEMALE,1910,{"somestreet", 12,76875,"Rowena","USA"}} }},
                        [i, starttime] (PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
                if (reply)
                {
                    if (i == LOOP_PARALLEL-1)
                    {
                        auto now = std::chrono::steady_clock::now();
                        std::chrono::duration<double> dur = now - starttime;
                        long long delta = static_cast<long long>(dur.count() * 1000);
                        std::cout << "time for " << LOOP_PARALLEL << " parallel requests: " << delta << "ms" << std::endl;
                        g_performanceTestFinished = true;
                    }
                }
                else
                {
                    std::cout << "REPLY error: " << status.toString() << std::endl;
                }
            });


#ifdef TWOCONNECTIONS
            i++;
            entityClient.requestReply<HelloReply>(peerId2,
                HelloRequest{ { {"Bonnie","Parker",Gender::FEMALE,1910,{"somestreet", 12,76875,"Rowena","USA"}} } },
                [i, starttime](PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
                    if (reply)
                    {
                        if (i == LOOP_PARALLEL - 1)
                        {
                            auto now = std::chrono::steady_clock::now();
                            std::chrono::duration<double> dur = now - starttime;
                            long long delta = static_cast<long long>(dur.count() * 1000);
                            std::cout << "time for " << LOOP_PARALLEL << " parallel requests: " << delta << "ms" << std::endl;
                            g_performanceTestFinished = true;
                        }
                    }
                    else
                    {
                        std::cout << "REPLY error: " << status.toString() << std::endl;
                    }
                });
#endif

        }
        g_performanceTestFinished.wait();
    }

    // performance measurement of latency
    auto starttime = std::chrono::steady_clock::now();
    triggerRequest(entityClient, peerId, starttime, 0);

    g_performanceTestFinished.wait();



    starttime = std::chrono::steady_clock::now();
    finalmq::CondVar condvar;
    struct ProtoThreadState
    {
        int i = 0;
        std::shared_ptr<HelloReply> reply;
    } protoThreadState;
    finalmq::ProtothreadRequestReply protothread([&entityClient, peerId, protoThreadState](finalmq::ProtothreadRequestReply& protothread) mutable {
        PT_BEGIN(protothread);

        for (; protoThreadState.i < LOOP_SEQUENTIAL; ++protoThreadState.i)
        {
            protothread.sendRequest(&entityClient, peerId, HelloRequest{ { {"Bonnie","Parker",Gender::FEMALE,1910,{"somestreet", 12,76875,"Rowena","USA"}} } });
            WAIT_FOR_REPLY(protothread);
            auto reply = protothread.reply<HelloReply>();
        }

        PT_END(protothread);
    }, [&condvar]() { condvar = true; });
    protothread.Run();
    condvar.wait();
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> dur = now - starttime;
    long long delta = static_cast<long long>(dur.count() * 1000);
    std::cout << "time for " << LOOP_SEQUENTIAL << " sequential requests: " << delta << "ms" << std::endl;


#ifndef MULTITHREADED
    // release the thread
    entityContainer.terminatePollerLoop();
    thread.join();
#endif

    return 0;
}
