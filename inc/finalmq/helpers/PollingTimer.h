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
#include <chrono>

namespace finalmq
{
class PollingTimer
{
public:
    void setTimeout(int timeout)
    {
        std::int64_t t = timeout;
        if (t != -1)
        {
            std::int64_t now = std::chrono::steady_clock::now().time_since_epoch().count() / (std::chrono::steady_clock::period::ratio::den / 1000);
            t += now;
        }
        m_timeExpired.store(t, std::memory_order_release);
    }

    void stop()
    {
        m_timeExpired.store(-1, std::memory_order_release);
    }

    bool isExpired(bool stopIfExpired = true)
    {
        bool expired = false;
        std::int64_t timeExpired = m_timeExpired.load(std::memory_order_acquire);
        if (timeExpired != -1)
        {
            std::int64_t now = std::chrono::steady_clock::now().time_since_epoch().count() / (std::chrono::steady_clock::period::ratio::den / 1000);
            if (now >= timeExpired)
            {
                expired = true;
                if (stopIfExpired)
                {
                    stop();
                }
            }
        }
        return expired;
    }

private:
    std::atomic<std::int64_t> m_timeExpired{-1};
};

} // namespace finalmq
