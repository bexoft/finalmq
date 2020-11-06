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
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocols/ProtocolDelimiter.h"



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

int RemoteObjectContainer::bind(const std::string& endpoint, RemoteObjectProtocol protocol)
{
    return m_streamConnectionContainer->bind(endpoint, this, createProtocolFactory(protocol));
}

void RemoteObjectContainer::unbind(const std::string& endpoint)
{
    m_streamConnectionContainer->unbind(endpoint);
}

void RemoteObjectContainer::connect(const std::string& endpoint, RemoteObjectProtocol protocol, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connect(endpoint, this, createProtocol(protocol), reconnectInterval, totalReconnectDuration);
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
int RemoteObjectContainer::bindSsl(const std::string& endpoint, RemoteObjectProtocol protocol, const CertificateData& certificateData)
{
    return m_streamConnectionContainer->bindSsl(endpoint, this, createProtocolFactory(protocol), certificateData);
}

void RemoteObjectContainer::connectSsl(const std::string& endpoint, RemoteObjectProtocol protocol, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration)
{
    m_streamConnectionContainer->connectSsl(endpoint, this, createProtocol(protocol), certificateData, reconnectInterval, totalReconnectDuration);
}

#endif

int RemoteObjectContainer::registerObject(hybrid_ptr<IRemoteObject> remoteObject, const std::string& name)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!name.empty())
    {
        auto it = m_name2objectId.find(name);
        if (it != m_name2objectId.end())
        {
            return INVALID_OBJECTID;
        }
    }

    ObjectId objectId = m_nextObjectId;
    ++m_nextObjectId;

    if (!name.empty())
    {
        m_name2objectId[name] = objectId;
    }

    m_objectId2object[objectId] = remoteObject;

    return objectId;
}

void RemoteObjectContainer::unregisterObject(int objectId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_name2objectId.begin(); it != m_name2objectId.end(); ++it)
    {
        if (it->second == objectId)
        {
            m_name2objectId.erase(it);
            break;
        }
    }
    m_objectId2object.erase(objectId);
}

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


IProtocolFactoryPtr RemoteObjectContainer::createProtocolFactory(RemoteObjectProtocol protocol)
{
    switch (protocol)
    {
    case RemoteObjectProtocol::PROT_PROTO:
        return std::make_shared<ProtocolHeaderBinarySizeFactory>();
    case RemoteObjectProtocol::PROT_JSON:
        return std::make_shared<ProtocolDelimiterFactory>("\n");
    }
    assert(false);
    return nullptr;
}


IProtocolPtr RemoteObjectContainer::createProtocol(RemoteObjectProtocol protocol)
{
    switch (protocol)
    {
    case RemoteObjectProtocol::PROT_PROTO:
        return std::make_shared<ProtocolHeaderBinarySize>();
    case RemoteObjectProtocol::PROT_JSON:
        return std::make_shared<ProtocolDelimiter>("\n");
    }
    assert(false);
    return nullptr;
}



}   // namespace finalmq
