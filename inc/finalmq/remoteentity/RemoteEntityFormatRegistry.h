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
#include <mutex>

#include "finalmq/protocolsession/ProtocolSessionContainer.h"
#include "finalmq/remoteentity/IRemoteEntity.h"

namespace finalmq
{
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;

class Header;

enum FormatStatus
{
    FORMATSTATUS_NONE = 0,
    FORMATSTATUS_SYNTAX_ERROR = 1,
    FORMATSTATUS_AUTOMATIC_CONNECT = 2,
    FORMATSTATUS_HEADER_PARSED_BY_FORMAT = 4,
};

struct IRemoteEntityFormat
{
    virtual ~IRemoteEntityFormat()
    {}
    virtual std::shared_ptr<StructBase> parse(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, int& formatStatus) = 0;
    virtual std::shared_ptr<StructBase> parseData(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, std::string& type, int& formatStatus, const std::string& typeOfGeneralMessage) = 0;
    virtual void serialize(const IProtocolSessionPtr& session, IMessage& message, const Header& header, const StructBase* structBase = nullptr) = 0;
    virtual void serializeData(const IProtocolSessionPtr& session, IMessage& message, const StructBase* structBase = nullptr) = 0;
};

struct IRemoteEntityFormatRegistry
{
    virtual ~IRemoteEntityFormatRegistry()
    {}
    virtual std::shared_ptr<StructBase> parse(const IProtocolSessionPtr& session, IMessage& message, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, int& formatStatus) = 0;
    virtual std::shared_ptr<StructBase> parseHeaderInMetainfo(const IProtocolSessionPtr& session, IMessage& message, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, int& formatStatus) = 0;
    virtual void send(const IProtocolSessionPtr& session, const std::string& virtualSessionId, Header& header, Variant&& echoData, const StructBase* structBase = nullptr, IMessage::Metainfo* metainfo = nullptr, Variant* controlData = nullptr) = 0;

    virtual void registerFormat(const std::string& contentTypeName, int contentType, const std::shared_ptr<IRemoteEntityFormat>& format) = 0;
    virtual bool isRegistered(int contentType) const = 0;
    virtual int getContentType(const std::string& contentTypeName) const = 0;
};

class RemoteEntityFormatRegistryImpl : public IRemoteEntityFormatRegistry
{
public:
    virtual std::shared_ptr<StructBase> parse(const IProtocolSessionPtr& session, IMessage& message, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, int& formatStatus) override;
    virtual std::shared_ptr<StructBase> parseHeaderInMetainfo(const IProtocolSessionPtr& session, IMessage& message, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, int& formatStatus) override;
    virtual void send(const IProtocolSessionPtr& session, const std::string& virtualSessionId, Header& header, Variant&& echoData, const StructBase* structBase = nullptr, IMessage::Metainfo* metainfo = nullptr, Variant* controlData = nullptr) override;
    virtual void registerFormat(const std::string& contentTypeName, int contentType, const std::shared_ptr<IRemoteEntityFormat>& format) override;
    virtual bool isRegistered(int contentType) const override;
    virtual int getContentType(const std::string& contentTypeName) const override;

private:
    void serializeHeaderToMetainfo(IMessage& message, const Header& header);
    std::string parseMetainfo(IMessage& message, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, std::string& typeOfGeneralMessage);
    bool serialize(const IProtocolSessionPtr& session, IMessage& message, const Header& header, const StructBase* structBase = nullptr);
    bool serializeData(const IProtocolSessionPtr& session, IMessage& message, const StructBase* structBase);

    std::unordered_map<int, std::shared_ptr<IRemoteEntityFormat>> m_contentTypeToFormat{};
    std::unordered_map<std::string, int> m_contentTypeNameToContentType{};
};

class SYMBOLEXP RemoteEntityFormatRegistry
{
public:
    inline static IRemoteEntityFormatRegistry& instance()
    {
        IRemoteEntityFormatRegistry* instance = getStaticInstanceRef().load(std::memory_order_acquire);
        if (!instance)
        {
            instance = createInstance();
        }
        return *instance;
    }

    /**
    * Overwrite the default implementation, e.g. with a mock for testing purposes.
    * This method is not thread-safe. Make sure that no one uses the current instance before
    * calling this method.
    */
    static void setInstance(std::unique_ptr<IRemoteEntityFormatRegistry>&& instance);

private:
    RemoteEntityFormatRegistry() = delete;
    ~RemoteEntityFormatRegistry() = delete;
    static IRemoteEntityFormatRegistry* createInstance();

    static std::atomic<IRemoteEntityFormatRegistry*>& getStaticInstanceRef();
    static std::unique_ptr<IRemoteEntityFormatRegistry>& getStaticUniquePtrRef();
};

} // namespace finalmq
