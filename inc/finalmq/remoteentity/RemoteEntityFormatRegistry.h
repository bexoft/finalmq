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

#include "finalmq/protocolsession/ProtocolSessionContainer.h"



namespace finalmq {

class StructBase;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


namespace remoteentity {
    class Header;
}



using EntityId = std::uint64_t;
static constexpr EntityId ENTITYID_DEFAULT = 0;
static constexpr EntityId ENTITYID_INVALID = 0x7fffffffffffffffull;

using CorrelationId = std::uint64_t;
static constexpr CorrelationId CORRELATIONID_NONE = 0;


struct IRemoteEntityFormat
{
    virtual ~IRemoteEntityFormat() {}
    virtual std::shared_ptr<StructBase> parse(const BufferRef& bufferRef, bool storeRawData, remoteentity::Header& header, bool& syntaxError) = 0;
    virtual std::shared_ptr<StructBase> parseData(const BufferRef& bufferRef, bool storeRawData, const std::string& type, bool& syntaxError) = 0;
    virtual void serialize(IMessage& message, const remoteentity::Header& header, const StructBase* structBase = nullptr) = 0;
    virtual void serializeData(IMessage& message, const StructBase* structBase = nullptr) = 0;
};


struct IRemoteEntityFormatRegistry
{
    virtual ~IRemoteEntityFormatRegistry() {}
    virtual std::shared_ptr<StructBase> parse(IMessage& message, int contentType, bool storeRawData, remoteentity::Header& header, bool& syntaxError) = 0;
    virtual std::shared_ptr<StructBase> parseHeaderInMetainfo(IMessage& message, int contentType, bool storeRawData, remoteentity::Header& header, bool& syntaxError) = 0;
    virtual std::shared_ptr<StructBase> parsePureData(IMessage& message, remoteentity::Header& header) = 0;
    virtual bool send(const IProtocolSessionPtr& session, remoteentity::Header& header, Variant&& echoData, const StructBase* structBase = nullptr, IMessage::Metainfo* metainfo = nullptr, Variant* controlData = nullptr) = 0;

    virtual void registerFormat(const std::string& contentTypeName, int contentType, const std::shared_ptr<IRemoteEntityFormat>& format) = 0;
    virtual bool isRegistered(int contentType) const = 0;
    virtual int getContentType(const std::string& contentTypeName) const = 0;
};




class RemoteEntityFormatRegistryImpl : public IRemoteEntityFormatRegistry
{
public:
    virtual std::shared_ptr<StructBase> parse(IMessage& message, int contentType, bool storeRawData, remoteentity::Header& header, bool& syntaxError) override;
    virtual std::shared_ptr<StructBase> parseHeaderInMetainfo(IMessage& message, int contentType, bool storeRawData, remoteentity::Header& header, bool& syntaxError) override;
    virtual std::shared_ptr<StructBase> parsePureData(IMessage& message, remoteentity::Header& header) override;
    virtual bool send(const IProtocolSessionPtr& session, remoteentity::Header& header, Variant&& echoData, const StructBase* structBase = nullptr, IMessage::Metainfo* metainfo = nullptr, Variant* controlData = nullptr) override;
    virtual void registerFormat(const std::string& contentTypeName, int contentType, const std::shared_ptr<IRemoteEntityFormat>& format) override;
    virtual bool isRegistered(int contentType) const override;
    virtual int getContentType(const std::string& contentTypeName) const override;

private:
    void serializeHeaderToMetainfo(IMessage& message, const remoteentity::Header& header);
    void parseMetainfo(IMessage& message, remoteentity::Header& header);
    bool serialize(IMessage& message, int contentType, const remoteentity::Header& header, const StructBase* structBase = nullptr);
    bool serializeData(IMessage& message, int contentType, const StructBase* structBase);

    std::unordered_map<int, std::shared_ptr<IRemoteEntityFormat>>   m_contentTypeToFormat;
    std::unordered_map<std::string, int>                            m_contentTypeNameToContentType;
};


class SYMBOLEXP RemoteEntityFormatRegistry
{
public:
    inline static IRemoteEntityFormatRegistry& instance()
    {
        if (!m_instance)
        {
            m_instance = std::make_unique<RemoteEntityFormatRegistryImpl>();
        }
        return *m_instance;
    }
    static void setInstance(std::unique_ptr<IRemoteEntityFormatRegistry>& instance);

private:
    RemoteEntityFormatRegistry() = delete;

    static std::unique_ptr<IRemoteEntityFormatRegistry> m_instance;
};





}   // namespace finalmq
