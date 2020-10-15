
#include "serializestruct/StructBase.h"


namespace test {
class Foo
{
public:
    enum Enum {
        FOO_WORLD = 0,
        FOO_HELLO = -2,
        FOO_WORLD2 = 1,
    };

    Foo();
    Foo(Enum en);
    operator Enum() const;
    const Foo& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = FOO_WORLD;
    static const EnumInfo _enumInfo;
};
}


namespace test {
struct TestStructBool : public StructBase
{
    bool value = false;

private:
    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructInt32
{
    std::int32_t value = 0;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructUInt32
{
    std::uint32_t value = 0;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructInt64
{
    std::int64_t value = 0;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructUInt64
{
    std::uint64_t value = 0;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructFloat
{
    float value = 0;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructDouble
{
    double value = 0;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructString
{
    std::string value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructBytes
{
    Bytes value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructStruct
{
    TestStructInt32 struct_int32;
    TestStructString struct_string;
    std::int32_t last_value = 0;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructEnum
{
    Foo::Enum value = Foo::FOO_WORLD;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayBool
{
    std::vector<bool> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayInt32
{
    std::vector<std::int32_t> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayUInt32
{
    std::vector<std::uint32_t> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayInt64
{
    std::vector<std::int64_t> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayUInt64
{
    std::vector<std::uint64_t> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayFloat
{
    std::vector<float> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayDouble
{
    std::vector<double> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayString
{
    std::vector<std::string> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayBytes
{
    std::vector<Bytes> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayStruct
{
    std::vector<TestStructStruct> value;

    static const StructInfo _structInfo;
};
}

namespace test {
struct TestStructArrayEnum
{
    std::vector<Foo> value;

    static const StructInfo _structInfo;
};
}
