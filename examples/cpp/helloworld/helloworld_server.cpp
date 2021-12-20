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
#include "finalmq/remoteentity/EntityFileService.h"
#include "finalmq/logger/Logger.h"
#include "finalmq/helpers/Executor.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/protocols/ProtocolMqtt5Client.h"

// the definition of the messages are in the file helloworld.fmq
#include "helloworld.fmq.h"

#include <iostream>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "helloworld_server"


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::IRemoteEntityContainer;
using finalmq::EntityFileServer;
using finalmq::PeerId;
using finalmq::EntityId;
using finalmq::PeerEvent;
using finalmq::RequestContextPtr;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::IExecutorPtr;
using finalmq::Executor;
using finalmq::VariantStruct;
using finalmq::ProtocolMqtt5Client;
using helloworld::HelloRequest;
using helloworld::HelloReply;



class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([] (PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
            streamInfo << "peer event " << peerEvent.toString();
        });

        // handle the HelloRequest
        // this is fun - try to access the server with the json interface at port 8888:
        // telnet localhost 8888  (or: netcat localhost 8888) and type:
        //   /MyService/helloworld.HelloRequest!4711{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
        // or open a browser and type:
        //   localhost:8080/MyService/helloworld.HelloRequest{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
        // or use a HTTP client and do an HTTP request (the method GET, POST, ... does not matter) to localhost:8080 with:
        //   /MyService/helloworld.HelloRequest
        //   and payload: {"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
        registerCommand<HelloRequest>([] (const RequestContextPtr& requestContext, const std::shared_ptr<HelloRequest>& request) {
            assert(request);

            // prepare the reply
            std::string prefix("Hello ");
            HelloReply reply;
            // go through all persons and make a greeting
            for (size_t i = 0; i < request->persons.size(); ++i)
            {
                reply.greetings.emplace_back(prefix + request->persons[i].name);
            }

//            PeerId peerid = requestContext->peerId();

            // send reply
            requestContext->reply(std::move(reply));

            // note:
            // The reply does not have to be sent immediately:
            // The requestContext is a shared_ptr, store it and reply later.

            // note:
            // The requestContext has the method requestContext->peerId()
            // The returned peerId can be used for calling requestReply() or sendEvent().
            // So, also a server entity can act as a client and can send requestReply()
            // to the peer entity that is calling this request.
            // An entity can act as a client and as a server. It is bidirectional (symmetric), like a socket.
        });

        
        // just to demonstrate REST API. You can access the service with either an HTTP PUT with path: "mypath/1234".
        // Or with "maypath/1234/PUT" and the HTTP method (like: GET, POST, PUT, ...) does not matter.
        // try to access the server with the json interface at port 8888:
        // telnet localhost 8888  (or: netcat localhost 8888) and type:
        //   /MyService/mypath/1234/PUT!4711{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
        // or open a browser and type:
        //   localhost:8080/MyService/mypath/1234/PUT{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
        // or use a HTTP client and do an HTTP PUT request to localhost:8080 with:
        //   /MyService/mypath/1234
        //   and payload: {"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
        registerCommand<HelloRequest>("mypath/{id}/PUT", [](const RequestContextPtr& requestContext, const std::shared_ptr<HelloRequest>& request) {
            assert(request);

            const std::string* id = requestContext->getMetainfo("PATH_id"); // when a field is defined as {keyname}, then you can get the value with the key prefix "PATH_" ("PATH_keyname"}

            // prepare the reply
            std::string prefix("Hello ");
            HelloReply reply;
            // go through all persons and make a greeting
            for (size_t i = 0; i < request->persons.size(); ++i)
            {
                reply.greetings.emplace_back(prefix + request->persons[i].name);
            }

            // send reply
            requestContext->reply(std::move(reply));
        });
    }
};





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

    // If you want that the commands and events shall be executed in extra threads, 
    // then call entityContainer.init with an executor.
    //IExecutorPtr executor = std::make_shared<Executor>();
    //std::vector<std::thread> threads;
    //for (int i = 0; i < 2; ++i)
    //{
    //    threads.emplace_back(std::thread([executor]() {
    //        executor->run();
    //    }));
    //}
    //entityContainer.init(executor);

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session->getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    EntityServer entityServer;
    entityContainer.registerEntity(&entityServer, "MyService");

    // register an entity for file download. The name "*" means that if an entity name, given by a client, is not found by name, 
    // then this entity will try to open a file inside the htdocs directory. An entity name can contain slashes ('/')
    EntityFileServer entityFileServer("htdocs");
    entityContainer.registerEntity(&entityFileServer, "*");

    // Open listener port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    entityContainer.bind("tcp://*:7777:headersize:protobuf");
//    entityContainer.bind("ipc://my_uds:headersize:protobuf");

    // Open listener port 8888 with delimiter framing protocol ProtocolDelimiterLinefeed ('\n' is end of frame).
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8888:delimiter_lf:json");

    // Open listener port 8080 with http.
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8080:httpserver:json");

    //// if you want to use mqtt5 -> connect to broker
    //entityContainer.connect("tcp://localhost:1883:mqtt5client:json", { {},{},
    //    VariantStruct{  //{ProtocolMqtt5Client::KEY_USERNAME, std::string("")},
    //                    //{ProtocolMqtt5Client::KEY_PASSWORD, std::string("")},
    //                    {ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL, 300},
    //                    {ProtocolMqtt5Client::KEY_KEEPALIVE, 20},
    //} });

    // note:
    // multiple access points (listening ports) can be activated by calling bind() several times.
    // For Unix Domain Sockets use: "ipc://socketname"
    // For SSL/TLS encryption use BindProperties e.g.:
    // entityContainer->bind("tcp://*:7777:headersize", 
    //                       {{true, SSL_VERIFY_NONE, "myservercertificate.cert", "myservercertificate.key"}});
    // And by the way, also connect()s are possible for an EntityContainer. An EntityContainer can be client and server at the same time.


    // run the entity container. this call blocks the execution. 
    // If you do not want to block, then execute run() in another thread
    entityContainer.run();

    //executor->terminate();
    //for (size_t i = 0; i < threads.size(); ++i)
    //{
    //    threads[i].join();
    //}

    return 0;
}
