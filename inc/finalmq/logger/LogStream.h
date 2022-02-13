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

#include "Logger.h"
#include <string>
#include <sstream>

#if defined(WIN32)
#define METHODNAME  __FUNCTION__
#else
#define METHODNAME  __FUNCTION__
#endif



#define STREAM_BASIC(level)     static const finalmq::LogContext TOKENPASTE2(context, __LINE__){level, MODULENAME, METHODNAME, __FILE__, __LINE__}; finalmq::LogStream::getStream(TOKENPASTE2(context, __LINE__))

#define streamDebug                 STREAM_BASIC(finalmq::LogLevel::LOG_DEBUG)
#define streamInfo                  STREAM_BASIC(finalmq::LogLevel::LOG_INFO)
#define streamWarning               STREAM_BASIC(finalmq::LogLevel::LOG_WARNING)
#define streamError                 STREAM_BASIC(finalmq::LogLevel::LOG_ERROR)
#define streamCritical              STREAM_BASIC(finalmq::LogLevel::LOG_CRITICAL)
#define streamFatal                 STREAM_BASIC(finalmq::LogLevel::LOG_FATAL)


namespace finalmq {


class SYMBOLEXP LogStream
{
public:

    inline static LogStream getStream(const LogContext& context)
    {
        return { context };
    }

    inline ~LogStream()
    {
        Logger::instance().triggerLog(m_context, m_buffer.str().c_str());
    }

    /**
     * Stream in arbitrary types and objects.
     * @param The value or object to stream in.
     * @return A reference to itself.
     **/
    template<typename T>
    LogStream& operator <<(const T& t)
    {
        m_buffer << t;
        return *this;
    }

    inline LogStream& operator <<(const char* t)
    {
        m_buffer << t;
        return *this;
    }

    inline LogStream& operator <<(const std::string& t)
    {
        m_buffer << t;
        return *this;
    }

    /**
    * Set the width output on LogStream
    **/
    inline std::streamsize width(std::streamsize wide)
    {
        return m_buffer.width(wide);
    }

private:
    inline LogStream(const LogContext& context)
        : m_context(context)
        , m_buffer()
    {
    }

    LogStream(const LogStream& rhs) = delete;
    const LogStream& operator =(const LogStream& rhs) = delete;
    LogStream(LogStream&& rhs) = delete;
    const LogStream& operator =(const LogStream&& rhs) = delete;

    const LogContext&   m_context;
    std::ostringstream  m_buffer;
};


} // namespace finalmq

