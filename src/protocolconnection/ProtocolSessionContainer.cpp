
#include "protocolconnection/ProtocolSessionContainer.h"
#include "streamconnection/StreamConnectionContainer.h"




ProtocolBind::ProtocolBind(bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const std::weak_ptr<IProtocolSessionList>& protocolSessionList)
    : m_callback(callback)
    , m_protocolFactory(protocolFactory)
    , m_protocolSessionList(protocolSessionList)
{

}

// IStreamConnectionCallback
bex::hybrid_ptr<IStreamConnectionCallback> ProtocolBind::connected(const IStreamConnectionPtr& connection)
{
    IProtocolPtr protocol = m_protocolFactory->createProtocol();
    assert(protocol);
    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(m_callback, protocol, m_protocolSessionList);
    protocolSession->setConnection(connection);
    return std::weak_ptr<IStreamConnectionCallback>(protocolSession);
}

void ProtocolBind::disconnected(const IStreamConnectionPtr& connection)
{
    // should never be called, because the callback will be overriden by connected
    assert(false);
}

void ProtocolBind::received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead)
{
    // should never be called, because the callback will be overriden by connected
    assert(false);
}



//////////////////////////////

ProtocolSessionContainer::ProtocolSessionContainer()
    : m_protocolSessionList(std::make_shared<ProtocolSessionList>())
    , m_streamConnectionContainer(std::make_shared<StreamConnectionContainer>())
{

}

ProtocolSessionContainer::~ProtocolSessionContainer()
{

}

// IProtocolSessionContainer
void ProtocolSessionContainer::init(int cycleTime, int checkReconnectInterval)
{
    m_streamConnectionContainer->init(cycleTime, checkReconnectInterval);
}

int ProtocolSessionContainer::bind(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory)
{
    int err = 0;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_endpoint2Bind.find(endpoint);
    if (it == m_endpoint2Bind.end())
    {
        ProtocolBindPtr bind = std::make_shared<ProtocolBind>(callback, protocolFactory, m_protocolSessionList);
        m_endpoint2Bind[endpoint] = bind;
        lock.unlock();

        err = m_streamConnectionContainer->bind(endpoint, bind);
    }
    return err;
}

void ProtocolSessionContainer::unbind(const std::string& endpoint)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_endpoint2Bind.find(endpoint);
    if (it != m_endpoint2Bind.end())
    {
        m_endpoint2Bind.erase(it);
        m_streamConnectionContainer->unbind(endpoint);
    }
}

IProtocolSessionPtr ProtocolSessionContainer::connect(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, int reconnectInterval, int totalReconnectDuration)
{
    assert(protocol);
    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(callback, protocol, m_protocolSessionList, m_streamConnectionContainer, endpoint, reconnectInterval, totalReconnectDuration);
    protocolSession->connect();
    return protocolSession;
}

std::vector< IProtocolSessionPtr > ProtocolSessionContainer::getAllSessions() const
{
    std::vector< IProtocolSessionPtr > protocolSessions = m_protocolSessionList->getAllSessions();
    return protocolSessions;
}

IProtocolSessionPtr ProtocolSessionContainer::getSession(std::int64_t sessionId) const
{
    IProtocolSessionPtr protocolSession = m_protocolSessionList->getSession(sessionId);
    return protocolSession;
}


void ProtocolSessionContainer::threadEntry()
{
    m_streamConnectionContainer->threadEntry();
}


bool ProtocolSessionContainer::terminatePollerLoop(int timeout)
{
    return m_streamConnectionContainer->terminatePollerLoop(timeout);
}


#ifdef USE_OPENSSL
int ProtocolSessionContainer::bindSsl(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, IProtocolFactoryPtr protocolFactory, const CertificateData& certificateData)
{
    int err = 0;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_endpoint2Bind.find(endpoint);
    if (it == m_endpoint2Bind.end())
    {
        ProtocolBindPtr bind = std::make_shared<ProtocolBind>(callback, protocolFactory, m_protocolSessionList);
        m_endpoint2Bind[endpoint] = bind;
        lock.unlock();

        err = m_streamConnectionContainer->bindSsl(endpoint, bind, certificateData);
    }
    return err;
}

IProtocolSessionPtr ProtocolSessionContainer::connectSsl(const std::string& endpoint, bex::hybrid_ptr<IProtocolSessionCallback> callback, const IProtocolPtr& protocol, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration)
{
    assert(protocol);
    IProtocolSessionPrivatePtr protocolSession = std::make_shared<ProtocolSession>(callback, protocol, m_protocolSessionList, m_streamConnectionContainer, endpoint, certificateData, reconnectInterval, totalReconnectDuration);
    protocolSession->connect();
    return protocolSession;
}
#endif

