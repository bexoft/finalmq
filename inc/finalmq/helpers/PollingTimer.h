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

#include <chrono>

namespace finalmq {


class PollingTimer
{
public:
    void setTimeout(int timeout)
    {
        m_timeoutMs = timeout;
        m_timer = std::chrono::system_clock::now();
    }

    void stop()
    {
        m_timeoutMs = -1;
    }

    bool isExpired(bool stopIfExpired = true)
    {
        bool expired = false;
        if (m_timeoutMs != -1)
        {
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            std::chrono::duration<double> dur = now - m_timer;
            long long delta = static_cast<long long>(dur.count() * 1000);
            if (delta > m_timeoutMs)
            {
                expired = true;
                if (stopIfExpired)
                {
                    stop();
                }
            }
            else if (delta < 0)
            {
                m_timer = now;
            }
        }
        return expired;
    }

private:
    int m_timeoutMs = -1;
    std::chrono::time_point<std::chrono::system_clock> m_timer = std::chrono::system_clock::now();
};


} // namespace finalmq
