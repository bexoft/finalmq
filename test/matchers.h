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

#include "gtest/gtest.h"
#include "finalmq/streamconnection/IMessage.h"


MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.typeId == metaField.typeId &&
        arg.typeName == metaField.typeName &&
        arg.name == metaField.name &&
        arg.index == metaField.index);
}

MATCHER_P(MatcherFloat, value, "")
{
    return (arg == value || (std::isnan(arg) && std::isnan(value)));
}

MATCHER_P(MatcherDouble, value, "")
{
    return (arg == value || (std::isnan(arg) && std::isnan(value)));
}

MATCHER_P(MatcherArrayFloat, value, "")
{
    if (arg.size() != value.size())
    {
        return false;
    }
    for (size_t i = 0; i < arg.size(); ++i)
    {
        if (!(arg[i] == value[i] || (std::isnan(arg[i]) && std::isnan(value[i]))))
        {
            return false;
        }
    }
    return true;
}

MATCHER_P(MatcherArrayDouble, value, "")
{
    if (arg.size() != value.size())
    {
        return false;
    }
    for (size_t i = 0; i < arg.size(); ++i)
    {
        if (!(arg[i] == value[i] || (std::isnan(arg[i]) && std::isnan(value[i]))))
        {
            return false;
        }
    }
    return true;
}


MATCHER_P2(ArrayEq, compareArray, n, "")
{
    return (memcmp(arg, compareArray, n * sizeof(*arg)) == 0);
}

MATCHER_P(Event, event, "")
{
    return (arg->events == event->events &&
        arg->data.fd == event->data.fd);
}

MATCHER_P(ReceivedMessage, msg, "")
{
    finalmq::BufferRef buffer = arg->getReceivePayload();
    std::string str(buffer.first, buffer.second);
    return str == msg;
}

MATCHER_P(MatcherReceiveMessage, message, "")
{
    return (arg->getAllMetainfo() == message->getAllMetainfo() &&
        arg->getReceivePayload().second == message->getReceivePayload().second &&
        memcmp(arg->getReceivePayload().first, message->getReceivePayload().first, arg->getReceivePayload().second) == 0);
}

MATCHER_P(FdSet, fds, "")
{
    return (memcmp(arg, fds, sizeof(*fds)) == 0);
}

MATCHER_P(Time, tim, "")
{
    return (memcmp(arg, tim, sizeof(*tim)) == 0);
}

