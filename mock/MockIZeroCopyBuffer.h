#pragma once


#include "helpers/IZeroCopyBuffer.h"


#include "gmock/gmock.h"

class MockIZeroCopyBuffer : public IZeroCopyBuffer
{
public:
    MOCK_METHOD(char*, addBuffer, (int size), (override));
    MOCK_METHOD(void, downsizeLastBuffer, (int newSize), (override));
};
