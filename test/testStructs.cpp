#include "testStructs.h"



fmq::test::Foo::Foo()
{
}
fmq::test::Foo::Foo(Enum en)
    : m_value(en)
{
}
fmq::test::Foo::operator Enum() const
{
    return m_value;
}
const fmq::test::Foo& fmq::test::Foo::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& fmq::test::Foo::toString() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
void fmq::test::Foo::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const EnumInfo fmq::test::Foo::_enumInfo = {
    "test.Foo", {
        {"FOO_WORLD",   0, "description"},
        {"FOO_HELLO",  -2, "description"},
        {"FOO_WORLD2",  1, "description"},
     }
};




const StructInfo fmq::test::TestBool::_structInfo = {
    "test.TestBool", {
         {TYPE_BOOL,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestBool, value)
     }
};

const StructInfo fmq::test::TestInt32::_structInfo = {
    "test.TestInt32", {
         {TYPE_INT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestInt32, value)
     }
};

const StructInfo fmq::test::TestUInt32::_structInfo = {
    "test.TestUInt32", {
         {TYPE_UINT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestUInt32, value)
     }
};

const StructInfo fmq::test::TestInt64::_structInfo = {
    "test.TestInt64", {
         {TYPE_INT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestInt64, value)
     }
};

const StructInfo fmq::test::TestUInt64::_structInfo = {
    "test.TestUInt64", {
         {TYPE_UINT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestUInt64, value)
     }
};

const StructInfo fmq::test::TestFloat::_structInfo = {
    "test.TestFloat", {
         {TYPE_FLOAT,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestFloat, value)
     }
};

const StructInfo fmq::test::TestDouble::_structInfo = {
    "test.TestDouble", {
         {TYPE_DOUBLE,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestDouble, value)
     }
};

const StructInfo fmq::test::TestString::_structInfo = {
    "test.TestString", {
         {TYPE_STRING,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestString, value)
     }
};

const StructInfo fmq::test::TestBytes::_structInfo = {
    "test.TestBytes", {
         {TYPE_BYTES,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestBytes, value)
     }
};

const StructInfo fmq::test::TestStruct::_structInfo = {
    "test.TestStruct", {
         {TYPE_STRUCT,  "test.TestInt32",     "struct_int32", "description", 0, 0},
         {TYPE_STRUCT,  "test.TestString",    "struct_string","description", 1, 0},
         {TYPE_INT32,   "",                   "last_value",   "description", 2, 0}
     },{
        OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStruct, struct_int32),
        OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStruct, struct_string),
        OFFSET_STRUCTBASE_TO_PARAM(TestStruct, last_value)
     }
};



const StructInfo fmq::test::TestEnum::_structInfo = {
    "test.TestEnum", {
         {TYPE_ENUM,  "test.Foo", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestEnum, value)
     }
};


const StructInfo fmq::test::TestArrayBool::_structInfo = {
    "test.TestArrayBool", {
         {TYPE_ARRAY_BOOL,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayBool, value)
     }
};

const StructInfo fmq::test::TestArrayInt32::_structInfo = {
    "test.TestArrayInt32", {
         {TYPE_ARRAY_INT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayInt32, value)
     }
};

const StructInfo fmq::test::TestArrayUInt32::_structInfo = {
    "test.TestArrayUInt32", {
         {TYPE_ARRAY_UINT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayUInt32, value)
     }
};

const StructInfo fmq::test::TestArrayInt64::_structInfo = {
    "test.TestArrayInt64", {
         {TYPE_ARRAY_INT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayInt64, value)
     }
};

const StructInfo fmq::test::TestArrayUInt64::_structInfo = {
    "test.TestArrayUInt64", {
         {TYPE_ARRAY_UINT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayUInt64, value)
     }
};

const StructInfo fmq::test::TestArrayFloat::_structInfo = {
    "test.TestArrayFloat", {
         {TYPE_ARRAY_FLOAT,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayFloat, value)
     }
};

const StructInfo fmq::test::TestArrayDouble::_structInfo = {
    "test.TestArrayDouble", {
         {TYPE_ARRAY_DOUBLE,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayDouble, value)
     }
};

const StructInfo fmq::test::TestArrayString::_structInfo = {
    "test.TestArrayString", {
         {TYPE_ARRAY_STRING,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayString, value)
     }
};

const StructInfo fmq::test::TestArrayBytes::_structInfo = {
    "test.TestArrayBytes", {
         {TYPE_ARRAY_BYTES,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayBytes, value)
     }
};

const StructInfo fmq::test::TestArrayStruct::_structInfo = {
    "test.TestArrayStruct", {
         {TYPE_ARRAY_STRUCT,  "test.TestStruct", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayStruct, value)
     }
};

const StructInfo fmq::test::TestArrayEnum::_structInfo = {
    "test.TestArrayEnum", {
         {TYPE_ARRAY_ENUM,  "test.Foo", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayEnum, value)
     }
};


