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

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "StructBase.h"

namespace finalmq
{
struct IStructFactoryRegistry
{
    virtual ~IStructFactoryRegistry()
    {}
    virtual void registerFactory(const std::string& typeName, FuncStructBaseFactory factory) = 0;
    virtual std::shared_ptr<StructBase> createStruct(const std::string& typeName) = 0;
};

class SYMBOLEXP StructFactoryRegistryImpl : public IStructFactoryRegistry
{
public:
private:
    // IStructFactoryRegistry
    virtual void registerFactory(const std::string& typeName, FuncStructBaseFactory factory) override;
    virtual std::shared_ptr<StructBase> createStruct(const std::string& typeName) override;

    std::unordered_map<std::string, FuncStructBaseFactory> m_factories{};
};

class SYMBOLEXP StructFactoryRegistry
{
public:
    inline static IStructFactoryRegistry& instance()
    {
        static auto& instanceRef = getStaticInstanceRef();
        auto* inst = instanceRef.load(std::memory_order_acquire);
        if (!inst)
        {
            inst = createInstance();
        }
        return *inst;
    }

    /**
    * Overwrite the default implementation, e.g. with a mock for testing purposes.
    * This method is not thread-safe. Make sure that no one uses the current instance before
    * calling this method.
    */
    static void setInstance(std::unique_ptr<IStructFactoryRegistry>&& instance);

private:
    StructFactoryRegistry() = delete;
    ~StructFactoryRegistry() = delete;
    static IStructFactoryRegistry* createInstance();

    static std::atomic<IStructFactoryRegistry*>& getStaticInstanceRef();
    static std::unique_ptr<IStructFactoryRegistry>& getStaticUniquePtrRef();
};

} // namespace finalmq
