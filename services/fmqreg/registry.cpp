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

#include "registry.h"
#include "finalmq/logger/LogStream.h"

#define MODULENAME "FmqRegistry"

using finalmq::PeerId;
using finalmq::IProtocolSessionPtr;
using finalmq::EntityId;
using finalmq::PeerEvent;
using finalmq::RequestContextPtr;
using finalmq::fmqreg::RegisterService;
using finalmq::fmqreg::GetService;
using finalmq::fmqreg::GetServiceReply;
using finalmq::fmqreg::Service;



Registry::Registry()
{
    // register peer events to see when a remote entity connects or disconnects.
    registerPeerEvent([] (PeerId peerId, const IProtocolSessionPtr& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
        streamInfo << "peer event " << peerEvent.toString();
    });

    registerCommand<RegisterService>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<RegisterService>& request) {
        assert(request);
        m_services[request->service.name] = request->service;
    });

    registerCommand<GetService>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<GetService>& request) {
        assert(request);
        auto it = m_services.find(request->name);
        if (it != m_services.end())
        {
            requestContext->reply(GetServiceReply(true, it->second));
        }
        else
        {
            requestContext->reply(GetServiceReply(false, Service()));
        }
    });
}



