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
        }
    }
}
