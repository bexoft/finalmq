
#include "serializestruct/StructBase.h"



namespace test {
class Foo
{
public:
    enum Enum : std::int32_t {
        FOO_WORLD = 0,
        FOO_HELLO = -2,
        FOO_WORLD2 = 1,
    };

    Foo();
    Foo(Enum en);
    operator const Enum&() const;
    operator Enum&();
    const Foo& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = Enum::FOO_WORLD;
    static const EnumInfo _enumInfo;
};
}



namespace test {
struct TestBool : public StructBase
{
    bool value = false;

    TestBool();
    TestBool(const decltype(value)& value_);
    bool operator ==(const TestBool& rhs) const;
    bool operator !=(const TestBool& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestInt32 : public StructBase
{
    std::int32_t value = 0;

    TestInt32();
    TestInt32(const decltype(value)& value_);
    bool operator ==(const TestInt32& rhs) const;
    bool operator !=(const TestInt32& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestUInt32 : public StructBase
{
    std::uint32_t value = 0;

    TestUInt32();
    TestUInt32(const decltype(value)& value_);
    bool operator ==(const TestUInt32& rhs) const;
    bool operator !=(const TestUInt32& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestInt64 : public StructBase
{
    std::int64_t value = 0;

    TestInt64();
    TestInt64(const decltype(value)& value_);
    bool operator ==(const TestInt64& rhs) const;
    bool operator !=(const TestInt64& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestUInt64 : public StructBase
{
    std::uint64_t value = 0;

    TestUInt64();
    TestUInt64(const decltype(value)& value_);
    bool operator ==(const TestUInt64& rhs) const;
    bool operator !=(const TestUInt64& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestFloat : public StructBase
{
    float value = 0;

    TestFloat();
    TestFloat(const decltype(value)& value_);
    bool operator ==(const TestFloat& rhs) const;
    bool operator !=(const TestFloat& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestDouble : public StructBase
{
    double value = 0;

    TestDouble();
    TestDouble(const decltype(value)& value_);
    bool operator ==(const TestDouble& rhs) const;
    bool operator !=(const TestDouble& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestString : public StructBase
{
    std::string value;

    TestString();
    TestString(const decltype(value)& value_);
    bool operator ==(const TestString& rhs) const;
    bool operator !=(const TestString& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestBytes : public StructBase
{
    Bytes value;

    TestBytes();
    TestBytes(const decltype(value)& value_);
    bool operator ==(const TestBytes& rhs) const;
    bool operator !=(const TestBytes& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestStruct : public StructBase
{
    TestInt32 struct_int32;
    TestString struct_string;
    std::int32_t last_value = 0;

    TestStruct();
    TestStruct(const decltype(struct_int32)& struct_int32_, const decltype(struct_string)& struct_string_, const decltype(last_value)& last_value_);
    bool operator ==(const TestStruct& rhs) const;
    bool operator !=(const TestStruct& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestEnum : public StructBase
{
    Foo value = Foo::FOO_WORLD;

    TestEnum();
    TestEnum(const decltype(value)& value_);
    bool operator ==(const TestEnum& rhs) const;
    bool operator !=(const TestEnum& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayBool : public StructBase
{
    std::vector<bool> value;

    TestArrayBool();
    TestArrayBool(const decltype(value)& value_);
    bool operator ==(const TestArrayBool& rhs) const;
    bool operator !=(const TestArrayBool& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayInt32 : public StructBase
{
    std::vector<std::int32_t> value;

    TestArrayInt32();
    TestArrayInt32(const decltype(value)& value_);
    bool operator ==(const TestArrayInt32& rhs) const;
    bool operator !=(const TestArrayInt32& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayUInt32 : public StructBase
{
    std::vector<std::uint32_t> value;

    TestArrayUInt32();
    TestArrayUInt32(const decltype(value)& value_);
    bool operator ==(const TestArrayUInt32& rhs) const;
    bool operator !=(const TestArrayUInt32& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayInt64 : public StructBase
{
    std::vector<std::int64_t> value;

    TestArrayInt64();
    TestArrayInt64(const decltype(value)& value_);
    bool operator ==(const TestArrayInt64& rhs) const;
    bool operator !=(const TestArrayInt64& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayUInt64 : public StructBase
{
    std::vector<std::uint64_t> value;

    TestArrayUInt64();
    TestArrayUInt64(const decltype(value)& value_);
    bool operator ==(const TestArrayUInt64& rhs) const;
    bool operator !=(const TestArrayUInt64& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayFloat : public StructBase
{
    std::vector<float> value;

    TestArrayFloat();
    TestArrayFloat(const decltype(value)& value_);
    bool operator ==(const TestArrayFloat& rhs) const;
    bool operator !=(const TestArrayFloat& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayDouble : public StructBase
{
    std::vector<double> value;

    TestArrayDouble();
    TestArrayDouble(const decltype(value)& value_);
    bool operator ==(const TestArrayDouble& rhs) const;
    bool operator !=(const TestArrayDouble& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayString : public StructBase
{
    std::vector<std::string> value;

    TestArrayString();
    TestArrayString(const decltype(value)& value_);
    bool operator ==(const TestArrayString& rhs) const;
    bool operator !=(const TestArrayString& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayBytes : public StructBase
{
    std::vector<Bytes> value;

    TestArrayBytes();
    TestArrayBytes(const decltype(value)& value_);
    bool operator ==(const TestArrayBytes& rhs) const;
    bool operator !=(const TestArrayBytes& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayStruct : public StructBase
{
    std::vector<TestStruct> value;

    TestArrayStruct();
    TestArrayStruct(const decltype(value)& value_);
    bool operator ==(const TestArrayStruct& rhs) const;
    bool operator !=(const TestArrayStruct& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}


namespace test {
struct TestArrayEnum : public StructBase
{
    std::vector<Foo> value;

    TestArrayEnum();
    TestArrayEnum(const decltype(value)& value_);
    bool operator ==(const TestArrayEnum& rhs) const;
    bool operator !=(const TestArrayEnum& rhs) const;

private:
    virtual void clear() override;
    virtual const StructInfo& getStructInfo() const override;
    static const StructInfo _structInfo;
};
}
