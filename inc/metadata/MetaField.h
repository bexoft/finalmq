#pragma once


#include <string>
#include <memory>


enum class MetaType
{
    TYPE_NONE       = 0,

    TYPE_BOOL       = 1,
    TYPE_INT32      = 2,
    TYPE_UINT32     = 3,
    TYPE_INT64      = 4,
    TYPE_UINT64     = 5,
    TYPE_FLOAT      = 6,
    TYPE_DOUBLE     = 7,
    TYPE_STRING     = 8,
    TYPE_BYTES      = 9,
    TYPE_STRUCT     = 10,
    TYPE_ENUM       = 11,

    TYPE_ARRAY_FLAG       = 32,

    TYPE_ARRAY_BOOL       = 32 + 1,
    TYPE_ARRAY_INT32      = 32 + 2,
    TYPE_ARRAY_UINT32     = 32 + 3,
    TYPE_ARRAY_INT64      = 32 + 4,
    TYPE_ARRAY_UINT64     = 32 + 5,
    TYPE_ARRAY_FLOAT      = 32 + 6,
    TYPE_ARRAY_DOUBLE     = 32 + 7,
    TYPE_ARRAY_STRING     = 32 + 8,
    TYPE_ARRAY_BYTES      = 32 + 9,
    TYPE_ARRAY_STRUCT     = 32 + 10,
    TYPE_ARRAY_ENUM       = 32 + 11,
};


class MetaEnum;
class MetaStruct;
class IMetaData;


enum MetaFieldFlags
{
    METAFLAG_PROTO_VARINT = 1,
    METAFLAG_PROTO_ZIGZAG = 2,
};


class MetaField
{
public:
    MetaType        type;                                   ///< type of the parameter
    std::string     typeName;                               ///< is needed for struct and enum
    std::string     name;                                   ///< parameter name
    std::string     description;                            ///< description of the parameter

    int             index = -1;                             ///< index of field inside struct

    int             flags = 0;                              ///< flaggs of the parameter

    mutable const MetaEnum*     metaEnum    = nullptr;      ///< cache to find MetaEnum of typeName faster
    mutable const MetaStruct*   metaStruct  = nullptr;      ///< cache to find MetaStruct of typeName faster
    mutable std::shared_ptr<MetaField> fieldWithoutArray;   ///< in case of an array, this is the MetaField for its entries
};


