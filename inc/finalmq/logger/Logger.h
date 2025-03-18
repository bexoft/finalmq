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
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/interfaces/fmqlog.fmq.h"

namespace finalmq
{

typedef std::function<void(const LogContext& context, const char* text)> FuncLogEvent;

struct ILogger
{
    virtual ~ILogger()
    {}
    virtual void registerConsumer(FuncLogEvent consumer) = 0;
    virtual void triggerLog(const LogContext& context, const char* text) = 0;
};

/**
 * @brief The LoggerImpl class implements a central point of collecting log data
 * of the whole application.
 * It is possible to register multiple log consumers which can forward the log data
 * to e.g. stdout, file, log4cpp, ...
 * Triggering log data to the consumers is implemented in a lock free way.
 */
class SYMBOLEXP LoggerImpl : public ILogger
{
public:
    LoggerImpl();

private:
    // ILogger
    virtual void registerConsumer(FuncLogEvent consumer) override;
    virtual void triggerLog(const LogContext& context, const char* text) override;

    std::deque<FuncLogEvent> m_consumers{};
    std::atomic_int m_sizeConsumers{};
    std::mutex m_mutex{};
};

class SYMBOLEXP Logger
{
public:
    static inline ILogger& instance()
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
    static void setInstance(std::unique_ptr<ILogger>&& instance);

private:
    Logger() = delete;
    ~Logger() = delete;
    static ILogger* createInstance();

    static std::atomic<ILogger*>& getStaticInstanceRef();
    static std::unique_ptr<ILogger>& getStaticUniquePtrRef();
};

} // namespace finalmq
