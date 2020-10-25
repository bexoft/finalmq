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

#include "metadataserialize/MetaDataExchange.h"
#include "metadata/MetaData.h"
#include "serializejson/ParserJson.h"
#include "serializejson/SerializerJson.h"
#include "serializeproto/ParserProto.h"
#include "serializeproto/SerializerProto.h"
#include "serializestruct/SerializerStruct.h"
#include "serializestruct/ParserStruct.h"
#include "helpers/ZeroCopyBuffer.h"

#include <algorithm>


namespace finalmq {


static MetaTypeId convert(SerializeMetaTypeId value)
{
    return static_cast<MetaTypeId>(static_cast<SerializeMetaTypeId::Enum>(value));
}
static SerializeMetaTypeId convert(MetaTypeId value)
{
    return static_cast<SerializeMetaTypeId::Enum>(static_cast<MetaTypeId>(value));
}

void MetaDataExchange::importMetaData(const SerializeMetaData& metadata)
{
    // enums
    for (size_t i = 0; i < metadata.enums.size(); ++i)
    {
        const SerializeMetaEnum& enumSource = metadata.enums[i];
        std::vector<MetaEnumEntry> entries;
        for (size_t n = 0; n < enumSource.entries.size(); ++n)
        {
            const SerializeMetaEnumEntry& entrySource = enumSource.entries[i];
            entries.push_back({entrySource.name, entrySource.id, entrySource.desc});
        }
        MetaDataGlobal::instance().addEnum({enumSource.type, enumSource.desc, std::move(entries)});
    }

    // structs
    for (size_t i = 0; i < metadata.structs.size(); ++i)
    {
        const SerializeMetaStruct& structSource = metadata.structs[i];
        std::vector<MetaField> fields;
        for (size_t n = 0; n < structSource.fields.size(); ++n)
        {
            const SerializeMetaField& fieldSource = structSource.fields[i];
            int flags = 0;
            std::for_each(fieldSource.flags.begin(), fieldSource.flags.end(), [&flags] (const SerializeMetaFieldFlags& flag) {
                flags |= flag;
            });
            fields.push_back({convert(fieldSource.tid), fieldSource.type, fieldSource.name, fieldSource.desc, flags});
        }
        MetaDataGlobal::instance().addStruct({structSource.type, structSource.desc, std::move(fields)});
    }
}

void MetaDataExchange::exportMetaData(SerializeMetaData& metadata)
{
    metadata.enums.clear();
    metadata.structs.clear();

    // enums
    const std::unordered_map<std::string, MetaEnum>& enumsSource = MetaDataGlobal::instance().getAllEnums();
    for (auto it = enumsSource.begin(); it != enumsSource.end(); ++it)
    {
        const MetaEnum& enumSource = it->second;
        SerializeMetaEnum enumDestination;
        enumDestination.type = enumSource.getTypeName();
        enumDestination.desc = enumSource.getDescription();
        for (int n = 0; n < enumSource.getEntrySize(); ++n)
        {
            const MetaEnumEntry* entrySource = enumSource.getEntryByIndex(n);
            assert(entrySource);
            enumDestination.entries.push_back({entrySource->name, entrySource->id, entrySource->description});
        }
        metadata.enums.push_back(std::move(enumDestination));
    }

    // structs
    const std::unordered_map<std::string, MetaStruct>& structsSource = MetaDataGlobal::instance().getAllStructs();
    for (auto it = structsSource.begin(); it != structsSource.end(); ++it)
    {
        const MetaStruct& structSource = it->second;
        SerializeMetaStruct structDestination;
        structDestination.type = structSource.getTypeName();
        structDestination.desc = structSource.getDescription();
        for (int n = 0; n < structSource.getFieldsSize(); ++n)
        {
            const MetaField* fieldSource = structSource.getFieldByIndex(n);
            assert(fieldSource);
            std::vector<SerializeMetaFieldFlags> flags;
            for (int k = 0; k < 32; ++k)
            {
                std::int32_t flag = 1 << k;
                if (fieldSource->flags & flag)
                {
                    flags.push_back(static_cast<SerializeMetaFieldFlags::Enum>(flag));
                }
            }
            structDestination.fields.push_back({convert(fieldSource->typeId), fieldSource->typeName, fieldSource->name, fieldSource->description, flags});
        }
        metadata.structs.push_back(std::move(structDestination));
    }
}



void MetaDataExchange::importMetaDataJson(const char* json)
{
    SerializeMetaData root;
    SerializerStruct serializer(root);
    ParserJson parser(serializer, json);
    parser.parseStruct("finalmq.SerializeMetaData");

    importMetaData(root);
}


void MetaDataExchange::exportMetaDataJson(std::string& json)
{
    SerializeMetaData root;
    exportMetaData(root);

    ZeroCopyBuffer buffer;
    SerializerJson serializer(buffer, 8192, true, false);
    ParserStruct parser(serializer, root);
    parser.parseStruct();

    json = buffer.getData();
}



void MetaDataExchange::importMetaDataProto(const char* proto, int size)
{
    SerializeMetaData root;
    SerializerStruct serializer(root);
    ParserProto parser(serializer, proto, size);
    parser.parseStruct("finalmq.SerializeMetaData");

    importMetaData(root);
}


void MetaDataExchange::exportMetaDataProto(std::string& proto)
{
    SerializeMetaData root;
    exportMetaData(root);

    ZeroCopyBuffer buffer;
    SerializerProto serializer(buffer, 8192);
    ParserStruct parser(serializer, root);
    parser.parseStruct();

    proto = buffer.getData();
}

}   // namespace finalmq
