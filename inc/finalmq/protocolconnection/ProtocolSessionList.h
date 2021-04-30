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

#include "IProtocolSession.h"

#include <unordered_map>

namespace finalmq {

struct IProtocolSessionPrivate;
typedef std::shared_ptr<IProtocolSessionPrivate> IProtocolSessionPrivatePtr;


struct IProtocolSessionList
{
    virtual ~IProtocolSessionList() {}
    virtual std::int64_t addProtocolSession(IProtocolSessionPrivatePtr ProtocolSession, bool verified) = 0;
    virtual void removeProtocolSession(std::int64_t sessionId) = 0;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const = 0;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const = 0;
    virtual IProtocolSessionPrivatePtr findSessionByName(const std::string& sessionName) const = 0;
    virtual void setSessionName(std::int64_t sessionId, const std::string& sessionName) = 0;
};

typedef std::shared_ptr<IProtocolSessionList> IProtocolSessionListPtr;



class ProtocolSessionList : public IProtocolSessionList
{
public:
    ProtocolSessionList();
    virtual ~ProtocolSessionList();
private:
    virtual std::int64_t addProtocolSession(IProtocolSessionPrivatePtr ProtocolSession, bool verified) override;
    virtual void removeProtocolSession(std::int64_t sessionId) override;
    virtual std::vector< IProtocolSessionPtr > getAllSessions() const override;
    virtual IProtocolSessionPtr getSession(std::int64_t sessionId) const override;
    virtual IProtocolSessionPrivatePtr findSessionByName(const std::string& sessionName) const override;
    virtual void setSessionName(std::int64_t sessionId, const std::string& sessionName) override;

    struct SessionData
    {
        IProtocolSessionPrivatePtr  session;
        bool                        verified = false;
        std::string                 name;
    };

    std::unordered_map<std::int64_t, SessionData>       m_connectionId2ProtocolSession;
    std::int64_t                                        m_nextSessionId = 1;
    mutable std::mutex                                  m_mutex;
};

}   // namespace finalmq
