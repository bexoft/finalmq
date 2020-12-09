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

#include "helpers/IZeroCopyBuffer.h"

#include <memory>
#include <string>
#include <memory.h>
#include <assert.h>
#include <list>


namespace finalmq {


struct IProtocol;

typedef std::pair<char*, ssize_t> BufferRef;

struct IMessage : public IZeroCopyBuffer
{
    virtual ~IMessage() {}

    // for send
    virtual void addSendPayload(const std::string& payload) = 0;
    virtual void addSendPayload(const char* payload, ssize_t size) = 0;
    virtual char* addSendPayload(ssize_t size) = 0;
    virtual void downsizeLastSendPayload(ssize_t newSize) = 0;

    // for receive
    virtual BufferRef getReceivePayload() const = 0;
    virtual char* resizeReceivePayload(ssize_t size) = 0;

    // for the framework
    virtual const std::list<BufferRef>& getAllSendBuffers() const = 0;
    virtual ssize_t getTotalSendBufferSize() const = 0;
    virtual const std::list<BufferRef>& getAllSendPayloads() const = 0;
    virtual ssize_t getTotalSendPayloadSize() const = 0;

    // for the protocol to add a header
    virtual void addSendHeader(const std::string& header) = 0;
    virtual void addSendHeader(const char* header, ssize_t size) = 0;
    virtual char* addSendHeader(ssize_t size) = 0;
    virtual void downsizeLastSendHeader(ssize_t newSize) = 0;

    // for the protocol to prepare the message for send
    virtual void prepareMessageToSend() = 0;

    // for the protocol to check which protocol created the message
    virtual std::uint32_t getProtocolId() const = 0;
    virtual bool wasSent() const = 0;

    virtual void addMessage(const std::shared_ptr<IMessage>& msg) = 0;
    virtual std::shared_ptr<IMessage> getMessage(int protocolId) const = 0;
};


typedef std::shared_ptr<IMessage>    IMessagePtr;

}   // namespace finalmq
