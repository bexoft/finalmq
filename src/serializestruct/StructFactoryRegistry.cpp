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

#include "finalmq/serializestruct/StructFactoryRegistry.h"

#include <assert.h>


namespace finalmq {


// IStructFactoryRegistry
void StructFactoryRegistryImpl::registerFactory(const std::string& typeName, FuncStructBaseFactory factory)
{
    m_factories[typeName] = factory;
}

std::shared_ptr<StructBase> StructFactoryRegistryImpl::createStruct(const std::string& typeName)
{
    auto it = m_factories.find(typeName);
    if (it != m_factories.end())
    {
        if (it->second)
        {
            return it->second();
        }
    }
    return {};
}





////////////////////////////////

std::atomic<IStructFactoryRegistry*> StructFactoryRegistry::m_instance{};
std::unique_ptr<IStructFactoryRegistry> StructFactoryRegistry::m_instanceUniquePtr;
std::mutex StructFactoryRegistry::m_mutex;

void StructFactoryRegistry::setInstance(std::unique_ptr<IStructFactoryRegistry>&& instance)
{
    m_instanceUniquePtr = std::move(instance);
    IStructFactoryRegistry* inst = m_instanceUniquePtr.get();
    m_instance.store(inst, std::memory_order_release);
}

IStructFactoryRegistry* StructFactoryRegistry::createInstance()
{
    std::unique_lock<std::mutex>(m_mutex);
    IStructFactoryRegistry* inst = m_instance.load(std::memory_order_relaxed);
    if (!inst)
    {
        m_instanceUniquePtr = std::make_unique<StructFactoryRegistryImpl>();
        inst = m_instanceUniquePtr.get();
        m_instance.store(inst, std::memory_order_relaxed);
    }
    return inst;
}


}   // namespace finalmq
