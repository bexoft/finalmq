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

#pragma once

#include <sys/types.h>

#include "finalmq/interfaces/fmqreg.fmq.h"
#include "finalmq/remoteentity/RemoteEntityContainer.h"

namespace finalmq
{
class SYMBOLEXP FmqRegistryClient
{
public:
    typedef std::function<void(Status status, const std::shared_ptr<fmqreg::GetServiceReply>& reply)> FuncGetServiceReply;

    FmqRegistryClient(const hybrid_ptr<IRemoteEntityContainer>& remoteEntityContainer);

    void registerService(const finalmq::fmqreg::Service& service, int retryDurationMs = -1);
    PeerId connectService(const std::string& serviceName, EntityId entityId, const ConnectProperties& connectProperties, FuncReplyConnect funcReplyConnect);
    void getService(const std::string& serviceName, FuncGetServiceReply funcGetServiceReply);

private:
    void init();
    SessionInfo createRegistrySession(const std::string& hostname, const ConnectProperties& connectProperties = {});

    bool m_init = false;
    hybrid_ptr<IRemoteEntityContainer> m_remoteEntityContainer{};
    IRemoteEntityPtr m_entityRegistry{};
};

} // namespace finalmq
