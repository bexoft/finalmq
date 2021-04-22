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


#include "finalmq/protocolconnection/ProtocolSessionContainer.h"
#include "finalmq/protocols/ProtocolHttp.h"
#include "finalmq/variant/Variant.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/logger/Logger.h"


#include <iostream>
#include <thread>


using finalmq::IMessagePtr;
using finalmq::IProtocolSessionCallback;
using finalmq::ProtocolSessionContainer;
using finalmq::ProtocolHttp;
using finalmq::ProtocolHttpFactory;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::Variant;



class HttpServer : public IProtocolSessionCallback
{
public:
    HttpServer()
    {
    }

private:
    // IProtocolSessionCallback
    virtual void connected(const IProtocolSessionPtr& session)
    {

    }

    virtual void disconnected(const IProtocolSessionPtr& session)
    {

    }

    virtual void received(const IProtocolSessionPtr& session, const IMessagePtr& message)
    {
        IMessagePtr response = session->createMessage();
        response->addMetainfo(ProtocolHttp::FMQ_HTTP, ProtocolHttp::HTTP_RESPONSE);
        response->addMetainfo(ProtocolHttp::FMQ_STATUS, "200");
        response->addMetainfo(ProtocolHttp::FMQ_STATUSTEXT, "OK");
        const std::string* path = message->getMetainfo(ProtocolHttp::FMQ_PATH);
        if (path)
        {
            response->addMetainfo("Content-Length", Variant(path->size()));
            response->addSendPayload(*path);
        }
        else
        {
            response->addMetainfo("Content-Length", "0");
        }
        session->sendMessage(response);
    }

    virtual void socketConnected(const IProtocolSessionPtr& session)
    {

    }

    virtual void socketDisconnected(const IProtocolSessionPtr& session)
    {

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
    ProtocolSessionContainer sessionContainer;
    sessionContainer.init();

    //// register lambda for connection events to see when a network node connects or disconnects.
    //entityContainer.registerConnectionEvent([] (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) {
    //    const ConnectionData connectionData = session->getConnectionData();
    //    std::cout << "connection event at " << connectionData.endpoint
    //              << " remote: " << connectionData.endpointPeer
    //              << " event: " << connectionEvent.toString() << std::endl;
    //});

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    HttpServer server;

    // Open listener port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    sessionContainer.bind("tcp://*:7777", &server, std::make_shared<ProtocolHttpFactory>());


    // run
    sessionContainer.run();

    return 0;
}
