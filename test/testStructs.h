
#include "serializestruct/StructBase.h"


namespace test {
enum Foo
{
    FOO_WORLD = 0,
    FOO_HELLO = -2,
    FOO_WORLD2 = 1,
};
}

namespace test {
struct TestStructBool
{
    bool value = false;
};
}

namespace test {
struct TestStructInt32
{
    std::int32_t value = 0;
};
}

namespace test {
struct TestStructUInt32
{
    std::uint32_t value = 0;
};
}

namespace test {
struct TestStructInt64
{
    std::int64_t value = 0;
};
}

namespace test {
struct TestStructUInt64
{
    std::uint64_t value = 0;
};
}

namespace test {
struct TestStructFloat
{
    float value = 0;
};
}

namespace test {
struct TestStructDouble
{
    double value = 0;
};
}

namespace test {
struct TestStructString
{
    std::string value;
};
}

namespace test {
struct TestStructBytes
{
    Bytes value;
};
}

namespace test {
struct TestStructStruct
{
    TestStructInt32 struct_int32;
    TestStructString struct_string;
    std::int32_t last_value = 0;
};
}

namespace test {
struct TestStructStructBlockSize
{
    TestStructInt32 struct_int32;
    TestStructString struct_string;
    std::uint32_t last_value = 0;
};
}

namespace test {
struct TestStructEnum
{
    Foo value = FOO_WORLD;
};
}

namespace test {
struct TestStructArrayBool
{
    std::vector<bool> value;
};
}

namespace test {
struct TestStructArrayInt32
{
    std::vector<std::int32_t> value;
};
}

namespace test {
struct TestStructArrayUInt32
{
    std::vector<std::uint32_t> value;
};
}

namespace test {
struct TestStructArrayInt64
{
    std::vector<std::int64_t> value;
};
}

namespace test {
struct TestStructArrayUInt64
{
    std::vector<std::uint64_t> value;
};
}

namespace test {
struct TestStructArrayFloat
{
    std::vector<float> value;
};
}

namespace test {
struct TestStructArrayDouble
{
    std::vector<double> value;
};
}

namespace test {
struct TestStructArrayString
{
    std::vector<std::string> value;
};
}

namespace test {
struct TestStructArrayBytes
{
    std::vector<Bytes> value;
};
}

namespace test {
struct TestStructArrayStruct
{
    std::vector<TestStructStruct> value;
};
}

namespace test {
struct TestStructArrayEnum
{
    std::vector<Foo> value;
};
}
