#include "testStructs.h"



fmq::test::Foo::Foo()
{
}
fmq::test::Foo::Foo(Enum en)
    : m_value(en)
{
}
fmq::test::Foo::operator const Enum&() const
{
    return m_value;
}
fmq::test::Foo::operator Enum&()
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



fmq::test::TestBool::TestBool()
{
}
fmq::test::TestBool::TestBool(bool value_)
    : value(value_)
{
}
void fmq::test::TestBool::clear()
{
    *this = fmq::test::TestBool();
}
const StructInfo& fmq::test::TestBool::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestBool::operator ==(const TestBool& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestBool::operator !=(const TestBool& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestBool::_structInfo = {
    "test.TestBool", {
         {TYPE_BOOL,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestBool, value)
     }
};

fmq::test::TestInt32::TestInt32()
{
}
fmq::test::TestInt32::TestInt32(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestInt32::clear()
{
    *this = fmq::test::TestInt32();
}
const StructInfo& fmq::test::TestInt32::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestInt32::operator ==(const TestInt32& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestInt32::operator !=(const TestInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestInt32::_structInfo = {
    "test.TestInt32", {
         {TYPE_INT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestInt32, value)
     }
};

fmq::test::TestUInt32::TestUInt32()
{
}
fmq::test::TestUInt32::TestUInt32(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestUInt32::clear()
{
    *this = fmq::test::TestUInt32();
}
const StructInfo& fmq::test::TestUInt32::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestUInt32::operator ==(const TestUInt32& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestUInt32::operator !=(const TestUInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestUInt32::_structInfo = {
    "test.TestUInt32", {
         {TYPE_UINT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestUInt32, value)
     }
};

fmq::test::TestInt64::TestInt64()
{
}
fmq::test::TestInt64::TestInt64(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestInt64::clear()
{
    *this = fmq::test::TestInt64();
}
const StructInfo& fmq::test::TestInt64::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestInt64::operator ==(const TestInt64& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestInt64::operator !=(const TestInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestInt64::_structInfo = {
    "test.TestInt64", {
         {TYPE_INT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestInt64, value)
     }
};

fmq::test::TestUInt64::TestUInt64()
{
}
fmq::test::TestUInt64::TestUInt64(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestUInt64::clear()
{
    *this = fmq::test::TestUInt64();
}
const StructInfo& fmq::test::TestUInt64::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestUInt64::operator ==(const TestUInt64& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestUInt64::operator !=(const TestUInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestUInt64::_structInfo = {
    "test.TestUInt64", {
         {TYPE_UINT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestUInt64, value)
     }
};

fmq::test::TestFloat::TestFloat()
{
}
fmq::test::TestFloat::TestFloat(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestFloat::clear()
{
    *this = fmq::test::TestFloat();
}
const StructInfo& fmq::test::TestFloat::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestFloat::operator ==(const TestFloat& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestFloat::operator !=(const TestFloat& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestFloat::_structInfo = {
    "test.TestFloat", {
         {TYPE_FLOAT,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestFloat, value)
     }
};

fmq::test::TestDouble::TestDouble()
{
}
fmq::test::TestDouble::TestDouble(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestDouble::clear()
{
    *this = fmq::test::TestDouble();
}
const StructInfo& fmq::test::TestDouble::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestDouble::operator ==(const TestDouble& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestDouble::operator !=(const TestDouble& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestDouble::_structInfo = {
    "test.TestDouble", {
         {TYPE_DOUBLE,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestDouble, value)
     }
};

fmq::test::TestString::TestString()
{
}
fmq::test::TestString::TestString(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestString::clear()
{
    *this = fmq::test::TestString();
}
const StructInfo& fmq::test::TestString::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestString::operator ==(const TestString& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestString::operator !=(const TestString& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestString::_structInfo = {
    "test.TestString", {
         {TYPE_STRING,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestString, value)
     }
};

fmq::test::TestBytes::TestBytes()
{
}
fmq::test::TestBytes::TestBytes(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestBytes::clear()
{
    *this = fmq::test::TestBytes();
}
const StructInfo& fmq::test::TestBytes::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestBytes::operator ==(const TestBytes& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestBytes::operator !=(const TestBytes& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestBytes::_structInfo = {
    "test.TestBytes", {
         {TYPE_BYTES,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestBytes, value)
     }
};

fmq::test::TestStruct::TestStruct()
{
}
fmq::test::TestStruct::TestStruct(decltype(struct_int32) struct_int32_, decltype(struct_string) struct_string_, decltype(last_value) last_value_)
    : struct_int32(struct_int32_)
    , struct_string(struct_string_)
    , last_value(last_value_)
{
}
void fmq::test::TestStruct::clear()
{
    *this = fmq::test::TestStruct();
}
const StructInfo& fmq::test::TestStruct::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestStruct::operator ==(const TestStruct& rhs) const
{
    return (struct_int32 == rhs.struct_int32 &&
            struct_string == rhs.struct_string &&
            last_value == rhs.last_value);
}
bool fmq::test::TestStruct::operator !=(const TestStruct& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestStruct::_structInfo = {
    "test.TestStruct", {
         {TYPE_STRUCT,  "test.TestInt32",     "struct_int32", "description", 0, 0},
         {TYPE_STRUCT,  "test.TestString",    "struct_string","description", 1, 0},
         {TYPE_UINT32,  "",                   "last_value",   "description", 2, 0}
     },{
        OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStruct, struct_int32),
        OFFSET_STRUCTBASE_TO_STRUCTBASE(TestStruct, struct_string),
        OFFSET_STRUCTBASE_TO_PARAM(TestStruct, last_value)
     }
};



fmq::test::TestEnum::TestEnum()
{
}
fmq::test::TestEnum::TestEnum(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestEnum::clear()
{
    *this = fmq::test::TestEnum();
}
const StructInfo& fmq::test::TestEnum::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestEnum::operator ==(const TestEnum& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestEnum::operator !=(const TestEnum& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestEnum::_structInfo = {
    "test.TestEnum", {
         {TYPE_ENUM,  "test.Foo", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestEnum, value.operator const Enum&())
     }
};


fmq::test::TestArrayBool::TestArrayBool()
{
}
fmq::test::TestArrayBool::TestArrayBool(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayBool::clear()
{
    *this = fmq::test::TestArrayBool();
}
const StructInfo& fmq::test::TestArrayBool::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayBool::operator ==(const TestArrayBool& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayBool::operator !=(const TestArrayBool& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayBool::_structInfo = {
    "test.TestArrayBool", {
         {TYPE_ARRAY_BOOL,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayBool, value)
     }
};

fmq::test::TestArrayInt32::TestArrayInt32()
{
}
fmq::test::TestArrayInt32::TestArrayInt32(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayInt32::clear()
{
    *this = fmq::test::TestArrayInt32();
}
const StructInfo& fmq::test::TestArrayInt32::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayInt32::operator ==(const TestArrayInt32& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayInt32::operator !=(const TestArrayInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayInt32::_structInfo = {
    "test.TestArrayInt32", {
         {TYPE_ARRAY_INT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayInt32, value)
     }
};

fmq::test::TestArrayUInt32::TestArrayUInt32()
{
}
fmq::test::TestArrayUInt32::TestArrayUInt32(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayUInt32::clear()
{
    *this = fmq::test::TestArrayUInt32();
}
const StructInfo& fmq::test::TestArrayUInt32::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayUInt32::operator ==(const TestArrayUInt32& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayUInt32::operator !=(const TestArrayUInt32& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayUInt32::_structInfo = {
    "test.TestArrayUInt32", {
         {TYPE_ARRAY_UINT32,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayUInt32, value)
     }
};

fmq::test::TestArrayInt64::TestArrayInt64()
{
}
fmq::test::TestArrayInt64::TestArrayInt64(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayInt64::clear()
{
    *this = fmq::test::TestArrayInt64();
}
const StructInfo& fmq::test::TestArrayInt64::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayInt64::operator ==(const TestArrayInt64& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayInt64::operator !=(const TestArrayInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayInt64::_structInfo = {
    "test.TestArrayInt64", {
         {TYPE_ARRAY_INT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayInt64, value)
     }
};

fmq::test::TestArrayUInt64::TestArrayUInt64()
{
}
fmq::test::TestArrayUInt64::TestArrayUInt64(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayUInt64::clear()
{
    *this = fmq::test::TestArrayUInt64();
}
const StructInfo& fmq::test::TestArrayUInt64::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayUInt64::operator ==(const TestArrayUInt64& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayUInt64::operator !=(const TestArrayUInt64& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayUInt64::_structInfo = {
    "test.TestArrayUInt64", {
         {TYPE_ARRAY_UINT64,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayUInt64, value)
     }
};

fmq::test::TestArrayFloat::TestArrayFloat()
{
}
fmq::test::TestArrayFloat::TestArrayFloat(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayFloat::clear()
{
    *this = fmq::test::TestArrayFloat();
}
const StructInfo& fmq::test::TestArrayFloat::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayFloat::operator ==(const TestArrayFloat& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayFloat::operator !=(const TestArrayFloat& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayFloat::_structInfo = {
    "test.TestArrayFloat", {
         {TYPE_ARRAY_FLOAT,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayFloat, value)
     }
};

fmq::test::TestArrayDouble::TestArrayDouble()
{
}
fmq::test::TestArrayDouble::TestArrayDouble(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayDouble::clear()
{
    *this = fmq::test::TestArrayDouble();
}
const StructInfo& fmq::test::TestArrayDouble::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayDouble::operator ==(const TestArrayDouble& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayDouble::operator !=(const TestArrayDouble& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayDouble::_structInfo = {
    "test.TestArrayDouble", {
         {TYPE_ARRAY_DOUBLE,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayDouble, value)
     }
};

fmq::test::TestArrayString::TestArrayString()
{
}
fmq::test::TestArrayString::TestArrayString(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayString::clear()
{
    *this = fmq::test::TestArrayString();
}
const StructInfo& fmq::test::TestArrayString::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayString::operator ==(const TestArrayString& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayString::operator !=(const TestArrayString& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayString::_structInfo = {
    "test.TestArrayString", {
         {TYPE_ARRAY_STRING,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayString, value)
     }
};

fmq::test::TestArrayBytes::TestArrayBytes()
{
}
fmq::test::TestArrayBytes::TestArrayBytes(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayBytes::clear()
{
    *this = fmq::test::TestArrayBytes();
}
const StructInfo& fmq::test::TestArrayBytes::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayBytes::operator ==(const TestArrayBytes& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayBytes::operator !=(const TestArrayBytes& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayBytes::_structInfo = {
    "test.TestArrayBytes", {
         {TYPE_ARRAY_BYTES,  "", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayBytes, value)
     }
};

fmq::test::TestArrayStruct::TestArrayStruct()
{
}
fmq::test::TestArrayStruct::TestArrayStruct(decltype(value) value_)
    : value(std::move(value_))
{
}
void fmq::test::TestArrayStruct::clear()
{
    *this = fmq::test::TestArrayStruct();
}
StructBase* fmq::test::TestArrayStruct::add(int index)
{
    StructBase* structBase = nullptr;
    switch (index)
    {
    case 0:
        value.resize(value.size() + 1);
        structBase = &value.back();
        break;
    default:
        break;
    }
    return structBase;
}
const StructInfo& fmq::test::TestArrayStruct::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayStruct::operator ==(const TestArrayStruct& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayStruct::operator !=(const TestArrayStruct& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayStruct::_structInfo = {
    "test.TestArrayStruct", {
         {TYPE_ARRAY_STRUCT,  "test.TestStruct", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayStruct, value)
     }
};

fmq::test::TestArrayEnum::TestArrayEnum()
{
}
fmq::test::TestArrayEnum::TestArrayEnum(decltype(value) value_)
    : value(value_)
{
}
void fmq::test::TestArrayEnum::clear()
{
    *this = fmq::test::TestArrayEnum();
}
const StructInfo& fmq::test::TestArrayEnum::getStructInfo() const
{
    return _structInfo;
}
bool fmq::test::TestArrayEnum::operator ==(const TestArrayEnum& rhs) const
{
    return (value == rhs.value);
}
bool fmq::test::TestArrayEnum::operator !=(const TestArrayEnum& rhs) const
{
    return !(*this == rhs);
}
const StructInfo fmq::test::TestArrayEnum::_structInfo = {
    "test.TestArrayEnum", {
         {TYPE_ARRAY_ENUM,  "test.Foo", "value", "description", 0, 0}
     },{
         OFFSET_STRUCTBASE_TO_PARAM(TestArrayEnum, value)
     }
};


