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

#include "RemoteEntityFormatRegistry.h"


namespace finalmq {


class SYMBOLEXP RemoteEntityFormatHl7 : public IRemoteEntityFormat
{
public:
    static const int CONTENT_TYPE;                  // 3
    static const std::string CONTENT_TYPE_NAME;     // hl7

    static const std::string PROPERTY_NAMESPACE;    // hl7namespace
    static const std::string PROPERTY_ENTITY;       // hl7entity


private:
    virtual std::shared_ptr<StructBase> parse(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, int& formatStatus) override;
    virtual std::shared_ptr<StructBase> parseData(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, std::string& type, int& formatStatus) override;
    virtual void serialize(const IProtocolSessionPtr& session, IMessage& message, const Header& header, const StructBase* structBase = nullptr) override;
    virtual void serializeData(const IProtocolSessionPtr& session, IMessage& message, const StructBase* structBase = nullptr) override;
};



}   // namespace finalmq
