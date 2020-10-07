

#include "helpers/SocketDescriptor.h"
#include "helpers/OperatingSystem.h"





SocketDescriptor::SocketDescriptor(int sd)
    : m_sd(sd)
{

}

SocketDescriptor::~SocketDescriptor()
{
    if (m_sd != INVALID_FD)
    {
        OperatingSystem::instance().closeSocket(m_sd);
    }
}

