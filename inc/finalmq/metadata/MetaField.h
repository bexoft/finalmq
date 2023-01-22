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


#include "MetaType.h"
#include "finalmq/helpers/FmqDefines.h"

#include <string>
#include <memory>


namespace finalmq {


class MetaEnum;
class MetaStruct;
struct IMetaData;


enum MetaFieldFlags : std::int32_t
{
    METAFLAG_NONE           = 0,
    METAFLAG_PROTO_VARINT   = 1,
    METAFLAG_PROTO_ZIGZAG   = 2,
    METAFLAG_NULLABLE       = 4,    // only for struct
};


class MetaField
{
public:
    MetaTypeId      typeId;                                 ///< type id of the parameter
    std::string     typeName;                               ///< is needed for struct and enum
    std::string     name{};                                 ///< parameter name
    std::string     description{};                          ///< description of the parameter
    int             flags = 0;                              ///< flaggs of the parameter
    int             index = -1;                             ///< index of field inside struct

    mutable const MetaEnum*     metaEnum    = nullptr;      ///< cache to find MetaEnum of typeName faster
    mutable const MetaStruct*   metaStruct  = nullptr;      ///< cache to find MetaStruct of typeName faster
    mutable std::shared_ptr<MetaField> fieldWithoutArray{}; ///< in case of an array, this is the MetaField for its entries
};


}   // namespace finalmq
