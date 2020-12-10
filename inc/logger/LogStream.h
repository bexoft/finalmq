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
#include "helpers/BexDefines.h"
#include <string>
#include <sstream>



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
        return LogStream(context);
    }

    explicit LogStream(const LogContext& context);
    LogStream(const LogStream& rhs);
    LogStream(LogStream&& rhs);

    ~LogStream();

    /**
     * Flush the contents of the stream buffer.
     **/
    void flush();

    /**
     * Stream in arbitrary types and objects.
     * @param The value or object to stream in.
     * @return A reference to itself.
     **/
    template<typename T>
    LogStream& operator <<(const T& t)
    {
        (*m_buffer) << t;
        return *this;
    }

    LogStream& operator <<(const char* t);
    LogStream& operator <<(const std::string& t);

    /**
    * Set the width output on LogStream
    **/
    std::streamsize width(std::streamsize wide );

private:
    const LogContext&   m_context;
    std::ostringstream* m_buffer = nullptr;
};


} // namespace finalmq

