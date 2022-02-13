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
#include "finalmq/remoteentity/IRemoteEntity.h"

#include <mutex>
#include <atomic>

namespace finalmq {

typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;

class Header;


struct IRemoteEntityFormat
{
    virtual ~IRemoteEntityFormat() {}
    virtual std::shared_ptr<StructBase> parse(const BufferRef& bufferRef, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError) = 0;
    virtual std::shared_ptr<StructBase> parseData(const BufferRef& bufferRef, bool storeRawData, std::string& type, bool& syntaxError) = 0;
    virtual void serialize(IMessage& message, const Header& header, const StructBase* structBase = nullptr) = 0;
    virtual void serializeData(IMessage& message, const StructBase* structBase = nullptr) = 0;
};


struct IRemoteEntityFormatRegistry
{
    virtual ~IRemoteEntityFormatRegistry() {}
    virtual std::shared_ptr<StructBase> parse(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError) = 0;
    virtual std::shared_ptr<StructBase> parseHeaderInMetainfo(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError) = 0;
//    virtual std::shared_ptr<StructBase> parsePureData(IMessage& message, Header& header) = 0;
    virtual bool send(const IProtocolSessionPtr& session, const std::string& virtualSessionId, Header& header, Variant&& echoData, const StructBase* structBase = nullptr, IMessage::Metainfo* metainfo = nullptr, Variant* controlData = nullptr) = 0;

    virtual void registerFormat(const std::string& contentTypeName, int contentType, const std::shared_ptr<IRemoteEntityFormat>& format) = 0;
    virtual bool isRegistered(int contentType) const = 0;
    virtual int getContentType(const std::string& contentTypeName) const = 0;
};




class RemoteEntityFormatRegistryImpl : public IRemoteEntityFormatRegistry
{
public:
    virtual std::shared_ptr<StructBase> parse(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError) override;
    virtual std::shared_ptr<StructBase> parseHeaderInMetainfo(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError) override;
//    virtual std::shared_ptr<StructBase> parsePureData(IMessage& message, Header& header) override;
    virtual bool send(const IProtocolSessionPtr& session, const std::string& virtualSessionId, Header& header, Variant&& echoData, const StructBase* structBase = nullptr, IMessage::Metainfo* metainfo = nullptr, Variant* controlData = nullptr) override;
    virtual void registerFormat(const std::string& contentTypeName, int contentType, const std::shared_ptr<IRemoteEntityFormat>& format) override;
    virtual bool isRegistered(int contentType) const override;
    virtual int getContentType(const std::string& contentTypeName) const override;

private:
    void serializeHeaderToMetainfo(IMessage& message, const Header& header);
    std::string parseMetainfo(IMessage& message, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header);
    bool serialize(IMessage& message, int contentType, const Header& header, const StructBase* structBase = nullptr);
    bool serializeData(IMessage& message, int contentType, const StructBase* structBase);

    std::unordered_map<int, std::shared_ptr<IRemoteEntityFormat>>   m_contentTypeToFormat;
    std::unordered_map<std::string, int>                            m_contentTypeNameToContentType;
};


class SYMBOLEXP RemoteEntityFormatRegistry
{
public:
    inline static IRemoteEntityFormatRegistry& instance()
    {
        IRemoteEntityFormatRegistry* inst = m_instance.load(std::memory_order_acquire);
        if (!inst)
        {
            inst = createInstance();
        }
        return *inst;
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

    static std::atomic<IRemoteEntityFormatRegistry*> m_instance;
    static std::unique_ptr<IRemoteEntityFormatRegistry> m_instanceUniquePtr;
    static std::mutex m_mutex;
};





}   // namespace finalmq
