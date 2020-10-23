#include "testStructs.h"



test::Foo::Foo()
{
}
test::Foo::Foo(Enum en)
    : m_value(en)
{
}
test::Foo::operator const Enum&() const
{
    return m_value;
}
test::Foo::operator Enum&()
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
    "test.Foo", "description", {
        {"FOO_WORLD",   FOO_WORLD, "description"},
        {"FOO_HELLO",   FOO_HELLO, "description"},
        {"FOO_WORLD2",  FOO_WORLD2, "description"},
     }
};



test::TestBool::TestBool()
{
}
test::TestBool::TestBool(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestBool::clear()
{
    *this = test::TestBool();
}
const StructInfo& test::TestBool::getStructInfo() const
{
    return _structInfo;
}
bool test::TestBool::operator ==(const TestBool& rhs) const
{
    return (value == rhs.value);
}
bool test::TestBool::operator !=(const TestBool& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestBool::_structInfo = {
    "test.TestBool", "description", {
         {TYPE_BOOL,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestBool, value)}
     }
};

test::TestInt32::TestInt32()
{
}
test::TestInt32::TestInt32(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestInt32::clear()
{
    *this = test::TestInt32();
}
const StructInfo& test::TestInt32::getStructInfo() const
{
    return _structInfo;
}
bool test::TestInt32::operator ==(const TestInt32& rhs) const
{
    return (value == rhs.value);
}
bool test::TestInt32::operator !=(const TestInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestInt32::_structInfo = {
    "test.TestInt32", "description", {
         {TYPE_INT32,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestInt32, value)}
     }
};

test::TestUInt32::TestUInt32()
{
}
test::TestUInt32::TestUInt32(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestUInt32::clear()
{
    *this = test::TestUInt32();
}
const StructInfo& test::TestUInt32::getStructInfo() const
{
    return _structInfo;
}
bool test::TestUInt32::operator ==(const TestUInt32& rhs) const
{
    return (value == rhs.value);
}
bool test::TestUInt32::operator !=(const TestUInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestUInt32::_structInfo = {
    "test.TestUInt32", "description", {
         {TYPE_UINT32,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestUInt32, value)}
     }
};

test::TestInt64::TestInt64()
{
}
test::TestInt64::TestInt64(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestInt64::clear()
{
    *this = test::TestInt64();
}
const StructInfo& test::TestInt64::getStructInfo() const
{
    return _structInfo;
}
bool test::TestInt64::operator ==(const TestInt64& rhs) const
{
    return (value == rhs.value);
}
bool test::TestInt64::operator !=(const TestInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestInt64::_structInfo = {
    "test.TestInt64", "description", {
         {TYPE_INT64,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestInt64, value)}
     }
};

test::TestUInt64::TestUInt64()
{
}
test::TestUInt64::TestUInt64(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestUInt64::clear()
{
    *this = test::TestUInt64();
}
const StructInfo& test::TestUInt64::getStructInfo() const
{
    return _structInfo;
}
bool test::TestUInt64::operator ==(const TestUInt64& rhs) const
{
    return (value == rhs.value);
}
bool test::TestUInt64::operator !=(const TestUInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestUInt64::_structInfo = {
    "test.TestUInt64", "description", {
         {TYPE_UINT64,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestUInt64, value)}
     }
};

test::TestFloat::TestFloat()
{
}
test::TestFloat::TestFloat(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestFloat::clear()
{
    *this = test::TestFloat();
}
const StructInfo& test::TestFloat::getStructInfo() const
{
    return _structInfo;
}
bool test::TestFloat::operator ==(const TestFloat& rhs) const
{
    return (value == rhs.value);
}
bool test::TestFloat::operator !=(const TestFloat& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestFloat::_structInfo = {
    "test.TestFloat", "description", {
         {TYPE_FLOAT,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestFloat, value)}
     }
};

test::TestDouble::TestDouble()
{
}
test::TestDouble::TestDouble(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestDouble::clear()
{
    *this = test::TestDouble();
}
const StructInfo& test::TestDouble::getStructInfo() const
{
    return _structInfo;
}
bool test::TestDouble::operator ==(const TestDouble& rhs) const
{
    return (value == rhs.value);
}
bool test::TestDouble::operator !=(const TestDouble& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestDouble::_structInfo = {
    "test.TestDouble", "description", {
         {TYPE_DOUBLE,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestDouble, value)}
     }
};

test::TestString::TestString()
{
}
test::TestString::TestString(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestString::clear()
{
    *this = test::TestString();
}
const StructInfo& test::TestString::getStructInfo() const
{
    return _structInfo;
}
bool test::TestString::operator ==(const TestString& rhs) const
{
    return (value == rhs.value);
}
bool test::TestString::operator !=(const TestString& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestString::_structInfo = {
    "test.TestString", "description", {
         {TYPE_STRING,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestString, value)}
     }
};

test::TestBytes::TestBytes()
{
}
test::TestBytes::TestBytes(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestBytes::clear()
{
    *this = test::TestBytes();
}
const StructInfo& test::TestBytes::getStructInfo() const
{
    return _structInfo;
}
bool test::TestBytes::operator ==(const TestBytes& rhs) const
{
    return (value == rhs.value);
}
bool test::TestBytes::operator !=(const TestBytes& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestBytes::_structInfo = {
    "test.TestBytes", "description", {
         {TYPE_BYTES,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestBytes, value)}
     }
};

test::TestStruct::TestStruct()
{
}
test::TestStruct::TestStruct(const decltype(struct_int32)& struct_int32_, const decltype(struct_string)& struct_string_, const decltype(last_value)& last_value_)
    : struct_int32(struct_int32_)
    , struct_string(struct_string_)
    , last_value(last_value_)
{
}
void test::TestStruct::clear()
{
    *this = test::TestStruct();
}
const StructInfo& test::TestStruct::getStructInfo() const
{
    return _structInfo;
}
bool test::TestStruct::operator ==(const TestStruct& rhs) const
{
    return (struct_int32 == rhs.struct_int32 &&
            struct_string == rhs.struct_string &&
            last_value == rhs.last_value);
}
bool test::TestStruct::operator !=(const TestStruct& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestStruct::_structInfo = {
    "test.TestStruct", "description", {
         {TYPE_STRUCT,  "test.TestInt32",     "struct_int32", "description", 0},
         {TYPE_STRUCT,  "test.TestString",    "struct_string","description", 0},
         {TYPE_UINT32,  "",                   "last_value",   "description", 0}
     },{
        {OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStruct, struct_int32)},
        {OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStruct, struct_string)},
        {OFFSET_STRUCTBASE_TO_PARAM(TestStruct, last_value)}
     }
};



test::TestEnum::TestEnum()
{
}
test::TestEnum::TestEnum(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestEnum::clear()
{
    *this = test::TestEnum();
}
const StructInfo& test::TestEnum::getStructInfo() const
{
    return _structInfo;
}
bool test::TestEnum::operator ==(const TestEnum& rhs) const
{
    return (value == rhs.value);
}
bool test::TestEnum::operator !=(const TestEnum& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestEnum::_structInfo = {
    "test.TestEnum", "description", {
         {TYPE_ENUM,  "test.Foo", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestEnum, value.operator const Enum&())}
     }
};


test::TestArrayBool::TestArrayBool()
{
}
test::TestArrayBool::TestArrayBool(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayBool::clear()
{
    *this = test::TestArrayBool();
}
const StructInfo& test::TestArrayBool::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayBool::operator ==(const TestArrayBool& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayBool::operator !=(const TestArrayBool& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayBool::_structInfo = {
    "test.TestArrayBool", "description", {
         {TYPE_ARRAY_BOOL,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayBool, value)}
     }
};

test::TestArrayInt32::TestArrayInt32()
{
}
test::TestArrayInt32::TestArrayInt32(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayInt32::clear()
{
    *this = test::TestArrayInt32();
}
const StructInfo& test::TestArrayInt32::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayInt32::operator ==(const TestArrayInt32& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayInt32::operator !=(const TestArrayInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayInt32::_structInfo = {
    "test.TestArrayInt32", "description", {
         {TYPE_ARRAY_INT32,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayInt32, value)}
     }
};

test::TestArrayUInt32::TestArrayUInt32()
{
}
test::TestArrayUInt32::TestArrayUInt32(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayUInt32::clear()
{
    *this = test::TestArrayUInt32();
}
const StructInfo& test::TestArrayUInt32::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayUInt32::operator ==(const TestArrayUInt32& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayUInt32::operator !=(const TestArrayUInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayUInt32::_structInfo = {
    "test.TestArrayUInt32", "description", {
         {TYPE_ARRAY_UINT32,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayUInt32, value)}
     }
};

test::TestArrayInt64::TestArrayInt64()
{
}
test::TestArrayInt64::TestArrayInt64(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayInt64::clear()
{
    *this = test::TestArrayInt64();
}
const StructInfo& test::TestArrayInt64::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayInt64::operator ==(const TestArrayInt64& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayInt64::operator !=(const TestArrayInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayInt64::_structInfo = {
    "test.TestArrayInt64", "description", {
         {TYPE_ARRAY_INT64,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayInt64, value)}
     }
};

test::TestArrayUInt64::TestArrayUInt64()
{
}
test::TestArrayUInt64::TestArrayUInt64(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayUInt64::clear()
{
    *this = test::TestArrayUInt64();
}
const StructInfo& test::TestArrayUInt64::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayUInt64::operator ==(const TestArrayUInt64& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayUInt64::operator !=(const TestArrayUInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayUInt64::_structInfo = {
    "test.TestArrayUInt64", "description", {
         {TYPE_ARRAY_UINT64,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayUInt64, value)}
     }
};

test::TestArrayFloat::TestArrayFloat()
{
}
test::TestArrayFloat::TestArrayFloat(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayFloat::clear()
{
    *this = test::TestArrayFloat();
}
const StructInfo& test::TestArrayFloat::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayFloat::operator ==(const TestArrayFloat& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayFloat::operator !=(const TestArrayFloat& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayFloat::_structInfo = {
    "test.TestArrayFloat", "description", {
         {TYPE_ARRAY_FLOAT,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayFloat, value)}
     }
};

test::TestArrayDouble::TestArrayDouble()
{
}
test::TestArrayDouble::TestArrayDouble(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayDouble::clear()
{
    *this = test::TestArrayDouble();
}
const StructInfo& test::TestArrayDouble::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayDouble::operator ==(const TestArrayDouble& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayDouble::operator !=(const TestArrayDouble& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayDouble::_structInfo = {
    "test.TestArrayDouble", "description", {
         {TYPE_ARRAY_DOUBLE,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayDouble, value)}
     }
};

test::TestArrayString::TestArrayString()
{
}
test::TestArrayString::TestArrayString(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayString::clear()
{
    *this = test::TestArrayString();
}
const StructInfo& test::TestArrayString::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayString::operator ==(const TestArrayString& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayString::operator !=(const TestArrayString& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayString::_structInfo = {
    "test.TestArrayString", "description", {
         {TYPE_ARRAY_STRING,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayString, value)}
     }
};

test::TestArrayBytes::TestArrayBytes()
{
}
test::TestArrayBytes::TestArrayBytes(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayBytes::clear()
{
    *this = test::TestArrayBytes();
}
const StructInfo& test::TestArrayBytes::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayBytes::operator ==(const TestArrayBytes& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayBytes::operator !=(const TestArrayBytes& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayBytes::_structInfo = {
    "test.TestArrayBytes", "description", {
         {TYPE_ARRAY_BYTES,  "", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayBytes, value)}
     }
};

test::TestArrayStruct::TestArrayStruct()
{
}
test::TestArrayStruct::TestArrayStruct(const decltype(value)& value_, const decltype(last_value)& last_value_)
    : value(std::move(value_))
    , last_value(last_value_)
{
}
void test::TestArrayStruct::clear()
{
    *this = test::TestArrayStruct();
}
const StructInfo& test::TestArrayStruct::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayStruct::operator ==(const TestArrayStruct& rhs) const
{
    return (value == rhs.value &&
            last_value == rhs.last_value);
}
bool test::TestArrayStruct::operator !=(const TestArrayStruct& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayStruct::_structInfo = {
    "test.TestArrayStruct", "description", {
        {TYPE_ARRAY_STRUCT,  "test.TestStruct", "value", "description", 0, 0},
        {TYPE_UINT32,  "", "last_value", "description", 0, 0}
     },{
        {OFFSET_STRUCTBASE_TO_PARAM(TestArrayStruct, value), new ArrayStructAdapter<test::TestStruct>},
        {OFFSET_STRUCTBASE_TO_PARAM(TestArrayStruct, last_value)}
     }
};

test::TestArrayEnum::TestArrayEnum()
{
}
test::TestArrayEnum::TestArrayEnum(const decltype(value)& value_)
    : value(value_)
{
}
void test::TestArrayEnum::clear()
{
    *this = test::TestArrayEnum();
}
const StructInfo& test::TestArrayEnum::getStructInfo() const
{
    return _structInfo;
}
bool test::TestArrayEnum::operator ==(const TestArrayEnum& rhs) const
{
    return (value == rhs.value);
}
bool test::TestArrayEnum::operator !=(const TestArrayEnum& rhs) const
{
    return !(*this == rhs);
}
const StructInfo test::TestArrayEnum::_structInfo = {
    "test.TestArrayEnum", "description", {
         {TYPE_ARRAY_ENUM,  "test.Foo", "value", "description", 0, 0}
     },{
         {OFFSET_STRUCTBASE_TO_PARAM(TestArrayEnum, value)}
     }
};


