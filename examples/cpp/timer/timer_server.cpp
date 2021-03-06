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

//#include "finalmq/remoteentity/RemoteEntityFormatProto.h"
//#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
//using finalmq::RemoteEntityFormatProto;
//using finalmq::RemoteEntityFormatJson;
//RemoteEntityFormatJson::CONTENT_TYPE
//RemoteEntityFormatProto::CONTENT_TYPE

#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatProto.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/remoteentity/FmqRegistryClient.h"
#include "finalmq/protocols/ProtocolHeaderBinarySize.h"
#include "finalmq/protocols/ProtocolDelimiterLinefeed.h"
#include "finalmq/logger/Logger.h"

// the definition of the messages are in the file timer.fmq
#include "timer.fmq.h"

#include <iostream>
#include <thread>
#include <iomanip>
#include <sstream>


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatProto;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::PeerEvent;
using finalmq::ReplyContextUPtr;
using finalmq::ProtocolHeaderBinarySizeFactory;
using finalmq::ProtocolDelimiterLinefeedFactory;
using finalmq::IProtocolSessionPtr;
using finalmq::ConnectionData;
using finalmq::ConnectionEvent;
using finalmq::FmqRegistryClient;
using finalmq::Logger;
using finalmq::LogContext;
using timer::StartRequest;
using timer::StopRequest;
using timer::TimerEvent;



#define MODULENAME  "timer_server"



/**
 * Generate a UTC ISO8601-formatted timestamp
 * and return as std::string
 */
std::string currentISO8601TimeUTC()
{
  auto now = std::chrono::system_clock::now();
  auto itt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream ss;
  ss << std::put_time(gmtime(&itt), "%FT%TZ");
  return ss.str();
}

class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
            std::cout << "peer event " << peerEvent.toString() << std::endl;
        });

        registerCommand<StartRequest>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<StartRequest>& request) {
            assert(request);
            m_timerActive = true;
        });

        registerCommand<StopRequest>([this] (ReplyContextUPtr& replyContext, const std::shared_ptr<StopRequest>& request) {
            assert(request);
            m_timerActive = false;
        });
    }

    void startThread()
    {
        m_thread = std::thread([this] () {

            while (true)
            {
                // send event every 1 second
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                // send, only if active
                if (m_timerActive)
                {
                    // get current time string
                    std::string strTime = currentISO8601TimeUTC();

                    // timer event
                    TimerEvent timerEvent{strTime};

                    // send timer event to all connected peers. No reply expected.
                    std::vector<PeerId> peers = getAllPeers();
                    for (size_t i = 0; i < peers.size(); ++i)
                    {
                        streamInfo << "sendEvent " << timerEvent.time;
                        sendEvent(peers[i], timerEvent);
                    }
                }
            }
        });
    }

private:

    bool            m_timerActive = true;
    std::thread     m_thread;
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

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session->getConnectionData();
        std::cout << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString() << std::endl;
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    EntityServer entityServer;
    entityServer.startThread();
    entityContainer.registerEntity(&entityServer, "TimerEntity");

    // Open listener port 7711 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    entityContainer.bind("tcp://*:7711", std::make_shared<ProtocolHeaderBinarySizeFactory>(), RemoteEntityFormatProto::CONTENT_TYPE);

    // Open listener port 8811 with delimiter framing protocol ProtocolDelimiterLinefeed ('\n' is end of frame).
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8811", std::make_shared<ProtocolDelimiterLinefeedFactory>(), RemoteEntityFormatJson::CONTENT_TYPE);

    // note:
    // multiple access points (listening ports) can be activated by calling bind() several times.
    // For Unix Domain Sockets use: "ipc://socketname"
    // For SSL/TLS encryption use BindProperties e.g.:
    // entityContainer->bind("tcp://*:7711", std::make_shared<ProtocolHeaderBinarySizeFactory>(),
    //                       RemoteEntityFormatProto::CONTENT_TYPE,
    //                       {{true, "myservercertificate.cert.pem", "myservercertificate.key.pem"}});
    // And by the way, also connect()s are possible for an EntityContainer. An EntityContainer can be client and server at the same time.


    FmqRegistryClient fmqRegistryClient(&entityContainer);
    fmqRegistryClient.registerService({"TimerService", "TimerEntity", finalmq::ENTITYID_INVALID,
                                       {{finalmq::fmqreg::SocketProtocol::SOCKET_TCP, finalmq::ProtocolHeaderBinarySize::PROTOCOL_ID, RemoteEntityFormatProto::CONTENT_TYPE, false, "tcp://*:7711"},
                                        {finalmq::fmqreg::SocketProtocol::SOCKET_TCP, finalmq::ProtocolDelimiterLinefeed::PROTOCOL_ID,        RemoteEntityFormatJson::CONTENT_TYPE,  false, "tcp://*:8811"}}});

    // run
    entityContainer.run();

    return 0;
}
