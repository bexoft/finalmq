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
#include "finalmq/remoteentity/FmqRegistryClient.h"
#include "finalmq/logger/Logger.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/protocols/ProtocolMqtt5.h"

#include "finalmq/helpers/ModulenameFinalmq.h"

// the definition of the messages are in the file timer.fmq
#include "timer.fmq.h"

#include <iostream>
#include <thread>
#include <algorithm>


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::EntityId;
using finalmq::PeerEvent;
using finalmq::RequestContextPtr;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::remoteentity::Status;
using finalmq::FmqRegistryClient;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::VariantStruct;
using finalmq::ProtocolMqtt5;
using timer::TimerEvent;



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

    // Create client entity and register it at the entityContainer
    // note: multiple entities can be registered.
    RemoteEntity entityClient;

    // It is possible to register a command in the constructor of a derived RemoteEntity.
    // But here, it is shown that also a register is possible from outside.
    entityClient.registerCommand<TimerEvent>([] (const RequestContextPtr& requestContext, const std::shared_ptr<TimerEvent>& request) {
        std::cout << "time: " << request->time << std::endl;
    });
    EntityId entityId = entityContainer.registerEntity(&entityClient);

    // register peer events to see when a remote entity connects or disconnects.
    entityClient.registerPeerEvent([] (PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
        std::cout << "peer event " << peerEvent.toString() << std::endl;
    });

    // connect to port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    // note: Also multiple connects are possible.
    // And by the way, also bind()s are possible. An EntityContainer can be client and server at the same time.
    // A client can be started before the server is started. The connect is been retried in the background till the server
    // becomes available. Use the ConnectProperties to change the reconnect properties
    // (default is: try to connect every 5s forever till the server becomes available).
    IProtocolSessionPtr sessionClient = entityContainer.connect("tcp://localhost:7777:headersize:protobuf");

    // use mqtt5 -> connect to broker
    //IProtocolSessionPtr sessionClient = entityContainer.connect("tcp://localhost:1883:mqtt5client:json", { {},{},
    //    VariantStruct{  //{ProtocolMqtt5::KEY_USERNAME, std::string("")},
    //                    //{ProtocolMqtt5::KEY_PASSWORD, std::string("")},
    //                    {ProtocolMqtt5::KEY_SESSIONEXPIRYINTERVAL, 30},
    //                    {ProtocolMqtt5::KEY_KEEPALIVE, 20},
    //    } });

    // connect entityClient to remote server entity "MyService" with the created TCP session.
    // The returned peerId identifies the peer entity.
    // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
    /*PeerId peerId =*/ entityClient.connect(sessionClient, "MyService", [](PeerId peerId, Status status) {
        streamInfo << "connect reply: " << status.toString();
        });

    // wait 20s
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));

    // release the thread
    entityContainer.terminatePollerLoop();
    thread.join();

    return 0;
}
