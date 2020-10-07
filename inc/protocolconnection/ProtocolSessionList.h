#pragma once

#include "IProtocolSession.h"

#include <unordered_map>



struct IProtocolSessionList
{
    virtual ~IProtocolSessionList() {}
    virtual std::int64_t addProtocolSession(IProtocolSessionPtr ProtocolSession) = 0;
    virtual void removeProtocolSession(std::int64_t sessionId) = 0;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const = 0;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const = 0;
};

typedef std::shared_ptr<IProtocolSessionList> IProtocolSessionListPtr;



class ProtocolSessionList : public IProtocolSessionList
{
public:
    ProtocolSessionList();
    virtual ~ProtocolSessionList();
private:
    virtual std::int64_t addProtocolSession(IProtocolSessionPtr ProtocolSession) override;
    virtual void removeProtocolSession(std::int64_t sessionId) override;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const override;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const override;

    std::unordered_map<std::int64_t, IProtocolSessionPtr>    m_connectionId2ProtocolSession;
    std::int64_t                                                m_nextSessionId = 1;
    mutable std::mutex                                          m_mutex;
};

