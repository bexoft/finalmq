#pragma once


#include "streamconnection/StreamConnectionContainer.h"


#include "gmock/gmock.h"

class MockIStreamConnectionCallback : public IStreamConnectionCallback
{
public:
    MOCK_METHOD(bex::hybrid_ptr<IStreamConnectionCallback>, connected, (const IStreamConnectionPtr& connection), (override));
    MOCK_METHOD(void, disconnected, (const IStreamConnectionPtr& connection), (override));
    MOCK_METHOD(void, received, (const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead), (override));
};

