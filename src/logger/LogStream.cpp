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

#include "finalmq/logger/LogStream.h"


namespace finalmq {


LogStream::LogStream(const LogContext& context)
    : m_context(context)
    , m_buffer(new std::ostringstream)
{
}

LogStream::LogStream(const LogStream& rhs)
    : m_context(rhs.m_context)
    , m_buffer(rhs.m_buffer)
{
}


LogStream::LogStream(LogStream&& rhs) noexcept
    : m_context(rhs.m_context)
    , m_buffer(rhs.m_buffer)
{
}



LogStream::~LogStream()
{
    flush();
}

void LogStream::flush()
{
    if (m_buffer)
    {
        Logger::instance().triggerLog(m_context, m_buffer->str().c_str());
        delete m_buffer;
        m_buffer = nullptr;
    }
}

LogStream& LogStream::operator <<(const char* t)
{
    (*m_buffer) << t;
    return *this;
}


LogStream& LogStream::operator <<(const std::string& t)
{
    (*m_buffer) << t;
    return *this;
}

std::streamsize LogStream::width(std::streamsize wide)
{
    return m_buffer->width(wide);
}


} // namespace finalmq

