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


#include "finalmq/protocolsession/IProtocol.h"

#include <unordered_map>

#include <mutex>
#include <atomic>

namespace finalmq {


enum
{
    REMOTEENTITYPROTOCOL_DELIMITERLINEFEED = 1,
    REMOTEENTITYPROTOCOL_HEADERBINARYSIZE = 2,
};


struct IProtocolRegistry
{
    virtual ~IProtocolRegistry() {}

    virtual void registerProtocolFactory(const std::string& protocolName, int protocolId, const IProtocolFactoryPtr& protocolFactory) = 0;
    virtual IProtocolFactoryPtr getProtocolFactory(const std::string& protocolName) const = 0;
    virtual IProtocolFactoryPtr getProtocolFactory(int protocolId) const = 0;
};




class ProtocolRegistryImpl : public IProtocolRegistry
{
public:
    virtual void registerProtocolFactory(const std::string& protocolName, int remoteEntityProtocolId, const IProtocolFactoryPtr& protocolFactory) override;
    virtual IProtocolFactoryPtr getProtocolFactory(const std::string& protocolName) const override;
    virtual IProtocolFactoryPtr getProtocolFactory(int protocolId) const override;

private:
    std::unordered_map<std::string, IProtocolFactoryPtr>    m_protocolNameToFactory;
    std::unordered_map<int, IProtocolFactoryPtr>            m_protocolIdToFactory;
};


class SYMBOLEXP ProtocolRegistry
{
public:
    inline static IProtocolRegistry& instance()
    {
        IProtocolRegistry* instance = getStaticInstanceRef().load(std::memory_order_acquire);
        if (!instance)
        {
            instance = createInstance();
        }
        return *instance;
    }

    /**
    * Overwrite the default implementation, e.g. with a mock for testing purposes.
    * This method is not thread-safe. Make sure that no one uses the current instance before
    * calling this method.
    */
    static void setInstance(std::unique_ptr<IProtocolRegistry>&& instance);

private:
    ProtocolRegistry() = delete;
    ~ProtocolRegistry() = delete;
    static IProtocolRegistry* createInstance();

    static std::atomic<IProtocolRegistry*>& getStaticInstanceRef();
    static std::unique_ptr<IProtocolRegistry>& getStaticUniquePtrRef();
};


}   // namespace finalmq
