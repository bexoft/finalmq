#pragma once


#include "MetaType.h"

#include <string>
#include <memory>



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
    MetaTypeId      typeId;                                 ///< type id of the parameter
    std::string     typeName;                               ///< is needed for struct and enum
    std::string     name;                                   ///< parameter name
    std::string     description;                            ///< description of the parameter

    int             index = -1;                             ///< index of field inside struct

    int             flags = 0;                              ///< flaggs of the parameter

    mutable const MetaEnum*     metaEnum    = nullptr;      ///< cache to find MetaEnum of typeName faster
    mutable const MetaStruct*   metaStruct  = nullptr;      ///< cache to find MetaStruct of typeName faster
    mutable std::shared_ptr<MetaField> fieldWithoutArray;   ///< in case of an array, this is the MetaField for its entries
};


