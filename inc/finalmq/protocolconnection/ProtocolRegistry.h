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

#include <unordered_map>


namespace finalmq {


enum
{
    REMOTEENTITYPROTOCOL_DELIMITERLINEFEED = 1,
    REMOTEENTITYPROTOCOL_HEADERBINARYSIZE = 2,
};


struct IProtocolRegistry
{
    virtual ~IProtocolRegistry() {}

    virtual void registerProtocolFactory(int protocolId, const IProtocolFactoryPtr& protocolFactory) = 0;
    virtual IProtocolFactoryPtr getProtocolFactory(int protocolId) const = 0;
};




class ProtocolRegistryImpl : public IProtocolRegistry
{
public:
    virtual void registerProtocolFactory(int remoteEntityProtocolId, const IProtocolFactoryPtr& protocolFactory) override;
    virtual IProtocolFactoryPtr getProtocolFactory(int remoteEntityProtocolId) const override;

private:
    std::unordered_map<int, IProtocolFactoryPtr> m_protocolFactories;
};


class SYMBOLEXP ProtocolRegistry
{
public:
    inline static IProtocolRegistry& instance()
    {
        if (!m_instance)
        {
            m_instance = std::make_unique<ProtocolRegistryImpl>();
        }
        return *m_instance;
    }
    static void setInstance(std::unique_ptr<IProtocolRegistry>& instance);

private:
    ProtocolRegistry() = delete;

    static std::unique_ptr<IProtocolRegistry> m_instance;
};





}   // namespace finalmq
