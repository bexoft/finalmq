//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "finalmq/variant/Variant.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/conversions/Conversions.h"


//using ::testing::_;
//using ::testing::Return;
//using ::testing::ReturnRef;
//using testing::Invoke;
//using testing::DoAll;


using namespace finalmq;


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


TEST_F(TestVariant, testFloat)
{
    float VALUE = 1234.5;

    Variant variant = VALUE;
    float val = variant;
    ASSERT_EQ(val, VALUE);
    float* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    std::string sval = variant;
    string2Number(sval, val);
    ASSERT_EQ(val, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    std::int32_t ival = variant;
    ASSERT_EQ(ival, 1234);
    pval = variant.getData<float>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testDouble)
{
    double VALUE = 1234.5;

    Variant variant = VALUE;
    double val = variant;
    ASSERT_EQ(val, VALUE);
    double* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    std::string sval = variant;
    string2Number(sval, val);
    ASSERT_EQ(val, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    std::int32_t ival = variant;
    ASSERT_EQ(ival, 1234);
    pval = variant.getData<double>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testString)
{
    std::string VALUE = "1234.5";

    Variant variant = VALUE;
    std::string val = variant;
    ASSERT_EQ(val, VALUE);
    std::string* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);

    bool bval = variant;
    ASSERT_EQ(bval, true);
    std::int32_t ival = variant;
    ASSERT_EQ(ival, 1234);
    std::uint32_t uival = variant;
    ASSERT_EQ(uival, 1234);
    std::int64_t llval = variant;
    ASSERT_EQ(llval, 1234);
    std::uint64_t ullval = variant;
    ASSERT_EQ(ullval, 1234);
    float fval = variant;
    ASSERT_EQ(fval, 1234.5);
    double dval = variant;
    ASSERT_EQ(dval, 1234.5);
    Bytes bytesval = variant;
    ASSERT_EQ(bytesval.empty(), true);
    std::vector<std::int32_t> aival = variant;
    ASSERT_EQ(aival.empty(), true);

    std::string* psval = variant.getData<std::string>("");
    ASSERT_NE(psval, nullptr);
    ASSERT_EQ(*psval, VALUE);
    double* pdval = variant.getData<double>("");
    ASSERT_EQ(pdval, nullptr);
}



TEST_F(TestVariant, testBytes)
{
    Bytes VALUE = {1, 2, 3, 4};

    Variant variant = VALUE;
    Bytes val = variant;
    ASSERT_EQ(val, VALUE);
    Bytes* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);

    bool bval = variant;
    ASSERT_EQ(bval, false);
    std::int32_t ival = variant;
    ASSERT_EQ(ival, 0);

    Bytes* pbytesval = variant.getData<Bytes>("");
    ASSERT_NE(pbytesval, nullptr);
    ASSERT_EQ(*pbytesval, VALUE);
    double* pdval = variant.getData<double>("");
    ASSERT_EQ(pdval, nullptr);
}

TEST_F(TestVariant, testArrayBool)
{
    std::vector<bool> VALUE = {true, false, true};

    Variant variant = VALUE;
    std::vector<bool> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<bool>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<bool>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayInt32)
{
    std::vector<std::int32_t> VALUE = {1, 2, 3, 4};

    Variant variant = VALUE;
    std::vector<std::int32_t> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<std::int32_t>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<std::int32_t>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayUInt32)
{
    std::vector<std::uint32_t> VALUE = {1, 2, 3, 4};

    Variant variant = VALUE;
    std::vector<std::uint32_t> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<std::uint32_t>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<std::uint32_t>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayInt64)
{
    std::vector<std::int64_t> VALUE = {1, 2, 3, 4};

    Variant variant = VALUE;
    std::vector<std::int64_t> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<std::int64_t>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<std::int64_t>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayUInt64)
{
    std::vector<std::uint64_t> VALUE = {1, 2, 3, 4};

    Variant variant = VALUE;
    std::vector<std::uint64_t> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<std::uint64_t>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<std::uint64_t>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayFloat)
{
    std::vector<float> VALUE = {1, 2, 3, 4};

    Variant variant = VALUE;
    std::vector<float> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<float>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<float>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayDouble)
{
    std::vector<double> VALUE = {1, 2, 3, 4};

    Variant variant = VALUE;
    std::vector<double> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<double>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<double>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayString)
{
    std::vector<std::string> VALUE = {"123", "234", "345", "456"};

    Variant variant = VALUE;
    std::vector<std::string> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<std::string>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<std::string>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testArrayBytes)
{
    std::vector<Bytes> VALUE = {{1,2,3}, {2,3,4}, {3,4,5}, {4,5,6}};

    Variant variant = VALUE;
    std::vector<Bytes> val = variant;
    ASSERT_EQ(val, VALUE);
    std::vector<Bytes>* pval = variant;
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
    Variant* pvariant = variant.getVariant("");
    ASSERT_EQ(pvariant, &variant);
    pvariant = variant.getVariant("abc");
    ASSERT_EQ(pvariant, nullptr);
    pval = variant.getData<std::vector<Bytes>>("");
    ASSERT_NE(pval, nullptr);
    ASSERT_EQ(*pval, VALUE);
}

