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

#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "metadataserialize/MetaDataExchange.h"
#include "metadata/MetaData.h"

#include "serializestruct/ParserStruct.h"
#include "serializestruct/SerializerStruct.h"
#include "serializejson/ParserJson.h"
#include "serializeproto/ParserProto.h"
#include "serializeproto/SerializerProto.h"
#include "helpers/ZeroCopyBuffer.h"


using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;

using namespace finalmq;

class TestMetaDataExchange : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

};


const std::string ENUM_TYPE = "MyTestEnumType";
const std::string ENUM_DESCRIPTION = "enum description 0";
const std::string ENUM_ENTRY_NAME = "ENTRY_TEST1";
const std::int32_t ENUM_ENTRY_ID = 0;
const std::string ENUM_ENTRY_DESCRIPTION = "enum description 1";

const std::string STRUCT_DESCRIPTION = "struct description";
const SerializeMetaTypeId STRUCT_ENTRY_TYPEID = SerializeMetaTypeId::TYPE_STRING;
const std::string STRUCT_ENTRY_TYPENAME = "JustATypeName";
const std::string STRUCT_ENTRY_NAME = "value";
const std::string STRUCT_ENTRY_DESCRIPTION = "value description";
const std::vector<SerializeMetaFieldFlags> STRUCT_ENTRY_FLAGS = {SerializeMetaFieldFlags::METAFLAG_PROTO_VARINT, SerializeMetaFieldFlags::METAFLAG_PROTO_ZIGZAG};


static SerializeMetaStruct importTestStruct(const std::string& typeName)
{
    SerializeMetaStruct stru;
    stru.type = typeName;
    stru.desc = STRUCT_DESCRIPTION;
    stru.fields.push_back({STRUCT_ENTRY_TYPEID, STRUCT_ENTRY_TYPENAME, STRUCT_ENTRY_NAME, STRUCT_ENTRY_DESCRIPTION, STRUCT_ENTRY_FLAGS});
    SerializeMetaData serializeMetaData;
    serializeMetaData.structs.push_back(stru);
    MetaDataExchange::importMetaData(serializeMetaData);

    return stru;
}



TEST_F(TestMetaDataExchange, testImportEnum)
{
    const MetaEnum* metaEnum = MetaDataGlobal::instance().getEnum(ENUM_TYPE);
    ASSERT_EQ(metaEnum, nullptr);

    SerializeMetaEnum en;

    en.type = ENUM_TYPE;
    en.desc = ENUM_DESCRIPTION;
    en.entries.push_back({ENUM_ENTRY_NAME, ENUM_ENTRY_ID, ENUM_ENTRY_DESCRIPTION});

    SerializeMetaData serializeMetaData;
    serializeMetaData.enums.push_back(en);

    MetaDataExchange::importMetaData(serializeMetaData);

    metaEnum = MetaDataGlobal::instance().getEnum(ENUM_TYPE);
    ASSERT_NE(metaEnum, nullptr);
    ASSERT_EQ(metaEnum->getTypeName(), ENUM_TYPE);
    ASSERT_EQ(metaEnum->getDescription(), ENUM_DESCRIPTION);
    ASSERT_EQ(metaEnum->getEntrySize(), 1);
    const MetaEnumEntry* enumEntry = metaEnum->getEntryByIndex(0);
    ASSERT_NE(enumEntry, nullptr);
    ASSERT_EQ(enumEntry->name, ENUM_ENTRY_NAME);
    ASSERT_EQ(enumEntry->id, ENUM_ENTRY_ID);
    ASSERT_EQ(enumEntry->description, ENUM_ENTRY_DESCRIPTION);
}

TEST_F(TestMetaDataExchange, testImportStruct)
{
    const std::string STRUCT_TYPE = "MyTest_testImportStruct";

    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct(STRUCT_TYPE);
    ASSERT_EQ(metaStruct, nullptr);

    importTestStruct(STRUCT_TYPE);

    metaStruct = MetaDataGlobal::instance().getStruct(STRUCT_TYPE);
    ASSERT_NE(metaStruct, nullptr);
    ASSERT_EQ(metaStruct->getTypeName(), STRUCT_TYPE);
    ASSERT_EQ(metaStruct->getDescription(), STRUCT_DESCRIPTION);
    ASSERT_EQ(metaStruct->getFieldsSize(), 1);
    const MetaField* field = metaStruct->getFieldByIndex(0);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(field->typeId, STRUCT_ENTRY_TYPEID);
    ASSERT_EQ(field->typeName, STRUCT_ENTRY_TYPENAME);
    ASSERT_EQ(field->name, STRUCT_ENTRY_NAME);
    ASSERT_EQ(field->description, STRUCT_ENTRY_DESCRIPTION);
    ASSERT_EQ(field->flags, METAFLAG_PROTO_VARINT | METAFLAG_PROTO_ZIGZAG);
}



