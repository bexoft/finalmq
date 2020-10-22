#include "metadataserialize/StructsMetaData.h"



SerializeMetaTypeId::SerializeMetaTypeId()
{
}
SerializeMetaTypeId::SerializeMetaTypeId(Enum en)
    : m_value(en)
{
}
SerializeMetaTypeId::operator const Enum&() const
{
    return m_value;
}
SerializeMetaTypeId::operator Enum&()
{
    return m_value;
}
const SerializeMetaTypeId& SerializeMetaTypeId::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& SerializeMetaTypeId::toString() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
void SerializeMetaTypeId::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo SerializeMetaTypeId::_enumInfo = {
    "SerializeMetaTypeId", "description", {
        {"TYPE_NONE",    TYPE_NONE, "description"},
        {"TYPE_BOOL",    TYPE_BOOL, "description"},
        {"TYPE_INT32",   TYPE_INT32, "description"},
        {"TYPE_UINT32",  TYPE_UINT32, "description"},
        {"TYPE_INT64",   TYPE_INT64, "description"},
        {"TYPE_UINT64",  TYPE_UINT64, "description"},
        {"TYPE_FLOAT",   TYPE_FLOAT, "description"},
        {"TYPE_DOUBLE",  TYPE_DOUBLE, "description"},
        {"TYPE_STRING",  TYPE_STRING, "description"},
        {"TYPE_BYTES",   TYPE_BYTES, "description"},
        {"TYPE_STRUCT",  TYPE_STRUCT, "description"},
        {"TYPE_ENUM",    TYPE_ENUM, "description"},

        {"OFFSET_ARRAY_FLAG", OFFSET_ARRAY_FLAG, "description"},

        {"TYPE_ARRAY_BOOL",    TYPE_ARRAY_BOOL, "description"},
        {"TYPE_ARRAY_INT32",   TYPE_ARRAY_INT32, "description"},
        {"TYPE_ARRAY_UINT32",  TYPE_ARRAY_UINT32, "description"},
        {"TYPE_ARRAY_INT64",   TYPE_ARRAY_INT64, "description"},
        {"TYPE_ARRAY_UINT64",  TYPE_ARRAY_UINT64, "description"},
        {"TYPE_ARRAY_FLOAT",   TYPE_ARRAY_FLOAT, "description"},
        {"TYPE_ARRAY_DOUBLE",  TYPE_ARRAY_DOUBLE, "description"},
        {"TYPE_ARRAY_STRING",  TYPE_ARRAY_STRING, "description"},
        {"TYPE_ARRAY_BYTES",   TYPE_ARRAY_BYTES, "description"},
        {"TYPE_ARRAY_STRUCT",  TYPE_ARRAY_STRUCT, "description"},
        {"TYPE_ARRAY_ENUM",    TYPE_ARRAY_ENUM, "description"},
     }
};




SerializeMetaFieldFlags::SerializeMetaFieldFlags()
{
}
SerializeMetaFieldFlags::SerializeMetaFieldFlags(Enum en)
    : m_value(en)
{
}
SerializeMetaFieldFlags::operator const Enum&() const
{
    return m_value;
}
SerializeMetaFieldFlags::operator Enum&()
{
    return m_value;
}
const SerializeMetaFieldFlags& SerializeMetaFieldFlags::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& SerializeMetaFieldFlags::toString() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
void SerializeMetaFieldFlags::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo SerializeMetaFieldFlags::_enumInfo = {
    "SerializeMetaFieldFlags", "description", {
        {"METAFLAG_NONE",           METAFLAG_NONE, "description"},
        {"METAFLAG_PROTO_VARINT",   METAFLAG_PROTO_VARINT, "description"},
        {"METAFLAG_PROTO_ZIGZAG",   METAFLAG_PROTO_ZIGZAG, "description"},
     }
};






SerializeMetaEnumEntry::SerializeMetaEnumEntry()
{
}
SerializeMetaEnumEntry::SerializeMetaEnumEntry(const decltype(name)& name_, const decltype(id)& id_, const decltype(description)& description_)
    : name(name_)
    , id(id_)
    , description(description_)
{
}
void SerializeMetaEnumEntry::clear()
{
    *this = SerializeMetaEnumEntry();
}
const StructInfo& SerializeMetaEnumEntry::getStructInfo() const
{
    return _structInfo;
}
bool SerializeMetaEnumEntry::operator ==(const SerializeMetaEnumEntry& rhs) const
{
    return (name == rhs.name &&
            id == rhs.id &&
            description == rhs.description);
}
bool SerializeMetaEnumEntry::operator !=(const SerializeMetaEnumEntry& rhs) const
{
    return !(*this == rhs);
}
const StructInfo SerializeMetaEnumEntry::_structInfo = {
    "SerializeMetaEnumEntry", "description", {
         {TYPE_STRING, "",  "name",         "description", 0},
         {TYPE_INT32,  "",  "id",           "description", 0},
         {TYPE_STRING, "",  "description",  "description", 0}
     },{
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaEnumEntry, name)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaEnumEntry, id)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaEnumEntry, description)}
     }
};


