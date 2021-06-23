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


#include "finalmq/protocolconnection/IProtocol.h"

#include "gmock/gmock.h"

namespace finalmq {

class MockIProtocolCallback : public IProtocolCallback
{
public:
    MOCK_METHOD(void, connected, (), (override));
    MOCK_METHOD(void, disconnected, (), (override));
    MOCK_METHOD(void, received, (const IMessagePtr& message, std::int64_t connectionId), (override));
    MOCK_METHOD(void, socketConnected, (), (override));
    MOCK_METHOD(void, socketDisconnected, (), (override));
    MOCK_METHOD(void, reconnect, (), (override));
    MOCK_METHOD(bool, findSessionByName, (const std::string& sessionName, const IProtocolPtr& protocol, const IStreamConnectionPtr& connection), (override));
    MOCK_METHOD(void, setSessionName, (const std::string& sessionName, const IProtocolPtr& protocol, const IStreamConnectionPtr& connection), (override));
    MOCK_METHOD(void, pollRequest, (std::int64_t connectionId, int timeout, int pollCountMax), (override));
    MOCK_METHOD(void, activity, (), (override));
    MOCK_METHOD(void, setActivityTimeout, (int timeout), (override));
    MOCK_METHOD(void, setPollMaxRequests, (int maxRequests), (override));
    MOCK_METHOD(void, disconnectedMultiConnection, (const IStreamConnectionPtr& connection), (override));
};

}   // namespace finalmq
