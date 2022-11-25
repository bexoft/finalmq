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

    public class TestVariantValueFactory : IDisposable
    {

        public TestVariantValueFactory()
        {
        }

        public void Dispose()
        {
        }

        [Fact]
        public void Test()
        {
            IVariantValue? value1 = VariantValueFactory.Instance.CreateVariantValue(5.3);
            Debug.Assert(value1 != null);
            Debug.Assert(value1.VarType == (int)MetaTypeId.TYPE_DOUBLE);
            Debug.Assert((double)value1.Data == 5.3);

            IVariantValue? value2 = VariantValueFactory.Instance.CreateVariantValue(7);
            Debug.Assert(value2 != null);
            Debug.Assert(value2.VarType == (int)MetaTypeId.TYPE_INT32);
            Debug.Assert((int)value2.Data == 7);

            IVariantValue? value3 = VariantValueFactory.Instance.CreateVariantValue(7.0);
            Debug.Assert(value3 != null);
            Debug.Assert(value3.VarType == (int)MetaTypeId.TYPE_DOUBLE);
            Debug.Assert((int)value3.Data == 7);

            IVariantValue? value4 = VariantValueFactory.Instance.CreateVariantValue("Hello");
            Debug.Assert(value4 != null);
            Debug.Assert(value4.VarType == (int)MetaTypeId.TYPE_STRING);
            Debug.Assert((string)value4.Data == "Hello");

            IVariantValue? value5 = VariantValueFactory.Instance.CreateVariantValue(new bool[5]);
            Debug.Assert(value5 != null);
            Debug.Assert(value5.VarType == (int)MetaTypeId.TYPE_ARRAY_BOOL);
            bool[] ba = value5.Data;
            Debug.Assert(ba.Length == 5);
        }
    }
}