SerializeMetaEnum::SerializeMetaEnum()
{
}
SerializeMetaEnum::SerializeMetaEnum(const decltype(typeName)& typeName_, const decltype(description)& description_, const decltype(entries)& entries_)
    : typeName(typeName_)
    , description(description_)
    , entries(entries_)
{
}
void SerializeMetaEnum::clear()
{
    *this = SerializeMetaEnum();
}
const StructInfo& SerializeMetaEnum::getStructInfo() const
{
    return _structInfo;
}
bool SerializeMetaEnum::operator ==(const SerializeMetaEnum& rhs) const
{
    return (typeName == rhs.typeName &&
            description == rhs.description &&
            entries == rhs.entries);
}
bool SerializeMetaEnum::operator !=(const SerializeMetaEnum& rhs) const
{
    return !(*this == rhs);
}
const StructInfo SerializeMetaEnum::_structInfo = {
    "SerializeMetaEnum", "description", {
        {TYPE_STRING,                             "",   "typeName",     "description", 0},
        {TYPE_STRING,                             "",   "description",  "description", 0},
        {TYPE_ARRAY_STRUCT, "SerializeMetaEnumEntry",   "entries",      "description", 0}
    },{
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaEnum, typeName)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaEnum, description)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaEnum, entries), new ArrayStructAdapter<SerializeMetaEnumEntry>}
     }
};




SerializeMetaField::SerializeMetaField()
{
}
SerializeMetaField::SerializeMetaField(const decltype(typeId)& typeId_, const decltype(typeName)& typeName_, const decltype(name)& name_,
                                       const decltype(description)& description_, const decltype(flags)& flags_)
    : typeId(typeId_)
    , typeName(typeName_)
    , name(name_)
    , description(description_)
    , flags(flags_)
{
}
void SerializeMetaField::clear()
{
    *this = SerializeMetaField();
}
const StructInfo& SerializeMetaField::getStructInfo() const
{
    return _structInfo;
}
bool SerializeMetaField::operator ==(const SerializeMetaField& rhs) const
{
    return (typeId == rhs.typeId &&
            typeName == rhs.typeName &&
            name == rhs.name &&
            description == rhs.description &&
            flags == rhs.flags);
}
bool SerializeMetaField::operator !=(const SerializeMetaField& rhs) const
{
    return !(*this == rhs);
}
const StructInfo SerializeMetaField::_structInfo = {
    "SerializeMetaField", "description", {
        {TYPE_ENUM,         "SerializeMetaTypeId",  "typeId",       "description", 0},
        {TYPE_STRING,       "",                     "typeName",     "description", 0},
        {TYPE_STRING,       "",                     "name",         "description", 0},
        {TYPE_STRING,       "",                     "description",  "description", 0},
        {TYPE_ARRAY_STRING, "",                     "flags",        "description", 0}
     },{
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaField, typeId)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaField, typeName)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaField, name)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaField, description)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaField, flags)}
     }
};



SerializeMetaStruct::SerializeMetaStruct()
{
}
SerializeMetaStruct::SerializeMetaStruct(const decltype(typeName)& typeName_, const decltype(description)& description_, const decltype(fields)& fields_)
    : typeName(typeName_)
    , description(description_)
    , fields(fields_)
{
}
void SerializeMetaStruct::clear()
{
    *this = SerializeMetaStruct();
}
const StructInfo& SerializeMetaStruct::getStructInfo() const
{
    return _structInfo;
}
bool SerializeMetaStruct::operator ==(const SerializeMetaStruct& rhs) const
{
    return (typeName == rhs.typeName &&
            description == rhs.description &&
            fields == rhs.fields);
}
bool SerializeMetaStruct::operator !=(const SerializeMetaStruct& rhs) const
{
    return !(*this == rhs);
}
const StructInfo SerializeMetaStruct::_structInfo = {
    "SerializeMetaStruct", "description", {
        {TYPE_STRING,                         "", "typeName",     "description", 0},
        {TYPE_STRING,                         "", "description",  "description", 0},
        {TYPE_ARRAY_STRUCT, "SerializeMetaField", "fields",       "description", 0}
    },{
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaStruct, typeName)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaStruct, description)},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaStruct, fields), new ArrayStructAdapter<SerializeMetaField>}
     }
};




SerializeMetaData::SerializeMetaData()
{
}
SerializeMetaData::SerializeMetaData(const decltype(enums)& enums_, const decltype(structs)& structs_)
    : enums(enums_)
    , structs(structs_)
{
}
void SerializeMetaData::clear()
{
    *this = SerializeMetaData();
}
const StructInfo& SerializeMetaData::getStructInfo() const
{
    return _structInfo;
}
bool SerializeMetaData::operator ==(const SerializeMetaData& rhs) const
{
    return (enums == rhs.enums &&
            structs == rhs.structs);
}
bool SerializeMetaData::operator !=(const SerializeMetaData& rhs) const
{
    return !(*this == rhs);
}
const StructInfo SerializeMetaData::_structInfo = {
    "SerializeMetaData", "description", {
        {TYPE_ARRAY_STRUCT, "SerializeMetaEnum",   "enums",   "description", 0},
        {TYPE_ARRAY_STRUCT, "SerializeMetaStruct", "structs", "description", 0}
    },{
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaData, enums),   new ArrayStructAdapter<SerializeMetaEnum>},
        {OFFSET_STRUCTBASE_TO_PARAM(SerializeMetaData, structs), new ArrayStructAdapter<SerializeMetaStruct>}
     }
};

