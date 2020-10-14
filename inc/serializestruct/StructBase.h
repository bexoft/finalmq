#pragma once

#include "metadata/MetaField.h"

#include <cstdint>
#include <vector>



class StructBase
{

};


struct StructInfo
{
    MetaField   field;
    int         offset;
};


#define OFFSET_STRUCTBASE_TO_PARAM(type, param)         ((int)((long long)(&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))
#define OFFSET_STRUCTBASE_TO_STRUCTBASE(type, param)    ((int)((long long)((StructBase*)&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))


////////////////////////////

#include <string>

namespace ns1
{
struct ExampleStruct1 : public StructBase
{
    std::int32_t a;
    std::string  b;

    static const std::vector<StructInfo> _structInfo;
};
const std::vector<StructInfo> ExampleStruct1::_structInfo = {
    {{TYPE_INT32,  "", "a", "description", 0, 0}, offsetof(ExampleStruct1, a)},
    {{TYPE_STRING, "", "b", "description", 1, 0}, offsetof(ExampleStruct1, b)}
};
}

namespace ns1 {

struct ExampleStruct2 : public StructBase
{
    std::int32_t a;
    std::string  b;
    ExampleStruct1 c;

    static const std::vector<StructInfo> _structInfo;
};


const std::vector<StructInfo> ExampleStruct2::_structInfo = {
    {{TYPE_INT32,  "", "a", "description", 0, 0}, OFFSET_STRUCTBASE_TO_PARAM(ExampleStruct2, a)},
    {{TYPE_STRING, "", "b", "description", 1, 0}, OFFSET_STRUCTBASE_TO_PARAM(ExampleStruct2, b)},
    {{TYPE_STRUCT, "ns1.ExampleStruct1", "c", "description", 2, 0}, OFFSET_STRUCTBASE_TO_STRUCTBASE(ExampleStruct2, c)}
};

}
