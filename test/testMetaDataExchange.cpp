#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "metadataserialize/MetaDataExchange.h"
#include "metadata/MetaData.h"

#include "serializestruct/SerializerStruct.h"
#include "serializejson/ParserJson.h"


using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;



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

const std::string STRUCT_TYPE = "MyTestStructType";
const std::string STRUCT_DESCRIPTION = "struct description";
const SerializeMetaTypeId STRUCT_ENTRY_TYPEID = TYPE_STRING;
const std::string STRUCT_ENTRY_TYPENAME = "JustATypeName";
const std::string STRUCT_ENTRY_NAME = "value";
const std::string STRUCT_ENTRY_DESCRIPTION = "value description";
const std::vector<SerializeMetaFieldFlags> STRUCT_ENTRY_FLAGS = {METAFLAG_PROTO_VARINT, METAFLAG_PROTO_ZIGZAG};


TEST_F(TestMetaDataExchange, testImportEnum)
{
    const MetaEnum* metaEnum = MetaDataGlobal::instance().getEnum(ENUM_TYPE);
    ASSERT_EQ(metaEnum, nullptr);

    SerializeMetaEnum en;

    en.typeName = ENUM_TYPE;
    en.description = ENUM_DESCRIPTION;
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
    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct(STRUCT_TYPE);
    ASSERT_EQ(metaStruct, nullptr);

    SerializeMetaStruct stru;
    stru.typeName = STRUCT_TYPE;
    stru.description = STRUCT_DESCRIPTION;
    stru.fields.push_back({STRUCT_ENTRY_TYPEID, STRUCT_ENTRY_TYPENAME, STRUCT_ENTRY_NAME, STRUCT_ENTRY_DESCRIPTION, STRUCT_ENTRY_FLAGS});

    SerializeMetaData serializeMetaData;
    serializeMetaData.structs.push_back(stru);

    MetaDataExchange::importMetaData(serializeMetaData);

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
    en.typeName = ENUM_TYPE;
    en.description = ENUM_DESCRIPTION;
    en.entries.push_back({ENUM_ENTRY_NAME, ENUM_ENTRY_ID, ENUM_ENTRY_DESCRIPTION});
    SerializeMetaData serializeMetaData;
    serializeMetaData.enums.push_back(en);
    MetaDataExchange::importMetaData(serializeMetaData);

    SerializeMetaData serializeMetaDataExport;
    MetaDataExchange::exportMetaData(serializeMetaDataExport);

    int found = 0;
    for (size_t i = 0; i < serializeMetaDataExport.enums.size(); ++i)
    {
        if (serializeMetaDataExport.enums[i].typeName == ENUM_TYPE)
        {
            found++;
            ASSERT_EQ(serializeMetaDataExport.enums[i], en);
        }
    }
    ASSERT_EQ(found, 1);
}


TEST_F(TestMetaDataExchange, testExportStruct)
{
    SerializeMetaStruct stru;
    stru.typeName = STRUCT_TYPE;
    stru.description = STRUCT_DESCRIPTION;
    stru.fields.push_back({STRUCT_ENTRY_TYPEID, STRUCT_ENTRY_TYPENAME, STRUCT_ENTRY_NAME, STRUCT_ENTRY_DESCRIPTION, STRUCT_ENTRY_FLAGS});
    SerializeMetaData serializeMetaData;
    serializeMetaData.structs.push_back(stru);
    MetaDataExchange::importMetaData(serializeMetaData);

    SerializeMetaData serializeMetaDataExport;
    MetaDataExchange::exportMetaData(serializeMetaDataExport);

    int found = 0;
    for (size_t i = 0; i < serializeMetaDataExport.structs.size(); ++i)
    {
        if (serializeMetaDataExport.structs[i].typeName == STRUCT_TYPE)
        {
            found++;
            ASSERT_EQ(serializeMetaDataExport.structs[i], stru);
        }
    }
    ASSERT_EQ(found, 1);
}


TEST_F(TestMetaDataExchange, testImportJson)
{
    static const std::string JSON = "{\"structs\":["
                                        "{\"typeName\": \"MyNextTestStruct\","
                                          "\"description\": \"struct description\","
                                            "\"fields\": [{"
                                                "\"typeId\": \"TYPE_STRING\","
                                                "\"typeName\": \"JustATypeName\","
                                                "\"name\": \"value\","
                                                "\"description\": \"value description\","
                                                "\"flags\": [\"METAFLAG_PROTO_VARINT\",\"METAFLAG_PROTO_ZIGZAG\"]"
                                            "}]"
                                        "}"
                                    "]}";

    const MetaStruct* metaStruct = MetaDataGlobal::instance().getStruct("MyNextTestStruct");
    ASSERT_EQ(metaStruct, nullptr);

    MetaDataExchange::importMetaDataJson(JSON.c_str());

    metaStruct = MetaDataGlobal::instance().getStruct("MyNextTestStruct");
    ASSERT_NE(metaStruct, nullptr);
    ASSERT_EQ(metaStruct->getTypeName(), "MyNextTestStruct");
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



TEST_F(TestMetaDataExchange, testExportJson)
{
    std::string json;
    MetaDataExchange::exportMetaDataJson(json);

    SerializeMetaData root;
    SerializerStruct serializer(root);
    ParserJson parser(serializer, json.c_str());
    parser.parseStruct("SerializeMetaData");

    SerializeMetaStruct stru;
    stru.typeName = STRUCT_TYPE;
    stru.description = STRUCT_DESCRIPTION;
    stru.fields.push_back({STRUCT_ENTRY_TYPEID, STRUCT_ENTRY_TYPENAME, STRUCT_ENTRY_NAME, STRUCT_ENTRY_DESCRIPTION, STRUCT_ENTRY_FLAGS});

    int found = 0;
    for (size_t i = 0; i < root.structs.size(); ++i)
    {
        if (root.structs[i].typeName == STRUCT_TYPE)
        {
            found++;
            ASSERT_EQ(root.structs[i], stru);
        }
    }
    ASSERT_EQ(found, 1);
}
