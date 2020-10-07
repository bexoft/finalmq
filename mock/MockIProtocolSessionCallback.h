#pragma once


#include "protocolconnection/IProtocolSession.h"


#include "gmock/gmock.h"

class MockIProtocolSessionCallback : public IProtocolSessionCallback
{
public:
    MOCK_METHOD(void, connected, (const IProtocolSessionPtr& connection), (override));
    MOCK_METHOD(void, disconnected, (const IProtocolSessionPtr& connection), (override));
    MOCK_METHOD(void, received, (const IProtocolSessionPtr& connection, const IMessagePtr& message), (override));
    MOCK_METHOD(void, socketConnected, (const IProtocolSessionPtr& connection), (override));
    MOCK_METHOD(void, socketDisconnected, (const IProtocolSessionPtr& connection), (override));
};
