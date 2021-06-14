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

#include "finalmq/protocolconnection/ProtocolRegistry.h"



namespace finalmq {



void ProtocolRegistryImpl::registerProtocolFactory(const std::string& protocolName, int protocolId, const IProtocolFactoryPtr& protocolFactory)
{
    m_protocolNameToFactory[protocolName] = protocolFactory;
    m_protocolIdToFactory[protocolId] = protocolFactory;
}


IProtocolFactoryPtr ProtocolRegistryImpl::getProtocolFactory(const std::string& protocolName) const
{
    auto it = m_protocolNameToFactory.find(protocolName);
    if (it != m_protocolNameToFactory.end())
    {
        return it->second;
    }
    return nullptr;
}


IProtocolFactoryPtr ProtocolRegistryImpl::getProtocolFactory(int protocolId) const
{
    auto it = m_protocolIdToFactory.find(protocolId);
    if (it != m_protocolIdToFactory.end())
    {
        return it->second;
    }
    return nullptr;
}





//////////////////////////////////////
/// ProtocolRegistry

std::unique_ptr<IProtocolRegistry> ProtocolRegistry::m_instance;

void ProtocolRegistry::setInstance(std::unique_ptr<IProtocolRegistry>& instance)
{
    m_instance = std::move(instance);
}




}   // namespace finalmq