TEST_F(TestVariant, testStruct)
{
    Variant variant = VariantStruct();
    variant.add("hello", -123);
    variant.add("world", std::string("200.5"));

    VariantStruct* pval = variant;
    ASSERT_NE(pval, nullptr);
    VariantStruct s = {{"hello", -123},{"world", "200.5"}};
    ASSERT_EQ(*pval == s, true);

    const Variant* var = variant.getVariant("hello");
    ASSERT_NE(var, nullptr);
    ASSERT_EQ(*var == s[0].second, true);

    double v = variant.getDataValue<double>("hello");
    ASSERT_EQ(v, -123);

    v = variant.getDataValue<double>("abc");
    ASSERT_EQ(v, 0);
}


TEST_F(TestVariant, testStructSub)
{
    Variant variant = VariantStruct();
    variant.add("hello", -123);
    variant.add("world", "200.5");
    variant.add("sub", VariantStruct());

    Variant* sub = variant.getVariant("sub");
    ASSERT_NE(sub, nullptr);

    sub->add("a", 100);
    sub->add("b", 200);

    VariantStruct s = {{"hello", -123},{"world", "200.5"},{"sub", VariantStruct({{"a", 100}, {"b", 200}})}};
    ASSERT_EQ(variant == s, true);

    const Variant* var = variant.getVariant("sub.b");
    ASSERT_NE(var, nullptr);
    ASSERT_EQ(*var == *sub->getVariant("b"), true);

    double v = variant.getDataValue<double>("sub.b");
    ASSERT_EQ(v, 200);

    var = variant.getVariant("hello.b");
    ASSERT_EQ(var, nullptr);
}



TEST_F(TestVariant, testList)
{
    Variant variant = VariantList();
    variant.add(-123);
    variant.add("200.5");

    VariantList* pval = variant;
    ASSERT_NE(pval, nullptr);
    VariantList l = {-123, "200.5"};
    ASSERT_EQ(*pval == l, true);

    const Variant* var = variant.getVariant("0");
    ASSERT_NE(var, nullptr);
    ASSERT_EQ(*var == l[0], true);

    double v = variant.getDataValue<double>("0");
    ASSERT_EQ(v, -123);

    var = variant.getVariant("7");
    ASSERT_EQ(var, nullptr);

    v = variant.getDataValue<double>("7");
    ASSERT_EQ(v, 0);
}


TEST_F(TestVariant, testListSub)
{
    Variant variant = VariantList();
    variant.add(-123);
    variant.add("200.5");
    variant.add(VariantStruct());

    Variant* sub = variant.getVariant("2");
    ASSERT_NE(sub, nullptr);

    sub->add("a", 100);
    sub->add("b", 200);

    VariantList l = {-123,"200.5",VariantStruct({{"a", 100}, {"b", 200}})};
    ASSERT_EQ(variant == l, true);

    const Variant* var = variant.getVariant("2.b");
    ASSERT_NE(var, nullptr);
    ASSERT_EQ(*var == *sub->getVariant("b"), true);

    double v = variant.getDataValue<double>("2.b");
    ASSERT_EQ(v, 200);

    var = variant.getVariant("0.b");
    ASSERT_EQ(var, nullptr);
}

