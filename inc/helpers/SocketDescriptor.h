#pragma once


#include "helpers/BexDefines.h"
#include <memory>


class SocketDescriptor
{
public:
    SocketDescriptor(int sd);
    ~SocketDescriptor();

	inline int getDescriptor() const
    {
        return m_sd;
    }

private:
    int		m_sd = INVALID_FD;
};

typedef std::shared_ptr<SocketDescriptor> SocketDescriptorPtr;

