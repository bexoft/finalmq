#pragma once

#include "helpers/IZeroCopyBuffer.h"

#include <memory>
#include <string>
#include <memory.h>
#include <assert.h>
#include <list>


struct IProtocol;

typedef std::pair<char*, int> BufferRef;

struct IMessage : public IZeroCopyBuffer
{
    virtual ~IMessage() {}

    // for send
    virtual void addSendPayload(const std::string& payload) = 0;
    virtual void addSendPayload(const char* payload, int size) = 0;
    virtual char* addSendPayload(int size) = 0;
    virtual void downsizeLastSendPayload(int newSize) = 0;

    // for receive
    virtual BufferRef getReceivePayload() = 0;
    virtual char* resizeReceivePayload(int size) = 0;

    // for the framework
    virtual const std::list<BufferRef>& getAllSendBuffers() const = 0;
    virtual int getTotalSendBufferSize() const = 0;
    virtual const std::list<BufferRef>& getAllSendPayloads() const = 0;
    virtual int getTotalSendPayloadSize() const = 0;

    // for the protocol to add a header
    virtual void addSendHeader(const std::string& header) = 0;
    virtual void addSendHeader(const char* header, int size) = 0;
    virtual char* addSendHeader(int size) = 0;
    virtual void downsizeLastSendHeader(int newSize) = 0;

    // for the protocol to prepare the message for send
    virtual void prepareMessageToSend() = 0;

    // for the protocol to check if which protocol created the message
    virtual int getProtocolId() const = 0;
    virtual bool wasSent() const = 0;

    virtual void addMessage(const std::shared_ptr<IMessage>& msg) = 0;
    virtual std::shared_ptr<IMessage> getMessage(int protocolId) const = 0;
};


typedef std::shared_ptr<IMessage>    IMessagePtr;


