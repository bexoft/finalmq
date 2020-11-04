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

#include "remoteobject/RemoteObjectContainer.h"


namespace finalmq {


RemoteObjectContainer::RemoteObjectContainer()
    : m_streamConnectionContainer(std::make_unique<ProtocolSessionContainer>())
{

}

RemoteObjectContainer::~RemoteObjectContainer()
{

}


// IRemoteObjectContainer
void RemoteObjectContainer::init(int cycleTime, int checkReconnectInterval)
{
    m_streamConnectionContainer->init(cycleTime, checkReconnectInterval);
}

int RemoteObjectContainer::bind(const std::string& endpoint, IProtocolFactoryPtr protocolFactory)
{
    return m_streamConnectionContainer->bind(endpoint, this, protocolFactory);
}

void RemoteObjectContainer::unbind(const std::string& endpoint)
{
    m_streamConnectionContainer->unbind(endpoint);
}

void RemoteObjectContainer::connect(const std::string& endpoint, const IProtocolPtr& protocol, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connect(endpoint, this, protocol, reconnectInterval, totalReconnectDuration);
}

void RemoteObjectContainer::threadEntry()
{
    m_streamConnectionContainer->threadEntry();
}

bool RemoteObjectContainer::terminatePollerLoop(int timeout)
{
    return m_streamConnectionContainer->terminatePollerLoop(timeout);
}


#ifdef USE_OPENSSL
int RemoteObjectContainer::bindSsl(const std::string& endpoint, IProtocolFactoryPtr protocolFactory, const CertificateData& certificateData)
{
    return m_streamConnectionContainer->bindSsl(endpoint, this, protocolFactory, certificateData);
}

void RemoteObjectContainer::connectSsl(const std::string& endpoint, const IProtocolPtr& protocol, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connectSsl(endpoint, this, protocol, certificateData, reconnectInterval, totalReconnectDuration);
}

#endif

// IProtocolSessionCallback
void RemoteObjectContainer::connected(const IProtocolSessionPtr& /*session*/)
{

}

void RemoteObjectContainer::disconnected(const IProtocolSessionPtr& /*session*/)
{

}

void RemoteObjectContainer::received(const IProtocolSessionPtr& /*session*/, const IMessagePtr& /*message*/)
{

}

void RemoteObjectContainer::socketConnected(const IProtocolSessionPtr& /*session*/)
{

}

void RemoteObjectContainer::socketDisconnected(const IProtocolSessionPtr& /*session*/)
{

}




}   // namespace finalmq
