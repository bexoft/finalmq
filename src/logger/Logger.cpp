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

#include "logger/Logger.h"


namespace finalmq {


LoggerImpl::LoggerImpl()
{
    m_flagIdle.test_and_set(std::memory_order_acquire);
}



void LoggerImpl::registerConsumer(FuncLogEvent consumer)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_consumersAdd.push_back(consumer);
    lock.unlock();
    m_flagIdle.clear(std::memory_order_release);
}

void LoggerImpl::triggerLog(const LogContext& context, const char* text)
{
    if (!m_flagIdle.test_and_set(std::memory_order_acquire))
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_consumers.insert(m_consumers.end(), m_consumersAdd.begin(), m_consumersAdd.end());
        lock.unlock();
    }
    for (auto it = m_consumers.begin(); it != m_consumers.end(); ++it)
    {
        FuncLogEvent& func = *it;
        if (func)
        {
            func(context, text);
        }
    }
}




std::unique_ptr<ILogger> Logger::m_instance;

void Logger::setInstance(std::unique_ptr<ILogger>& instance)
{
    m_instance = std::move(instance);
}



} // namespace finalmq

