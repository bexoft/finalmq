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

#include "finalmq/logger/Logger.h"


namespace finalmq {


LoggerImpl::LoggerImpl()
{
}



void LoggerImpl::registerConsumer(FuncLogEvent consumer)
{
    // lock, so that registerConsumer can be called from multiple threads
    std::unique_lock<std::mutex> lock(m_mutex);
    m_consumers.push_back(consumer);
    m_sizeConsumers.store(static_cast<int>(m_consumers.size()), std::memory_order_release);
    lock.unlock();
}

void LoggerImpl::triggerLog(const LogContext& context, const char* text)
{
    size_t size = m_sizeConsumers.load(std::memory_order_acquire);
    for (size_t i = 0; i < size; ++i)
    {
        FuncLogEvent& func = m_consumers[i];
        if (func)
        {
            func(context, text);
        }
    }
}




/////////////////////////////////////////////////////

void Logger::setInstance(std::unique_ptr<ILogger>&& instanceUniquePtr)
{
    getStaticUniquePtrRef() = std::move(instanceUniquePtr);
    getStaticInstanceRef().store(getStaticUniquePtrRef().get(), std::memory_order_release);
}

ILogger* Logger::createInstance()
{
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    ILogger* inst = getStaticInstanceRef().load(std::memory_order_relaxed);
    if (!inst)
    {
        setInstance(std::make_unique<LoggerImpl>());
        inst = getStaticInstanceRef().load(std::memory_order_relaxed);
    }
    return inst;
}

std::atomic<ILogger*>& Logger::getStaticInstanceRef()
{
    static std::atomic<ILogger*> instance;
    return instance;
}

std::unique_ptr<ILogger>& Logger::getStaticUniquePtrRef()
{
    static std::unique_ptr<ILogger> instanceUniquePtr;
    return instanceUniquePtr;
}


} // namespace finalmq

