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

// the definition of the messages are in the file restapi.fmq
#include "restapi.fmq.h"

#include <iostream>
#include <thread>
#include <algorithm>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "restapi_server"


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatProto;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainer;
using finalmq::RequestContextPtr;
using finalmq::PeerId;
using finalmq::EntityId;
using finalmq::PeerEvent;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::Status;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::VariantStruct;
using finalmq::ProtocolMqtt5Client;
using finalmq::IMessage;
using finalmq::NoData;
using restapi::Gender;
using restapi::Address;
using restapi::Person;
using restapi::PersonList;
using restapi::PersonId;
using restapi::PersonChanged;




void triggerDeletePerson(RemoteEntity& entityClient, PeerId peerId, const std::string& id)
{
    entityClient.requestReply<NoData>(peerId, "persons/" + id + "/DELETE", NoData{},
        [id](PeerId peerId, Status status, const std::shared_ptr<NoData>& reply)
    {
        if (reply)
        {
            std::cout << "person deleted, id:" << id << std::endl;
        }
        else
        {
            std::cout << "REPLY error: " << status.toString() << std::endl;
        }
    });
}




void triggerGetPersons(RemoteEntity& entityClient, PeerId peerId, const std::string& id)
{
    entityClient.requestReply<PersonList>(peerId, "persons/GET", { {"QUERY_filter", "Bon"}}, NoData{},
        [&entityClient, id](PeerId peerId, Status status, IMessage::Metainfo& metainfo, const std::shared_ptr<PersonList>& reply)
    {
        if (reply)
        {
            for (size_t i = 0; i < reply->persons.size(); ++i)
            {
                const Person& person = reply->persons[i];
                std::cout << person.id << " " << person.name << " " << person.surname << std::endl;
            }
            triggerDeletePerson(entityClient, peerId, id);
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

    entityClient.registerCommand<PersonChanged>("events/personChanged",
        [](const RequestContextPtr& requestContext, const std::shared_ptr<PersonChanged>& request)
    {
        std::cout << "person changed event: " << request->changeType.toString() << ", id:" << request->person.id << ", "
                  << " " << request->person.name << " " << request->person.surname << std::endl;
    });

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

    //// if you want to use mqtt5 -> connect to broker
    //IProtocolSessionPtr sessionClient = entityContainer.connect("tcp://broker.emqx.io:1883:mqtt5client:json", { {},{},
    //    VariantStruct{  //{ProtocolMqtt5Client::KEY_USERNAME, std::string("")},
    //                    //{ProtocolMqtt5Client::KEY_PASSWORD, std::string("")},
    //                    {ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL, 300},
    //                    {ProtocolMqtt5Client::KEY_KEEPALIVE, 20},
    //} });

    // connect entityClient to remote server entity "MyService" with the created TCP session.
    // The returned peerId identifies the peer entity.
    // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
    PeerId peerId = entityClient.connect(sessionClient, "MyService", [] (PeerId peerId, Status status) {
        streamInfo << "connect reply: " << status.toString();
    });


    // asynchronous request/reply
    // A peer entity is been identified by its peerId.
    // each request has its own lambda. The lambda is been called when the corresponding reply is received.
    entityClient.requestReply<PersonId>(peerId, "persons/POST",
        Person{ {}, "Bonnie", "Parker", Gender::FEMALE, 1910, {"somestreet", 12,76875, "Rowena", "USA"} },
        [&entityClient](PeerId peerId, Status status, const std::shared_ptr<PersonId>& reply)
    {
        if (reply)
        {
            std::cout << "person added, id:" << reply->id << std::endl;
            triggerGetPersons(entityClient, peerId, reply->id);
        }
        else
        {
            std::cout << "REPLY error: " << status.toString() << std::endl;
        }
    });

    // wait 20s
    std::this_thread::sleep_for(std::chrono::milliseconds(200000));

    // release the thread
    entityContainer.terminatePollerLoop();
    thread.join();

    return 0;
}
