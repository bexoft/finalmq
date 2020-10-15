#include "testStructs.h"



test::Foo::Foo()
{
}
test::Foo::Foo(Enum en)
    : m_value(en)
{
}
test::Foo::operator Enum() const
{
    return m_value;
}
const test::Foo& test::Foo::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& test::Foo::toString() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
void test::Foo::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo test::Foo::_enumInfo = {
    "test.Foo", {
        {"FOO_WORLD",   0, "description"},
        {"FOO_HELLO",  -2, "description"},
        {"FOO_WORLD2",  1, "description"},
     }
};




const StructInfo test::TestStructBool::_structInfo = {
    "test.TestStructBool", {
         {TYPE_BOOL,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructBool, value)
     }
};

const StructInfo test::TestStructInt32::_structInfo = {
    "test.TestStructInt32", {
         {TYPE_INT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructInt32, value)
     }
};

const StructInfo test::TestStructUInt32::_structInfo = {
    "test.TestStructUInt32", {
         {TYPE_UINT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructUInt32, value)
     }
};

const StructInfo test::TestStructInt64::_structInfo = {
    "test.TestStructInt64", {
         {TYPE_INT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructInt64, value)
     }
};

const StructInfo test::TestStructUInt64::_structInfo = {
    "test.TestStructUInt64", {
         {TYPE_UINT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructUInt64, value)
     }
};

const StructInfo test::TestStructFloat::_structInfo = {
    "test.TestStructFloat", {
         {TYPE_FLOAT,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructFloat, value)
     }
};

const StructInfo test::TestStructDouble::_structInfo = {
    "test.TestStructDouble", {
         {TYPE_DOUBLE,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructDouble, value)
     }
};

const StructInfo test::TestStructString::_structInfo = {
    "test.TestStructString", {
         {TYPE_STRING,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructString, value)
     }
};

const StructInfo test::TestStructBytes::_structInfo = {
    "test.TestStructBytes", {
         {TYPE_BYTES,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructBytes, value)
     }
};

const StructInfo test::TestStructStruct::_structInfo = {
    "test.TestStructStruct", {
         {TYPE_STRUCT,  "test.TestStructInt32",     "struct_int32", "description", 0, 0},
         {TYPE_STRUCT,  "test.TestStructString",    "struct_string","description", 1, 0},
         {TYPE_INT32,   "",                         "last_value",   "description", 2, 0}
     },{
        OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStructStruct, struct_int32),
        OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStructStruct, struct_string),
        OFFSET_STRUCTBASE_TO_PARAM(TestStructStruct, last_value)
     }
};



const StructInfo test::TestStructEnum::_structInfo = {
    "test.TestStructEnum", {
         {TYPE_ENUM,  "test.Enum", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestStructEnum, value)
     }
};
