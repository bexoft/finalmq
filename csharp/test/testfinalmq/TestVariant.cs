using System;
using System.Runtime;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;

using Xunit;
using Moq;

using finalmq;
using System.Text;


namespace testfinalmq
{
    using VariantStruct = List<NameValue>;
    using VariantList = List<Variant>;

    public class TestVariant : IDisposable
    {

        public TestVariant()
        {
        }

        public void Dispose()
        {
        }


        [Fact]
        public void TestBool()
        {
            Variant variant = Variant.Create(true);
            bool val = variant;
            Debug.Assert(val == true);
            string sval = variant;
            Debug.Assert(sval == "true");
            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);
            int ival = variant;
            Debug.Assert(ival == 1);
            val = variant;
            Debug.Assert(val == true);

            variant.SetData(false);
            val = variant;
            Debug.Assert(val == false);
            variant.SetData(true);
            val = variant;
            Debug.Assert(val == true);
        }

        [Fact]
        public void TestInt32()
        {
            int VALUE = 1234;
            Variant variant = Variant.Create(VALUE);
            int val = variant;
            Debug.Assert(val == VALUE);
            string sval = variant;
            Debug.Assert(sval == "1234");
            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);
            long lval = variant;
            Debug.Assert(lval == VALUE);

            variant.SetData(5555);
            val = variant;
            Debug.Assert(val == 5555);
        }

        [Fact]
        public void TestUInt32()
        {
            uint VALUE = 1234;
            Variant variant = Variant.Create(VALUE);
            uint val = variant;
            Debug.Assert(val == VALUE);
            string sval = variant;
            Debug.Assert(sval == "1234");
            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);
            long lval = variant;
            Debug.Assert(lval == VALUE);

            variant.SetData(5555);
            val = variant;
            Debug.Assert(val == 5555);
        }

        [Fact]
        public void TestInt64()
        {
            long VALUE = 1234;
            Variant variant = Variant.Create(VALUE);
            long val = variant;
            Debug.Assert(val == VALUE);
            string sval = variant;
            Debug.Assert(sval == "1234");
            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);
            int lval = variant;
            Debug.Assert(lval == VALUE);

            variant.SetData(5555);
            val = variant;
            Debug.Assert(val == 5555);
        }

        [Fact]
        public void TestUInt64()
        {
            ulong VALUE = 1234;
            Variant variant = Variant.Create(VALUE);
            ulong val = variant;
            Debug.Assert(val == VALUE);
            string sval = variant;
            Debug.Assert(sval == "1234");
            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);
            uint lval = variant;
            Debug.Assert(lval == VALUE);

            variant.SetData(5555);
            val = variant;
            Debug.Assert(val == 5555);
        }

        [Fact]
        public void TestFloat()
        {
            float VALUE = 1234.5f;
            Variant variant = Variant.Create(VALUE);
            float val = variant;
            Debug.Assert(val == VALUE);
            string sval = variant;
            Debug.Assert(sval == "1234.5");
            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);
            int lval = variant;
            Debug.Assert(lval == 1234);

            variant.SetData(5555.3f);
            val = variant;
            Debug.Assert(val == 5555.3f);
        }

        [Fact]
        public void TestDouble()
        {
            double VALUE = 1234.5;
            Variant variant = Variant.Create(VALUE);
            double val = variant;
            Debug.Assert(val == VALUE);
            string sval = variant;
            Debug.Assert(sval == "1234.5");
            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);
            int lval = variant;
            Debug.Assert(lval == 1234);

            variant.SetData(5555.3);
            val = variant;
            Debug.Assert(val == 5555.3);
        }

        [Fact]
        public void TestString()
        {
            string VALUE = "1234.5";
            Variant variant = Variant.Create(VALUE);
            string val = variant;
            Debug.Assert(val == VALUE);

            Variant? pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);

            bool bval = variant;
            Debug.Assert(bval == true);
            int ival = variant;
            Debug.Assert(ival == 1234);
            uint uival = variant;
            Debug.Assert(uival == 1234);
            long lval = variant;
            Debug.Assert(lval == 1234);
            ulong ulval = variant;
            Debug.Assert(ulval == 1234);
            float fval = variant;
            Debug.Assert(fval == 1234.5);
            double dval = variant;
            Debug.Assert(dval == 1234.5);

            variant.SetData("Hello");
            val = variant;
            Debug.Assert(val == "Hello");
        }

        [Fact]
        public void TestBytes()
        {
            byte[] VALUE = { 1, 2, 3, 4 };

            Variant variant = Variant.Create(VALUE);
            byte[] val = variant;
            Debug.Assert(val == VALUE);
            Variant pvariant = variant.GetVariant("abc");
            Debug.Assert(pvariant == null);

            bool bval = variant;
            Debug.Assert(bval == false);
            int ival = variant;
            Debug.Assert(ival == 0);

            byte[] VALUE2 = { 4, 3, 6, 2, 3, 5 };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayBool()
        {
            bool[] VALUE = { true, false, true };
            Variant variant = Variant.Create(VALUE);
            bool[] val = variant;
            Debug.Assert(val == VALUE);

            bool[] VALUE2 = { false, false, false, true };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayInt32()
        {
            int[] VALUE = { 1, 2, 3, 4 };
            Variant variant = Variant.Create(VALUE);
            int[] val = variant;
            Debug.Assert(val == VALUE);

            int[] VALUE2 = { 4, 3, 6, 2, 3, 5 };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayUInt32()
        {
            uint[] VALUE = { 1, 2, 3, 4 };
            Variant variant = Variant.Create(VALUE);
            uint[] val = variant;
            Debug.Assert(val == VALUE);

            uint[] VALUE2 = { 4, 3, 6, 2, 3, 5 };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayInt54()
        {
            long[] VALUE = { 1, 2, 3, 4 };
            Variant variant = Variant.Create(VALUE);
            long[] val = variant;
            Debug.Assert(val == VALUE);

            long[] VALUE2 = { 4, 3, 6, 2, 3, 5 };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayUInt64()
        {
            ulong[] VALUE = { 1, 2, 3, 4 };
            Variant variant = Variant.Create(VALUE);
            ulong[] val = variant;
            Debug.Assert(val == VALUE);

            ulong[] VALUE2 = { 4, 3, 6, 2, 3, 5 };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayFloat()
        {
            float[] VALUE = { 1.1f, 2.2f, 3.3f, 4.4f };
            Variant variant = Variant.Create(VALUE);
            float[] val = variant;
            Debug.Assert(val == VALUE);

            float[] VALUE2 = { 4.3f, 3.5f, 6.1f, 2.0f, 3.8f, 5.9f };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayDouble()
        {
            double[] VALUE = { 1.1, 2.2, 3.3, 4.4 };
            Variant variant = Variant.Create(VALUE);
            double[] val = variant;
            Debug.Assert(val == VALUE);

            double[] VALUE2 = { 4.3, 3.5, 6.1, 2.0, 3.8, 5.9 };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayString()
        {
            string[] VALUE = { "Hello", "", "World" };
            Variant variant = Variant.Create(VALUE);
            string[] val = variant;
            Debug.Assert(val == VALUE);

            string[] VALUE2 = { "Hi", "Sky" };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestArrayBytes()
        {
            byte[][] VALUE = { new byte[] { 1, 2, 3 }, new byte[] { 2, 3, 4 }, new byte[] { 3, 4, 5 }, new byte[] { 4, 5, 6 } };
            Variant variant = Variant.Create(VALUE);
            byte[][] val = variant;
            Debug.Assert(val == VALUE);

            byte[][] VALUE2 = { new byte[] { 1, 2, 3 }, new byte[] { 3, 4, 5 }, new byte[] { 4, 5, 6 } };
            variant.SetData(VALUE2);
            val = variant;
            Debug.Assert(val == VALUE2);
        }

        [Fact]
        public void TestStruct()
        {
            Variant variant = Variant.Create(new VariantStruct());
            variant.Add("hello", -123);
            variant.Add("world", "200.5");

            VariantStruct val = variant.GetData<VariantStruct>();
            Debug.Assert(val != null);

            int val1 = variant.GetData<int>("hello");
            Debug.Assert(val1 == -123);

            string val2 = variant.GetData<string>("world");
            Debug.Assert(val2 == "200.5");

            double val3 = variant.GetData<double>("world");
            Debug.Assert(val3 == 200.5);
        }

        [Fact]
        public void TestStructSub()
        {
            Variant variant = Variant.Create(new VariantStruct());
            variant.Add<int>("hello", -123);
            variant.Add("world", "200.5");
            variant.Add("sub", new VariantStruct());

            Variant sub = variant.GetVariant("sub");
            Debug.Assert(sub != null);

            sub.Add("a", 100);
            sub.Add("b", 200);

            int val = variant.GetData<int>("sub.a");
            Debug.Assert(val == 100);
            Debug.Assert(variant.GetVariant("sub.b") == sub.GetVariant("b"));

            double v = variant.GetData<double>("sub.b");
            Debug.Assert(v == 200.0);

            Variant var = variant.GetVariant("hello.b");
            Debug.Assert(var == null);
        }

        [Fact]
        public void TestList()
        {
            Variant variant = Variant.Create(new VariantList());
            variant.Add(-123);
            variant.Add("200.5");

            Variant var = variant.GetVariant("0");
            Debug.Assert(var != null);
            Debug.Assert(var == -123);
            
            double v = variant.GetData<double>("0");
            Debug.Assert(v == -123.0);

            var = variant.GetVariant("7");
            Debug.Assert(var == null);

            v = variant.GetData<double>("7");
            Debug.Assert(v == 0);
        }

        [Fact]
        public void TestListSub()
        {
            Variant variant = Variant.Create(new VariantList());
            variant.Add(-123);
            variant.Add("200.5");
            variant.Add(new VariantStruct());

            Variant sub = variant.GetVariant("2");
            Debug.Assert(sub != null);

            sub.Add("a", 100);
            sub.Add("b", 200);

            Variant var = variant.GetVariant("2.b");
            Debug.Assert(var != null);
            Debug.Assert(var == sub.GetVariant("b"));

            double v = variant.GetData<double>("2.b");
            Debug.Assert(v == 200.0);

            var = variant.GetVariant("0.b");
            Debug.Assert(var == null);
        }

    }
}