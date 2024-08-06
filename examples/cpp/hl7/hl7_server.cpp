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
#include "finalmq/remoteentity/RemoteEntityFormatHl7.h"
#include "finalmq/logger/Logger.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/protocols/ProtocolDelimiterX.h"

// the definition of the messages is in the file hl7dictionary.2.6.js.fmq
#include "hl7dictionary.2.6.js.fmq.h"

#include <iostream>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "hl7_server"


using namespace finalmq;


class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
            streamInfo << "peer event " << peerEvent.toString();
        });

        registerCommand<hl7::ORM_O01>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<hl7::ORM_O01>& request) {
            assert(request);

            hl7::ORM_O01 msg = *request;
            std::cout << "Request received #" << msg.msh.messageControlId << std::endl;

            // prepare the reply
            hl7::ACK reply;
            reply.msh.messageControlId = msg.msh.messageControlId;
            reply.msh.versionId = request->msh.versionId;
            reply.msh.countryCode = "de";

            // send reply
            requestContext->reply(std::move(reply));
            std::cout << "Reply sent #" << reply.msh.messageControlId << std::endl;
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

    EntityServer entityServer;

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([&entityServer] (const SessionInfo& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session.getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    entityContainer.registerEntity(&entityServer, "MyService");

    entityContainer.bind("tcp://*:7000:delimiter_x:hl7", { {}, 
        VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\x1C\x0D"} },
        VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},
                        {RemoteEntityFormatHl7::PROPERTY_ENTITY, "MyService"},
                        {RemoteEntityFormatHl7::PROPERTY_MESSAGESTART, "\x0B"} } });

    // run the entity container. this call blocks the execution. 
    // If you do not want to block, then execute run() in another thread
    entityContainer.run();

    return 0;
}
