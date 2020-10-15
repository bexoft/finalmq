
#include "serializestruct/StructBase.h"


namespace fmq {
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
}}


namespace fmq {
namespace test {
struct TestBool : public StructBase
{
    bool value = false;

private:
    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestInt32
{
    std::int32_t value = 0;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestUInt32
{
    std::uint32_t value = 0;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestInt64
{
    std::int64_t value = 0;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestUInt64
{
    std::uint64_t value = 0;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestFloat
{
    float value = 0;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestDouble
{
    double value = 0;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestString
{
    std::string value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestBytes
{
    Bytes value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestStruct
{
    TestInt32 struct_int32;
    TestString struct_string;
    std::int32_t last_value = 0;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestEnum
{
    Foo::Enum value = Foo::FOO_WORLD;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayBool
{
    std::vector<bool> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayInt32
{
    std::vector<std::int32_t> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayUInt32
{
    std::vector<std::uint32_t> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayInt64
{
    std::vector<std::int64_t> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayUInt64
{
    std::vector<std::uint64_t> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayFloat
{
    std::vector<float> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayDouble
{
    std::vector<double> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayString
{
    std::vector<std::string> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayBytes
{
    std::vector<Bytes> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayStruct
{
    std::vector<TestStruct> value;

    static const StructInfo _structInfo;
};
}}

namespace fmq {
namespace test {
struct TestArrayEnum
{
    std::vector<Foo> value;

    static const StructInfo _structInfo;
};
}}