TEST_F(TestMetaDataExchange, testExportEnum)
{
    SerializeMetaEnum en;
    en.type = ENUM_TYPE;
    en.desc = ENUM_DESCRIPTION;
    en.entries.push_back({ENUM_ENTRY_NAME, ENUM_ENTRY_ID, ENUM_ENTRY_DESCRIPTION});
    SerializeMetaData serializeMetaData;
    serializeMetaData.enums.push_back(en);
    MetaDataExchange::importMetaData(serializeMetaData);

    SerializeMetaData serializeMetaDataExport;
    MetaDataExchange::exportMetaData(serializeMetaDataExport);

    int found = 0;
    for (size_t i = 0; i < serializeMetaDataExport.enums.size(); ++i)
    {
        if (serializeMetaDataExport.enums[i].type == ENUM_TYPE)
        {
            found++;
            ASSERT_EQ(serializeMetaDataExport.enums[i], en);
        }
    }
    ASSERT_EQ(found, 1);
}


TEST_F(TestMetaDataExchange, testExportStruct)
{
    static const std::string STRUCT_TYPE = "MyTest_testExportStruct";

    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct(STRUCT_TYPE);
    ASSERT_EQ(metaStruct, nullptr);

    SerializeMetaStruct stru = importTestStruct(STRUCT_TYPE);

    SerializeMetaData serializeMetaDataExport;
    MetaDataExchange::exportMetaData(serializeMetaDataExport);

    int found = 0;
    for (size_t i = 0; i < serializeMetaDataExport.structs.size(); ++i)
    {
        if (serializeMetaDataExport.structs[i].type == STRUCT_TYPE)
        {
            found++;
            ASSERT_EQ(serializeMetaDataExport.structs[i], stru);
        }
    }
    ASSERT_EQ(found, 1);
}


TEST_F(TestMetaDataExchange, testImportProto)
{
    static const std::string STRUCT_TYPE = "MyTest_testImportProto";

    SerializeMetaStruct stru;
    stru.type = STRUCT_TYPE;
    stru.desc = STRUCT_DESCRIPTION;
    stru.fields.push_back({STRUCT_ENTRY_TYPEID, STRUCT_ENTRY_TYPENAME, STRUCT_ENTRY_NAME, STRUCT_ENTRY_DESCRIPTION, STRUCT_ENTRY_FLAGS});
    SerializeMetaData serializeMetaData;
    serializeMetaData.structs.push_back(stru);

    ZeroCopyBuffer buffer;
    SerializerProto serializer(buffer, 8192);
    ParserStruct parser(serializer, serializeMetaData);
    parser.parseStruct();

    std::string proto = buffer.getData();

    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct(STRUCT_TYPE);
    ASSERT_EQ(metaStruct, nullptr);

    MetaDataExchange::importMetaDataProto(proto.c_str(), proto.size());

    metaStruct = MetaDataGlobal::instance().getStruct(STRUCT_TYPE);
    ASSERT_NE(metaStruct, nullptr);
    ASSERT_EQ(metaStruct->getTypeName(), STRUCT_TYPE);
    ASSERT_EQ(metaStruct->getDescription(), STRUCT_DESCRIPTION);
    ASSERT_EQ(metaStruct->getFieldsSize(), 1);
    const MetaField* field = metaStruct->getFieldByIndex(0);
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(field->typeId, STRUCT_ENTRY_TYPEID);
    ASSERT_EQ(field->typeName, STRUCT_ENTRY_TYPENAME);
    ASSERT_EQ(field->name, STRUCT_ENTRY_NAME);
    ASSERT_EQ(field->description, STRUCT_ENTRY_DESCRIPTION);
    ASSERT_EQ(field->flags, METAFLAG_PROTO_VARINT | METAFLAG_PROTO_ZIGZAG);
}



TEST_F(TestMetaDataExchange, testExportProto)
{
    static const std::string STRUCT_TYPE = "MyTest_testExportProto";

    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct(STRUCT_TYPE);
    ASSERT_EQ(metaStruct, nullptr);

    SerializeMetaStruct stru = importTestStruct(STRUCT_TYPE);

    std::string proto;
    MetaDataExchange::exportMetaDataProto(proto);

    SerializeMetaData root;
    SerializerStruct serializer(root);
    ParserProto parser(serializer, proto.c_str(), proto.size());
    parser.parseStruct("finalmq.SerializeMetaData");

    int found = 0;
    for (size_t i = 0; i < root.structs.size(); ++i)
    {
        if (root.structs[i].type == STRUCT_TYPE)
        {
            found++;
            ASSERT_EQ(root.structs[i], stru);
        }
    }
    ASSERT_EQ(found, 1);
}
