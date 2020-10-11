#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "variant/Variant.h"
#include "variant/VariantValues.h"


//using ::testing::_;
//using ::testing::Return;
//using ::testing::ReturnRef;
//using testing::Invoke;
//using testing::DoAll;




class TestVariant : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};




TEST_F(TestVariant, testBool)
{
    Variant variant = true;
    bool val = variant;
    ASSERT_EQ(val, true);
    bool* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, true);
    std::string sval = variant;
    ASSERT_EQ(sval, "true");
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    std::int32_t ival = variant;
    ASSERT_EQ(ival, 1);
    pval = variant.getData<bool>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, true);
}

TEST_F(TestVariant, testInt32)
{
    std::int32_t VALUE = 1234;

    Variant variant = VALUE;
    std::int32_t val = variant;
    ASSERT_EQ(val, VALUE);
    std::int32_t* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    std::string sval = variant;
    ASSERT_EQ(sval, "1234");
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    std::int64_t ival = variant;
    ASSERT_EQ(ival, VALUE);
    pval = variant.getData<std::int32_t>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testUInt32)
{
    std::uint32_t VALUE = 1234;

    Variant variant = VALUE;
    std::uint32_t val = variant;
    ASSERT_EQ(val, VALUE);
    std::uint32_t* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    std::string sval = variant;
    ASSERT_EQ(sval, "1234");
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    std::int64_t ival = variant;
    ASSERT_EQ(ival, VALUE);
    pval = variant.getData<std::uint32_t>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testInt64)
{
    std::int64_t VALUE = 1234;

    Variant variant = VALUE;
    std::int64_t val = variant;
    ASSERT_EQ(val, VALUE);
    std::int64_t* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    std::string sval = variant;
    ASSERT_EQ(sval, "1234");
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    std::int32_t ival = variant;
    ASSERT_EQ(ival, VALUE);
    pval = variant.getData<std::int64_t>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testUInt64)
{
    std::uint64_t VALUE = 1234;

    Variant variant = VALUE;
    std::uint64_t val = variant;
    ASSERT_EQ(val, VALUE);
    std::uint64_t* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    std::string sval = variant;
    ASSERT_EQ(sval, "1234");
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    std::int32_t ival = variant;
    ASSERT_EQ(ival, VALUE);
    pval = variant.getData<std::uint64_t>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

