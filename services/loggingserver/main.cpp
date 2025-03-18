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

#include "finalmq/interfaces/fmqlog.fmq.h"
#include "finalmq/interfaces/fmqprocess.fmq.h"

#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/EntityFileService.h"

#include <iostream>

#define MODULENAME	"LoggingServer"

using namespace finalmq;
using namespace finalmq::fmqprocess;


class LoggingServer : public RemoteEntity
{
public:
	LoggingServer()
	{
		// register peer events to see when a remote entity connects or disconnects.
		registerPeerEvent([this](PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
			streamInfo << "peer event " << peerEvent.toString();
		});

		registerCommand<LogEntry>("log", [this](const RequestContextPtr& requestContext, const std::shared_ptr<LogEntry>& request) {
			sendEventToAllPeers("log", *request);
		});

		registerCommand<NoData>("pollentity", [](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
			requestContext->reply(PollEntityReply("", RecoverMode::RECOVERMODE_NONE));
		});
	}
};


int main(int argc, char* argv[])
{
	// display log traces
	Logger::instance().registerConsumer([](const LogContext& context, const char* text) {
		std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
	});

	{
		RemoteEntityContainer entityContainer;
		entityContainer.init();

		// register lambda for connection events to see when a network node connects or disconnects.
		entityContainer.registerConnectionEvent([](const SessionInfo& session, ConnectionEvent connectionEvent) {
			const ConnectionData connectionData = session.getConnectionData();
			streamInfo << "connection event at " << connectionData.endpoint
				<< " remote: " << connectionData.endpointPeer
				<< " event: " << connectionEvent.toString();
		});

		LoggingServer loggingServer;
		entityContainer.registerEntity(&loggingServer, "LoggingServer");

		EntityFileServer entityFileServer("htdocs");
		entityContainer.registerEntity(&entityFileServer, "*");

		entityContainer.bind("tcp://*:9200:headersize:protobuf");
		entityContainer.bind("tcp://*:9201:delimiter_lf:json");
		entityContainer.bind("tcp://*:9280:httpserver:json");

		entityContainer.run();
	}

	return 0;
}
