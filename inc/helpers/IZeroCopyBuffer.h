#pragma once

#include <memory>

struct IZeroCopyBuffer
{
    virtual ~IZeroCopyBuffer() {}

    virtual char* addBuffer(int size) = 0;
    virtual void downsizeLastBuffer(int newSize) = 0;
};


typedef std::shared_ptr<IZeroCopyBuffer>    IZeroCopyBufferPtr;


