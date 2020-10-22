#pragma once

#include "IZeroCopyBuffer.h"

#include <list>

class ZeroCopyBuffer : public IZeroCopyBuffer
{
public:
    std::string getData() const;

private:
    virtual char* addBuffer(int size) override;
    virtual void downsizeLastBuffer(int newSize) override;

    std::list<std::string>    m_strings;
};




