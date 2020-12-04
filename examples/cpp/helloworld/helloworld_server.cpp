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
#include "helloworld.fmq.h"

using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::PeerEvent;
using finalmq::ReplyContextUPtr;
using finalmq::ProtocolHeaderBinarySizeFactory;
using finalmq::ProtocolDelimiterFactory;
using finalmq::RemoteEntityContentType;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::Logger;
using finalmq::LogContext;
using helloworld::HelloRequest;
using helloworld::HelloReply;

#include <iostream>
#include <thread>


class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
            std::cout << "peer event " << peerEvent.toString() << std::endl;
        });

        // json example: /MyService/helloworld.HelloRequest#1{"name":"World"}
        registerCommand<HelloRequest>([] (ReplyContextUPtr& replyContext, const std::shared_ptr<HelloRequest>& request) {
            assert(request);

            // The replyContext is a unique_ptr, it can be moved to another unique_ptr, so that the reply can be called later.
            std::string prefix("Hello ");
            replyContext->reply(HelloReply(prefix + request->name));
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
    std::shared_ptr<IRemoteEntityContainer> entityContainer = std::make_shared<RemoteEntityContainer>();
    entityContainer->init();

    entityContainer->registerConnectionEvent([] (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session->getConnectionData();
        std::cout << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString() << std::endl;
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    EntityServer entityServer;
    entityContainer->registerEntity(&entityServer, "MyService");

    // Open listener port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    entityContainer->bind("tcp://*:7777", std::make_shared<ProtocolHeaderBinarySizeFactory>(), RemoteEntityContentType::CONTENTTYPE_PROTO);

    // Open listener port 8888 with delimiter framing protocol ProtocolDelimiter ('\n' is end of frame).
    // content type in payload: JSON
    entityContainer->bind("tcp://*:8888", std::make_shared<ProtocolDelimiterFactory>("\n"), RemoteEntityContentType::CONTENTTYPE_JSON);

    // note:
    // multiple access points (listening ports) can be activated by calling bind() several times.
    // For Unix Domain Sockets use: "ipc://socketname"
    // For SSL/TLS encryption use BindProperties e.g.:
    // entityContainer->bind("tcp://*:7777", std::make_shared<ProtocolHeaderBinarySizeFactory>(),
    //                       RemoteEntityContentType::CONTENTTYPE_PROTO,
    //                       {{true, "myservercertificate.cert.pem", "myservercertificate.key.pem"}});
    // And by the way, also connects()s are possible for an EntityContainer. An EntityContainer can be client and server at the same time.

    // run
    entityContainer->run();

    return 0;
}
