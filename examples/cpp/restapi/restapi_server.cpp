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
#include "finalmq/remoteentity/entitydata.fmq.h"

// the definition of the messages are in the file restapi.fmq
#include "restapi.fmq.h"

#include <iostream>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "restapi_server"


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
using finalmq::remoteentity::NoData;
using restapi::Person;
using restapi::PersonList;
using restapi::PersonId;
using restapi::PersonChanged;



class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([] (PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
            streamInfo << "peer event " << peerEvent.toString();
        });

        // handle GET persons -> get all persons
        // try to access the server with the json/TCP interface at port 8888:
        // telnet localhost 8888  (or: netcat localhost 8888) and type:
        //   /MyService/persons/GET!4711
        //   with query parameter you have to use the following syntax. In the meta array you can put
        //   key/value pairs, the strings with even index are keys, the strings with odd index are the according values.
        //   The query parameter "filter" needs a prefix "QUERY_".
        //   [{"destname":"MyService","path":"persons/GET","corrid":"4711","meta":["QUERY_filter","Bon"]},{}]
        // or open a browser and type:
        //   localhost:8080/MyService/persons?filter=Bon
        // or use a HTTP client and do an HTTP GET request to localhost:8080 with: /MyService/persons?filter=Bon
        registerCommand<NoData>("persons/GET", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {

            std::string* filter = requestContext->getMetainfo("QUERY_filter");

            // send reply
            PersonList persons;
            for (auto it = m_persons.begin(); it != m_persons.end(); ++it)
            {
                if (!filter || filter->empty() ||
                    (it->second.name.compare(0, filter->size(), *filter) == 0) ||
                    (it->second.surname.compare(0, filter->size(), *filter) == 0))
                {
                    persons.persons.emplace_back(it->second);
                }
            }
            requestContext->reply(std::move(persons));
        });

        // handle POST persons -> add a person, it returns a person-ID.
        // try to access the server with the json/TCP interface at port 8888:
        // telnet localhost 8888  (or: netcat localhost 8888) and type:
        //   /MyService/persons/POST!4711{"name":"Bonnie"}
        // or open a browser and type:
        //   localhost:8080/MyService/persons/POST{"name":"Bonnie"}
        // or use a HTTP client and do an HTTP POST request to localhost:8080 with: /MyService/persons
        //   and payload: {"name":"Bonnie"}
        registerCommand<Person>("persons/POST", [this](const RequestContextPtr& requestContext, const std::shared_ptr<Person>& request) {
            if (request)
            {
                request->id = std::to_string(m_idNext);
                m_idNext++;
                m_persons[request->id] = *request;
                // send reply
                requestContext->reply(PersonId{ request->id });

                sendEventToAllPeers("events/personChanged", PersonChanged{ *request, restapi::ChangeType::ADDED });
            }
        });

        // handle PUT persons -> change a person. Use the person-ID to identify the person (replace <id> with the person-ID)
        // try to access the server with the json/TCP interface at port 8888:
        // telnet localhost 8888  (or: netcat localhost 8888) and type:
        //   /MyService/persons/<id>/PUT!4711{"name":"Clyde"}       example: /MyService/persons/1/PUT!4711{"name":"Clyde"}
        // or open a browser and type:
        //   localhost:8080/MyService/persons/<id>/PUT{"name":"Clyde"}
        // or use a HTTP client and do an HTTP PUT request to localhost:8080 with: /MyService/persons/<id>
        //   and payload: {"name":"Clyde"}
        registerCommand<Person>("persons/{id}/PUT", [this](const RequestContextPtr& requestContext, const std::shared_ptr<Person>& request) {
            if (request)
            {
                std::string* id = requestContext->getMetainfo("PATH_id");
                assert(id);
                request->id = *id;
                auto it = m_persons.find(*id);
                if (it != m_persons.end())
                {
                    it->second = *request;
                    requestContext->reply(finalmq::remoteentity::Status::STATUS_OK);

                    sendEventToAllPeers("events/personChanged", PersonChanged{ *request, restapi::ChangeType::CHANGED });
                }
                else
                {
                    requestContext->reply(finalmq::remoteentity::Status::STATUS_REQUEST_NOT_FOUND);
                }
            }
        });

        // handle DELETE persons -> delete a person. Use the person-ID to identify the person (replace <id> with the person-ID)
        // try to access the server with the json/TCP interface at port 8888:
        // telnet localhost 8888  (or: netcat localhost 8888) and type:
        //   /MyService/persons/<id>/DELETE!4711                example: /MyService/persons/1/DELETE!4711
        // or open a browser and type:
        //   localhost:8080/MyService/persons/<id>/DELETE
        // or use a HTTP client and do an HTTP DELETE request to localhost:8080 with: /MyService/persons/<id>
        registerCommand<NoData>("persons/{id}/DELETE", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
            std::string* id = requestContext->getMetainfo("PATH_id");
            assert(id);
            auto it = m_persons.find(*id);
            if (it != m_persons.end())
            {
                sendEventToAllPeers("events/personChanged", PersonChanged{ it->second, restapi::ChangeType::DELETED});
                m_persons.erase(it);
                requestContext->reply(finalmq::remoteentity::Status::STATUS_OK);
            }
            else
            {
                requestContext->reply(finalmq::remoteentity::Status::STATUS_REQUEST_NOT_FOUND);
            }
        });
    }

private:
    std::unordered_map<std::string, Person>  m_persons;
    int                                      m_idNext = 1;
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

    IExecutorPtr executor = std::make_shared<Executor>();
    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i)
    {
        threads.emplace_back(std::thread([executor]() {
            executor->run();
        }));
    }

    entityContainer.init();//executor);

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session->getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create server entity and register it at the entityContainer with the service name "MyService".
    // note: multiple entities can be registered
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

    std::this_thread::sleep_for(std::chrono::seconds(200000));

    executor->terminate();
    for (size_t i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }

    return 0;
}
